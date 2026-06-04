# VisIt 3.4.0 — CS 510 bugfix sprint

**Course:** CS 510 Scientific Visualization  
**Author:** Madeline Luu, Spring 2026  
**Base:** [VisIt 3.4.0](https://github.com/visit-dav/visit)

This repository is a personal fork of VisIt 3.4.0 with fixes and documentation for four documented upstream issues. Code changes live in the normal VisIt `src/` tree; assignment evidence is under [`docs/`](docs/).

## Quick links

| Document | Purpose |
|----------|---------|
| [docs/README.md](docs/README.md) | Sprint summary for Hank |
| [docs/bugs/README.md](docs/bugs/README.md) | Per-issue index (repro, evidence, notes) |

## Bugs addressed

| Issue | Status |
|-------|--------|
| [#18516](https://github.com/visit-dav/visit/issues/18516) UTF-16 curve files | **Fixed** |
| [#20096](https://github.com/visit-dav/visit/issues/20096) First-state cycle in series | **Fixed** |
| [#19952](https://github.com/visit-dav/visit/issues/19952) Average Value / statistics | **Fixed + enhanced** |
| [#18984](https://github.com/visit-dav/visit/issues/18984) Scatter log axis labels | **Diagnosed (pending)** |

## Build and verify (local)

1. Configure and build VisIt so `build/bin/run_visit` exists (same as your course build).
2. Run all repro scripts:

```bash
./scripts/run_all_repros.sh
```

Or run one issue from `build/bin`:

```bash
cd build/bin
./run_visit -s ../../docs/bugs/19952-average-value/repro/bug_19952.py
```

## Environment limits

- **No GUI** in this build (`-nowin` CLI only).
- **Image rendering broken** — compute engine fails OpenGL init (`GLEW could not be initialized`). Verification uses metadata and queries, not saved images.
- **No Silo plugin** in this build — repros use synthetic VTK/ultra files under `docs/bugs/*/repro/`.

## For Hank — review checklist

| Issue | Code | Test | Evidence |
|-------|------|------|----------|
| #18516 | `src/databases/Curve2D/` | `curve_utf16.py` | `docs/bugs/18516-utf16-curve/evidence/` |
| #20096 | `src/databases/VTK/` | `cycle_from_filename.py` | `docs/bugs/20096-guess-cycle/evidence/` |
| #19952 | `src/avt/Queries/` | `src/test/tests/queries/*.py` | `docs/bugs/19952-average-value/evidence/` |
| #18984 | (diagnosis) | — | `docs/bugs/18984-scatter-log-axis/` |

The `build/` directory is **not** committed; a local build is required to run the repro scripts.
