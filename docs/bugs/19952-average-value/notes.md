# VisIt issue #19952 - "Average Value" / Statistics queries

GitHub: https://github.com/visit-dav/visit/issues/19952

## Symptom

Users run the **Average Value** query expecting the simple arithmetic mean of a
variable, `Sum(v) / N`, but get a different number. On the issue's example data
the discrepancies were:

| variable | what users expected (arithmetic mean) | what "Average Value" returned |
|----------|---------------------------------------|-------------------------------|
| `d`      | 3.495                                 | 3.861                         |
| `u`      | 0.0685                                | 0.0222                        |

The maintainers also noted two further problems:

1. The result text ("The average value of X is ...") never said *how* the
   average was computed, so the weighting was invisible.
2. The **Sample / Population Statistics** queries double-counted ghost cells,
   skewing the mean/variance at domain boundaries.

## Root cause

"Average Value" (`avtAverageValueQuery`) is a one-line subclass of
`avtWeightedVariableSummationQuery`. It computes a **cell-size-weighted** mean

```
Sum_i (v_i * w_i) / Sum_i (w_i)
```

where `w_i` is the element's geometric size: `1` for points, length (1D), area
(2D-XY), revolved volume (2D-RZ), or volume (3D). When cells differ in size this
is *not* the arithmetic mean - hence the mismatch. The behavior was correct but
undocumented and surprising.

`avtSampleStatisticsQuery::Execute1/Execute2` iterated every tuple of the scalar
array with no ghost test, so ghost cells/nodes (which exist at domain/processor
boundaries) were counted, unlike the summation queries which already skip them.

## Changes

Decision (per the issue discussion - markcmiller86's "compute both"): keep the
existing size-weighted behavior for backward compatibility, but make it explicit
and *also* give users an easy way to get the plain arithmetic mean.

### Phase 1 - Transparency
- `src/avt/Queries/Queries/avtSummationQuery.{h,C}`
  - New protected `double elementCount`, accumulated (ghost-excluded) in
    `Execute`, reduced across processors in `PostExecute`.
  - New virtuals `AverageByCount()` and `GetAverageWeightingDescriptor()`.
  - `PostExecute` now names the weighting basis in the message
    ("The **area-weighted** average value of X is ...") and reports the
    non-ghost element count and total weight, plus a pointer to the unweighted
    query. `count` / `total_weight` are added to the result `MapNode`.
- `src/avt/Queries/Queries/avtWeightedVariableSummationQuery.{h,C}`
  - Records the weighting basis ("length"/"area"/"revolved-volume"/"volume")
    in `ApplyFilters` and returns it from `GetAverageWeightingDescriptor()`.

### Phase 2 - True unweighted arithmetic mean
- New query **`avtAverageValueUnweightedQuery`**
  (`src/avt/Queries/Queries/avtAverageValueUnweightedQuery.{h,C}`): subclass of
  `avtVariableSummationQuery` with `CalculateAverage()==true` and
  `AverageByCount()==true`, so it computes `Sum(v) / N` with ghosts excluded.
- Registered as **"Average Value (Unweighted)"** in
  `avtQueryFactory.C` and `ViewerQueryManager.C`.

### Phase 3 - Ghost handling in statistics
- `src/avt/Queries/Queries/avtSampleStatisticsQuery.{h,C}`: `Execute1` and
  `Execute2` now skip ghost cells/nodes (new static `TupleIsGhost`, mirroring
  `avtSummationQuery::Execute`). Fixes mean/variance/stddev/skewness/kurtosis
  for meshes with ghosts; applies to both Sample and Population variants.

### Phase 4 - New "Statistics" query
- New query **`avtStatisticsQuery`**
  (`src/avt/Queries/Queries/avtStatisticsQuery.{h,C}`): a ghost-aware two-pass
  query that reports count, min, max, range, sum, (unweighted) mean, variance,
  standard deviation, skewness, and kurtosis in one go. Every measure is also
  returned in the result `MapNode`. Registered as **"Statistics"** in
  `avtQueryFactory.C`, `ViewerQueryManager.C`, and the query `CMakeLists.txt`.
  - Scope note: the size-weighted mean is intentionally left to "Average Value"
    (computing it here would require the size-expression pipeline; the message
    points users there). Median/mode/percentiles are a deliberate follow-up.

### Cross-cutting - terminology
- Averaging/statistics messages now consistently describe the inputs as
  "non-ghost values", matching the summation queries' ghost semantics.

## Validation (non-rendering)

This build cannot render images and has no Silo plugin, so validation uses
data-level queries on synthetic VTK meshes (deliberately unequal cell sizes so
weighted != unweighted, and a `vtkGhostType` array to create ghost cells).

- `build/bin/bug_19952.py` - rectilinear mesh, areas 1 and 2.
  - `zvar = [10, 20]`: Average Value = **16.6667** (area-weighted),
    Average Value (Unweighted) = **15.0**.
  - `nvar = [1..6]`: Average Value (Unweighted) = **3.5**.
  - Statistics(zvar): count 2, min 10, max 20, range 10, sum 30, mean 15.
  - **Result: 11 / 11 checks passed.**
- `build/bin/bug_19952_ghost.py` - 3 cells `[10, 20, 30]`, third flagged ghost.
  - Sample Statistics mean = **15** (ghost-excluded; was 20 before Phase 3),
    Statistics count = 2 / mean = 15, Average Value (Unweighted) = 15.

Regression tests (VisIt test harness, self-contained / inline `TestValueEQ`,
no Silo or text baselines):
- `src/test/tests/queries/average_weighting.py`
- `src/test/tests/queries/statistics_ghosts.py`
- `src/test/tests/queries/statistics_query.py`

### Build

Rebuilt incrementally and cleanly:
- `make avtquery_ser` (new query sources + edited summation/statistics queries)
- `make viewer` (new query-name registrations)

The engine loads `libavtquery_ser.so` dynamically, so the factory changes are
picked up without relinking the engine executable.

## Known follow-ups / limitations

- The existing nightly test `src/test/tests/queries/avg_value.py` compares the
  Average Value **message text** against stored baselines. Phase 1 intentionally
  changes that text, so `avg_value_01..04` baselines must be regenerated in the
  upstream nightly environment (the Silo data and baseline files are not part of
  this checkout, so they cannot be regenerated here).
- `CalculateAverage()` in the new unweighted path divides by the non-ghost
  element count; for empty selections this is guarded (no divide-by-zero).
