# Issue #18516 — UTF-16 curve / ultra files

**GitHub:** https://github.com/visit-dav/visit/issues/18516  
**Status:** Fixed and verified  
**Upstream PR:** Not submitted in this sprint

## Files changed

- `src/databases/Curve2D/avtCurve2DFileFormat.C`

## Regression test

- `src/test/tests/databases/curve_utf16.py`

## Reproduce

From a built VisIt tree (`build/bin/run_visit` must exist):

```bash
cd build/bin
./run_visit -s ../../docs/bugs/18516-utf16-curve/repro/bug_18516.py
```

Expected: all four encoding variants (UTF-8, UTF-8 BOM, UTF-16 LE, UTF-16 BE) report curves `curve_a`, `curve_b`.

## Evidence

- `evidence/before.txt` — behavior before the fix
- `evidence/after.txt` — behavior after the fix
- `evidence/repro.log` — output from the repro script (if present)

## Notes

See [notes.md](notes.md) for root cause and implementation details.
