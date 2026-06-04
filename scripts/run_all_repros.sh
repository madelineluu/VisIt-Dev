#!/usr/bin/env bash
# Run all assignment repro scripts and save logs under docs/bugs/*/evidence/.
# Requires: build/bin/run_visit (VisIt already built).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
RUN_VISIT="${ROOT}/build/bin/run_visit"

if [[ ! -x "${RUN_VISIT}" ]]; then
  echo "ERROR: ${RUN_VISIT} not found. Build VisIt first." >&2
  exit 1
fi

cd "${ROOT}/build/bin"

run_one() {
  local script="$1"
  local log="$2"
  local name
  name="$(basename "${script}")"
  echo "======== ${name} ========"
  mkdir -p "$(dirname "${log}")"
  if timeout 300 "${RUN_VISIT}" -s "${script}" > "${log}" 2>&1; then
    echo "OK  ${name} -> ${log}"
    tail -5 "${log}" || true
    return 0
  else
    local ec=$?
    echo "FAIL ${name} (exit ${ec}) -> ${log}"
    tail -15 "${log}" || true
    return "${ec}"
  fi
}

FAIL=0

run_one "${ROOT}/docs/bugs/18516-utf16-curve/repro/bug_18516.py" \
  "${ROOT}/docs/bugs/18516-utf16-curve/evidence/repro.log" || FAIL=1

run_one "${ROOT}/docs/bugs/20096-guess-cycle/repro/bug_20096.py" \
  "${ROOT}/docs/bugs/20096-guess-cycle/evidence/repro.log" || FAIL=1

run_one "${ROOT}/docs/bugs/19952-average-value/repro/bug_19952.py" \
  "${ROOT}/docs/bugs/19952-average-value/evidence/validation.log" || FAIL=1

run_one "${ROOT}/docs/bugs/19952-average-value/repro/bug_19952_ghost.py" \
  "${ROOT}/docs/bugs/19952-average-value/evidence/ghost_probe.log" || FAIL=1

run_one "${ROOT}/docs/bugs/18984-scatter-log-axis/repro/bug_18984.py" \
  "${ROOT}/docs/bugs/18984-scatter-log-axis/evidence/repro.log" || FAIL=1

echo ""
if [[ "${FAIL}" -eq 0 ]]; then
  echo "All repro scripts finished successfully."
else
  echo "One or more repro scripts failed (see evidence/*.log)."
  exit 1
fi
