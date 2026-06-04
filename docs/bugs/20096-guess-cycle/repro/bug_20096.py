# ---------------------------------------------------------------------------
#  Repro script for VisIt issue #20096
#  "GuessCycle() skipped for the first file in a time series"
#
#  Run from build/bin/ with:
#      ./run_visit -s ../../docs/bugs/20096-guess-cycle/repro/bug_20096.py
#
#  Before the fix : cycles = [0, 200, 300]
#  After  the fix : cycles = [100, 200, 300]
# ---------------------------------------------------------------------------

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(SCRIPT_DIR, "output")
os.makedirs(OUT_DIR, exist_ok=True)

VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "bug20096\n"
    "ASCII\n"
    "DATASET STRUCTURED_POINTS\n"
    "DIMENSIONS 2 2 2\n"
    "ORIGIN 0 0 0\n"
    "SPACING 1 1 1\n"
    "POINT_DATA 8\n"
    "SCALARS s float 1\n"
    "LOOKUP_TABLE default\n"
    "0 1 2 3 4 5 6 7\n"
)

NAMES = ["series_0100.vtk", "series_0200.vtk", "series_0300.vtk"]
EXPECTED = [100, 200, 300]


def write_series():
    for n in NAMES:
        with open(os.path.join(OUT_DIR, n), "w") as fp:
            fp.write(VTK_TEXT)
    visit_path = os.path.join(OUT_DIR, "bug20096_series.visit")
    with open(visit_path, "w") as fp:
        for n in NAMES:
            fp.write(os.path.join(OUT_DIR, n) + "\n")
    return visit_path


db = write_series()
CloseDatabase(db)
OpenDatabase(db)
md = GetMetaData(db)
cycles = list(md.cycles)

print("=" * 70)
print("VisIt #20096 - cycle-from-filename for the first state in a series")
print("=" * 70)
print("Expected cycles : %s" % EXPECTED)
print("Reported cycles : %s" % cycles)
print("-" * 70)
ok = cycles == EXPECTED
if ok:
    print("RESULT: PASS - every state reports its filename cycle (#20096 fixed)")
else:
    print("RESULT: FAIL - first state lost its cycle (#20096 present)")
print("=" * 70)

sys.stdout.flush()
sys.stderr.flush()
os._exit(0 if ok else 1)
