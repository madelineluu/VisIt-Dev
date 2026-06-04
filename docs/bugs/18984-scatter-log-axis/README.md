# Issue #18984 — Scatter plot log axis labels

**GitHub:** https://github.com/visit-dav/visit/issues/18984  
**Status:** Diagnosed, **not fixed** (pending)  
**Upstream PR:** Not submitted in this sprint

## Files changed

None (diagnosis only). Target for a future fix:

- `src/plots/Scatter/avtScatterFilter.C`
- Viewer log-axis integration (`View2D`, `avtMeshLogFilter`)

## Regression test

None yet.

## Reproduce (non-rendering)

Uses bundled `repro/data/kitchen1.vtk`.

```bash
cd build/bin
./run_visit -s ../../docs/bugs/18984-scatter-log-axis/repro/bug_18984.py
```

Shows that Log scaling stores `log10` values as coordinates, so axis labels would be wrong.

## Evidence

- `evidence/repro.log` — spatial extents under Linear vs Log scaling
- `evidence/rendering_failure.txt` — notes on GLEW / engine crash if image tests were attempted

## Blocker

Offscreen rendering fails in this build (`GLEW could not be initialized`), so tick-label images cannot be used to verify a fix here.

## Notes

See [notes.md](notes.md).
