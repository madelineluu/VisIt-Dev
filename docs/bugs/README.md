# VisIt bugfix index (CS 510)

One folder per GitHub issue. Each folder has a short **README**, detailed **notes.md**, **repro/** scripts, and **evidence/** logs or before/after output.

| Issue | Folder | Status | Source fix | Harness test | Repro |
|-------|--------|--------|------------|--------------|-------|
| [#18516](https://github.com/visit-dav/visit/issues/18516) | [18516-utf16-curve/](18516-utf16-curve/) | Fixed | `src/databases/Curve2D/avtCurve2DFileFormat.C` | `src/test/tests/databases/curve_utf16.py` | `repro/bug_18516.py` |
| [#20096](https://github.com/visit-dav/visit/issues/20096) | [20096-guess-cycle/](20096-guess-cycle/) | Fixed | `src/databases/VTK/avtVTKFileFormat.C` | `src/test/tests/databases/cycle_from_filename.py` | `repro/bug_20096.py` |
| [#19952](https://github.com/visit-dav/visit/issues/19952) | [19952-average-value/](19952-average-value/) | Fixed + enhanced | `src/avt/Queries/Queries/` (+ factory, viewer, `Query.rst`) | `src/test/tests/queries/average_weighting.py`, `statistics_ghosts.py`, `statistics_query.py` | `repro/bug_19952.py`, `repro/bug_19952_ghost.py` |
| [#18984](https://github.com/visit-dav/visit/issues/18984) | [18984-scatter-log-axis/](18984-scatter-log-axis/) | Diagnosed (pending) | (no code fix yet) | — | `repro/bug_18984.py` |

Sprint summary: [../BUGFIX_REPORT.md](../BUGFIX_REPORT.md)

Run all repro scripts (requires a local `build/`):

```bash
./scripts/run_all_repros.sh
```
