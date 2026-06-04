# Sprint 2: VisIt 3.4.0 Bugfix Report
CS 510 Scientific Visualization \
Madeline Luu, Spring 2026

This report summarizes work on **four** documented VisIt bugs. Each bug has a
folder under [`docs/bugs/`](bugs/) with detailed **notes.md**, **repro/** scripts,
and **evidence/** logs. Code fixes remain in the normal VisIt `src/` tree;
regression tests are under `src/test/tests/databases/` and
`src/test/tests/queries/`.

Important environment note: **image rendering does not work in this build.** While
working on bug #18984, which involved log scaling labels on scatter plots, I found
that the compute engine cannot initialize its offscreen OpenGL context
(`GLEW could not be initialized` in `A.engine_ser.*.vlog`, then SIGSEGV). This
is an engine OSMesa/GL problem, not a missing-GUI problem. Because of it, all
verification here uses **non-rendering metadata/query checks** rather than image
comparisons.

| Issue | Title | Status | Folder |
|-------|-------|--------|--------|
| #18516 | Unable to read UTF-16 Little-endian ultra files | **Fixed & verified** | [bugs/18516-utf16-curve/](bugs/18516-utf16-curve/) |
| #20096 | `GuessCycle()` called for every file in a series except the first | **Fixed & verified** | [bugs/20096-guess-cycle/](bugs/20096-guess-cycle/) |
| #18984 | Log scaling in the scatter plot leaves axis labels incorrect | **Diagnosed, not fixed (pending)** | [bugs/18984-scatter-log-axis/](bugs/18984-scatter-log-axis/) |
| #19952 | "Average Value" query doesn't make sense (+ ghost handling) | **Fixed & enhanced, verified** | [bugs/19952-average-value/](bugs/19952-average-value/) |

**Per-issue index:** [bugs/README.md](bugs/README.md)

---

## #18516 - UTF-16 curve/ultra files (FIXED)

**Problem.** The Curve2D reader assumed ASCII/UTF-8 text, so ultra/curve files
saved as UTF-16 (with a byte-order mark) failed to load.

**Fix.** `src/databases/Curve2D/avtCurve2DFileFormat.C` now detects a leading
BOM (UTF-8, UTF-16 LE, UTF-16 BE) and transcodes UTF-16 content to plain text
before parsing.

**Verification.**
- Repro: `docs/bugs/18516-utf16-curve/repro/bug_18516.py`
- Evidence: `docs/bugs/18516-utf16-curve/evidence/before.txt`, `after.txt`
- Test: `src/test/tests/databases/curve_utf16.py`

---

## #20096 - First state in a series shows cycle 0 (FIXED)

**Problem.** For a series whose cycle numbers live in the file names
(`series_0100.vtk`, ...), the time slider showed the right cycle for every state
except the first, which showed `0`.

**Root cause.** The format interface's capability probe
(`GetCycleFromFilename("")` on the first timestep) had a side effect in the VTK
reader: it overwrote the first timestep's cached `cycleFromFilename` with
INVALID. The accurate-cycle read (`GetCycle()`) then returned that INVALID
fallback for state 0 only.

**Fix.** `src/databases/VTK/avtVTKFileFormat.C` - the STSD and STMD
`GetCycleFromFilename` no longer cache the result when called with the empty
probe string; capability detection is unchanged.

**Verification (non-rendering).**
- Repro: `docs/bugs/20096-guess-cycle/repro/bug_20096.py`
- Test: `src/test/tests/databases/cycle_from_filename.py`
- Before: `cycles = [0, 200, 300]`
- After: `cycles = [100, 200, 300]`

---

## #18984 - Scatter plot log axis labels (DIAGNOSED, PENDING)

**Problem.** Setting a Scatter plot coordinate axis to Log produces the correct
point spacing but wrong tick labels (`0,1,2,3` instead of `1,10,100,1000`).

**Root cause.** `avtScatterFilter` applies `log10()` to the coordinates and
stores the logged values as the actual positions, so the annotation system
labels the axis from logged data. VisIt's correct log-axis machinery
(`View2D.xScale = LOG` -> `avtMeshLogFilter` + log-aware labels) is bypassed.

**Why not fixed.** Validating tick labels requires rendered images, which this
build cannot produce (engine GL failure). A non-rendering
`Query("SpatialExtents")` check in
`docs/bugs/18984-scatter-log-axis/repro/bug_18984.py` confirms the diagnosis.
See [bugs/18984-scatter-log-axis/notes.md](bugs/18984-scatter-log-axis/notes.md)
and `evidence/rendering_failure.txt`.

---

## #19952 - "Average Value" / Statistics queries (FIXED & ENHANCED)

**Problem.** The **Average Value** query silently returns a *cell-size-weighted*
mean (`Sum(v*size)/Sum(size)`), not the arithmetic mean users expect, and never
said so. Separately, the **Sample/Population Statistics** queries double-counted
ghost cells.

**Changes.**
- *Transparency:* the Average Value message now names its weighting basis
  (e.g. "The **area-weighted** average value ..."), reports the non-ghost
  element count and total weight, and points to the unweighted query.
- *Unweighted mean:* new **"Average Value (Unweighted)"** query
  (`avtAverageValueUnweightedQuery`) = `Sum(v)/N`, ghost-excluded.
- *Ghost fix:* `avtSampleStatisticsQuery` now skips ghost cells/nodes.
- *New query:* **"Statistics"** (`avtStatisticsQuery`) reports count/min/max/
  range/sum/mean/stddev/variance/skewness/kurtosis in one ghost-aware pass.

**Verification (non-rendering).**
- Repro: `docs/bugs/19952-average-value/repro/bug_19952.py` (**11/11 checks**)
  and `bug_19952_ghost.py` (ghost mean 15, not 20).
- Tests: `src/test/tests/queries/average_weighting.py`,
  `statistics_ghosts.py`, `statistics_query.py`.

---

## Per-bug artifacts

| Issue | Notes | Repro | Evidence |
|-------|-------|-------|----------|
| #18516 | `bugs/18516-utf16-curve/notes.md` | `.../repro/bug_18516.py` | `.../evidence/` |
| #20096 | `bugs/20096-guess-cycle/notes.md` | `.../repro/bug_20096.py` | `.../evidence/repro.log` |
| #18984 | `bugs/18984-scatter-log-axis/notes.md` | `.../repro/bug_18984.py` | `.../evidence/` |
| #19952 | `bugs/19952-average-value/notes.md` | `.../repro/bug_19952*.py` | `validation.log`, `ghost_probe.log` |

Run all repros from repo root: `./scripts/run_all_repros.sh` (requires local `build/`).

Tests: `src/test/tests/databases/curve_utf16.py`,
`cycle_from_filename.py`, `src/test/tests/queries/average_weighting.py`,
`statistics_ghosts.py`, `statistics_query.py`.
