# Submission checklist

Use this before pushing to your personal GitHub repository.

## Documentation

- [x] [BUGFIX_REPORT.md](BUGFIX_REPORT.md) — sprint summary
- [x] [bugs/README.md](bugs/README.md) — per-issue index
- [x] Each `docs/bugs/*/README.md` — status, files changed, repro command, evidence
- [x] Each `docs/bugs/*/notes.md` — detailed write-up
- [x] [../README.md](../README.md) — repo landing page for graders

## Evidence (committed under `docs/bugs/*/evidence/`)

- [x] #18516 — `before.txt`, `after.txt`, `repro.log`
- [x] #20096 — `repro.log`
- [x] #19952 — `validation.log`, `ghost_probe.log`
- [x] #18984 — `repro.log`, `rendering_failure.txt`

Regenerate logs after code changes:

```bash
./scripts/run_all_repros.sh
```

## Code and tests (in `src/`)

- [x] Fixes remain in standard VisIt paths (not duplicated under `docs/`)
- [x] Regression tests under `src/test/tests/databases/` and `src/test/tests/queries/`

## Git hygiene

- [x] Root `.gitignore` excludes `build/`
- [ ] Run `git status` — no `*.vlog`, `nohup.out`, or large binaries staged
- [ ] #18984 clearly marked **not fixed** in docs

## Git commands (personal GitHub — no upstream PRs)

From the repository root (`visit3.4.0/`):

```bash
# First time only
git init
git remote add origin git@github.com:YOUR_USER/YOUR_REPO.git

# Stage assignment + source changes (build/ is ignored)
git add docs/ README.md scripts/ .gitignore src/

# Review what will be committed
git status
git diff --cached --stat

# Commit (pick one style)
git commit -m "$(cat <<'EOF'
CS510: VisIt 3.4.0 bugfixes (#18516, #20096, #19952) + docs

- Fix UTF-16 curve reading, VTK first-state cycle, query averages/statistics
- Diagnose scatter log-axis issue (#18984, pending)
- Add docs/bugs evidence and repro scripts for assignment submission
EOF
)"

# Optional tag for the instructor
git tag -a cs510-sprint2-submission -m "CS 510 sprint 2 submission"

# Push
git branch -M main
git push -u origin main
git push origin cs510-sprint2-submission   # if you created the tag
```

If the VisIt tree is already a git clone, use `git add` on the paths above instead of `git init`.

## What graders need

1. Clone your repo and read [README.md](../README.md).
2. Build VisIt (or trust committed `src/` diffs + evidence logs).
3. Optionally run `./scripts/run_all_repros.sh` from a machine with your `build/` present.

## Not in scope for this submission

- Pull requests to [visit-dav/visit](https://github.com/visit-dav/visit)
- Regenerating nightly baselines for `avg_value.py` (Silo data not in this tree)
- Fixing #18984 or the rendering environment
