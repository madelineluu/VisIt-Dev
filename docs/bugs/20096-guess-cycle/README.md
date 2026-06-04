# Issue #20096 — First timestep cycle wrong in a series

**GitHub:** https://github.com/visit-dav/visit/issues/20096  
**Status:** Fixed and verified  
**Upstream PR:** Not submitted in this sprint

## Files changed

- `src/databases/VTK/avtVTKFileFormat.C` (STSD and STMD `GetCycleFromFilename`)

## Regression test

- `src/test/tests/databases/cycle_from_filename.py`

## Reproduce

```bash
cd build/bin
./run_visit -s ../../docs/bugs/20096-guess-cycle/repro/bug_20096.py
```

Expected cycles: `[100, 200, 300]` (from filenames `series_0100.vtk`, etc.).  
Before fix: `[0, 200, 300]`.

## Evidence

- `evidence/repro.log` — repro script output (if present)

## Notes

See [notes.md](notes.md).
