# Issue #19952 — Average Value / statistics queries

**GitHub:** https://github.com/visit-dav/visit/issues/19952  
**Status:** Fixed and enhanced  
**Upstream PR:** Not submitted in this sprint

## Files changed

- `src/avt/Queries/Queries/avtSummationQuery.{h,C}`
- `src/avt/Queries/Queries/avtWeightedVariableSummationQuery.{h,C}`
- `src/avt/Queries/Queries/avtAverageValueUnweightedQuery.{h,C}` (new)
- `src/avt/Queries/Queries/avtSampleStatisticsQuery.{h,C}`
- `src/avt/Queries/Queries/avtStatisticsQuery.{h,C}` (new)
- `src/avt/Queries/Queries/CMakeLists.txt`
- `src/avt/Queries/Misc/avtQueryFactory.C`
- `src/viewer/core/ViewerQueryManager.C`
- `src/doc/using_visit/Quantitative/Query.rst`

## Regression tests

- `src/test/tests/queries/average_weighting.py`
- `src/test/tests/queries/statistics_ghosts.py`
- `src/test/tests/queries/statistics_query.py`

## Reproduce

```bash
cd build/bin
./run_visit -s ../../docs/bugs/19952-average-value/repro/bug_19952.py
./run_visit -s ../../docs/bugs/19952-average-value/repro/bug_19952_ghost.py
```

Main script: **11/11** checks (weighted vs unweighted mean on unequal cell sizes).  
Ghost script: Sample Statistics mean **15** (ghost cell excluded), not 20.

## Evidence

- `evidence/validation.log` — full `bug_19952.py` output
- `evidence/ghost_probe.log` — `bug_19952_ghost.py` output

## Notes

See [notes.md](notes.md).
