# Bug #20096 - First state in a series shows cycle 0 (GuessCycle skipped)

Status: **FIXED and verified** (non-rendering metadata check).

GitHub issue: https://github.com/visit-dav/visit/issues/20096
Reporter (maintainer): markcmiller86

---

## 1. Symptom

Open a time series whose cycle numbers are encoded in the file names
(`series_0100.vtk`, `series_0200.vtk`, `series_0300.vtk`, ...). The time slider
shows the correct cycle for every state EXCEPT the first, which shows `0`.

The maintainer summarized it as: `avtFileFormat::GuessCycle()` is called for
every file in the series except the first (the one being opened).

Reproduced here with a VTK series:

```
opened series  cycles = [0, 200, 300]   cyclesAreAccurate = [0, 1, 1]
```

States 1 and 2 carry their guessed cycle (200, 300) and are marked accurate;
state 0 is `0` and not accurate.

---

## 2. Root cause

The trigger is a **destructive capability probe** combined with a reader that
**caches** its filename-cycle guess.

### The VTK reader caches the guess

`avtVTK_STSDFileFormat` (and the STMD variant) override `GetCycleFromFilename`
to "bless" guesses made by the base-class `GuessCycle`. The override also stores
the guess in a mutable member, and `GetCycle()` returns that member as a
fallback when the file itself has no cycle:

```97:102:src/databases/VTK/avtVTKFileFormat.C
int
avtVTK_STSDFileFormat::GetCycleFromFilename(const char *f) const
{
    cycleFromFilename = GuessCycle(f);   // <-- original code (clobbers cache)
    return cycleFromFilename;
}
```

```48:56:src/databases/VTK/avtVTKFileFormat.C
int
avtVTK_STSDFileFormat::GetCycle(void)
{
    int cycle = reader->GetCycle();
    if( cycle == INVALID_CYCLE )
      return cycleFromFilename;          // <-- fallback to the cached guess
    else
      return cycle;
}
```

The constructor seeds `cycleFromFilename` with the correct guess for the file
(`GetCycleFromFilename(filename)`).

### The interface probes with an empty filename

To decide whether a plugin's filename guesses can be trusted,
`avtST{SD,MD}FileFormatInterface::SetDatabaseMetaData` probes the **first**
timestep:

```481:486:src/avt/Database/Formats/avtSTSDFileFormatInterface.C
        bool canGetGoodCycleFromFilename =
                 timesteps[0][0]->FormatGetCycleFromFilename("") !=
                     avtFileFormat::FORMAT_INVALID_CYCLE;
```

`GuessCycle("")` returns `INVALID_CYCLE` (`-INT_MAX`), which differs from
`FORMAT_INVALID_CYCLE` (`-INT_MAX + 1`), so the probe correctly concludes "this
format blesses filename guesses." **But** the VTK override also did
`cycleFromFilename = GuessCycle("") = INVALID_CYCLE`, **clobbering the first
timestep's cached cycle.**

### Why only the first state breaks

When VisIt later reads *accurate* cycles for the time slider it uses the
`forceReadAllCyclesTimes` path, which calls `FormatGetCycle()` (not
`GetCycleFromFilename`). For VTK files with no embedded cycle, `GetCycle()`
falls back to `cycleFromFilename`:

- **State 0** (the probed timestep): `cycleFromFilename` was overwritten with
  `INVALID_CYCLE` by the probe -> `GetCycle()` returns INVALID -> cycle stays 0,
  not accurate.
- **States 1..n**: never probed, so their constructor-seeded `cycleFromFilename`
  is intact -> `GetCycle()` returns 200, 300, ... and they are accurate.

That is precisely "GuessCycle effectively skipped for the first file only."

Note: readers that do NOT fall back to a filename guess in `GetCycle()` (e.g.
Curve2D) show a different, unrelated symptom (all states 0 on the accurate-read
path) and are out of scope for this issue.

---

## 3. The fix

Do not let the empty-filename capability probe overwrite the cached cycle. The
probe still returns the guess (so capability detection is unchanged), but the
cache is only updated for a real filename.

```src/databases/VTK/avtVTKFileFormat.C
int
avtVTK_STSDFileFormat::GetCycleFromFilename(const char *f) const
{
    // An empty filename is the capability probe used by
    // avtSTXXFileFormatInterface::SetDatabaseMetaData. We still return the
    // guess (INVALID_CYCLE) so VisIt detects that this format "blesses"
    // filename-based cycle guesses, but we must NOT overwrite the cached
    // cycleFromFilename for the real file. Overwriting it here clobbers the
    // first timestep's cycle, which GetCycle() later returns as a fallback,
    // leaving the first state in a series with cycle 0 (issue #20096).
    if (f == NULL || f[0] == '\0')
        return GuessCycle(f);

    cycleFromFilename = GuessCycle(f);
    return cycleFromFilename;
}
```

The same guard is applied to `avtVTK_STMDFileFormat::GetCycleFromFilename`.

### Why fix the plugin rather than the interface

The deeper trigger is the interface probing a live timestep object with a
side-effecting call. A plugin-local guard is the smallest, lowest-risk change:
it leaves the probe mechanism (used by every reader) untouched and fixes the
canonical reader that exhibits the bug. Any other reader that caches its
`GetCycleFromFilename` result into a `GetCycle()` fallback would want the same
one-line guard.

---

## 4. Validation

Non-rendering metadata check (rendering is broken in this build; cycles are read
from `GetMetaData`, no image needed).

- Repro script: `build/bin/bug_20096.py` - builds a synthetic VTK series
  (`series_0100.vtk` / `0200` / `0300`) and prints `GetMetaData(db).cycles`.
- Nightly test: `src/test/tests/databases/cycle_from_filename.py` (TestText).

Results:

| build            | cycles reported   |
|------------------|-------------------|
| before fix (VTK) | `[0, 200, 300]`   |
| after fix        | `[100, 200, 300]` |

(The first verification was confirmed against the existing `kitchen.visit`
series, which showed `cycles = (0, 2, 3)` before the fix.)

---

## 5. Files

- Fixed: `src/databases/VTK/avtVTKFileFormat.C`
  (`avtVTK_STSDFileFormat::GetCycleFromFilename`,
   `avtVTK_STMDFileFormat::GetCycleFromFilename`).
- Repro: `build/bin/bug_20096.py`.
- Test: `src/test/tests/databases/cycle_from_filename.py`.
- Relevant context (not modified):
  `src/avt/Database/Formats/avtSTSDFileFormatInterface.C` and
  `avtSTMDFileFormatInterface.C` (the capability probe and the
  forceReadAllCyclesTimes cycle read), `src/avt/Database/Formats/avtFileFormat.C`
  (`GuessCycle`, `INVALID_CYCLE` vs `FORMAT_INVALID_CYCLE`).
