# ---------------------------------------------------------------------------
#  Validation script for VisIt issue #19952
#
#  Run from build/bin/ with:
#      ./run_visit -s ../../docs/bugs/19952-average-value/repro/bug_19952.py
# ---------------------------------------------------------------------------

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(SCRIPT_DIR, "output")
os.makedirs(OUT_DIR, exist_ok=True)

VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "bug19952\n"
    "ASCII\n"
    "DATASET RECTILINEAR_GRID\n"
    "DIMENSIONS 3 2 1\n"
    "X_COORDINATES 3 float\n"
    "0 1 3\n"
    "Y_COORDINATES 2 float\n"
    "0 1\n"
    "Z_COORDINATES 1 float\n"
    "0\n"
    "CELL_DATA 2\n"
    "SCALARS zvar float 1\n"
    "LOOKUP_TABLE default\n"
    "10 20\n"
    "POINT_DATA 6\n"
    "SCALARS nvar float 1\n"
    "LOOKUP_TABLE default\n"
    "1 2 3 4 5 6\n"
)

DB = os.path.join(OUT_DIR, "bug19952.vtk")
TOL = 1.0e-4


def approx(a, b):
    return abs(a - b) <= TOL * max(1.0, abs(b))


def run_query(name, var):
    DeleteAllPlots()
    AddPlot("Pseudocolor", var)
    DrawPlots()
    Query(name)
    return GetQueryOutputValue(), GetQueryOutputString()


with open(DB, "w") as fp:
    fp.write(VTK_TEXT)

CloseDatabase(DB)
OpenDatabase(DB)

results = []
print("=" * 72)
print("VisIt #19952 - Average Value / Statistics query validation")
print("=" * 72)

wval, wmsg = run_query("Average Value", "zvar")
print("\n[Average Value, zvar]  -> %s" % str(wval))
print(wmsg.strip())
results.append(("zvar area-weighted mean == 16.6667", approx(wval, 50.0/3.0)))
results.append(("zvar message mentions 'area-weighted'", "area-weighted" in wmsg))

uval, umsg = run_query("Average Value (Unweighted)", "zvar")
print("\n[Average Value (Unweighted), zvar]  -> %s" % str(uval))
print(umsg.strip())
results.append(("zvar unweighted mean == 15.0", approx(uval, 15.0)))

nval, nmsg = run_query("Average Value (Unweighted)", "nvar")
print("\n[Average Value (Unweighted), nvar]  -> %s" % str(nval))
print(nmsg.strip())
results.append(("nvar unweighted mean == 3.5", approx(nval, 3.5)))

sval, smsg = run_query("Statistics", "zvar")
print("\n[Statistics, zvar]")
print(smsg.strip())
results.append(("Statistics count == 2", approx(sval[0], 2.0)))
results.append(("Statistics min == 10", approx(sval[1], 10.0)))
results.append(("Statistics max == 20", approx(sval[2], 20.0)))
results.append(("Statistics range == 10", approx(sval[3], 10.0)))
results.append(("Statistics sum == 30", approx(sval[4], 30.0)))
results.append(("Statistics mean == 15 (unweighted)", approx(sval[5], 15.0)))

ssval, ssmsg = run_query("Sample Statistics", "nvar")
print("\n[Sample Statistics, nvar]")
print(ssmsg.strip())
results.append(("Sample Statistics mean(nvar) == 3.5", approx(ssval[0], 3.5)))

print("\n" + "-" * 72)
npass = sum(1 for _, ok in results if ok)
for desc, ok in results:
    print("  %-45s : %s" % (desc, "PASS" if ok else "FAIL"))
print("-" * 72)
print("RESULT: %d / %d checks passed" % (npass, len(results)))
print("=" * 72)

sys.stdout.flush()
sys.stderr.flush()
os._exit(0 if npass == len(results) else 1)
