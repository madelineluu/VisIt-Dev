# ---------------------------------------------------------------------------
#  Ghost-cell probe for VisIt issue #19952 (Phase 3)
#
#  Run from build/bin/ with:
#      ./run_visit -s ../../docs/bugs/19952-average-value/repro/bug_19952_ghost.py
#
#  zvar = [10, 20, 30]; cell 2 is ghost. Correct mean = 15, not 20.
# ---------------------------------------------------------------------------

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(SCRIPT_DIR, "output")
os.makedirs(OUT_DIR, exist_ok=True)

VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "ghostprobe\n"
    "ASCII\n"
    "DATASET RECTILINEAR_GRID\n"
    "DIMENSIONS 4 2 1\n"
    "X_COORDINATES 4 float\n"
    "0 1 2 3\n"
    "Y_COORDINATES 2 float\n"
    "0 1\n"
    "Z_COORDINATES 1 float\n"
    "0\n"
    "CELL_DATA 3\n"
    "SCALARS zvar float 1\n"
    "LOOKUP_TABLE default\n"
    "10 20 30\n"
    "SCALARS vtkGhostType unsigned_char 1\n"
    "LOOKUP_TABLE default\n"
    "0 0 1\n"
)

DB = os.path.join(OUT_DIR, "bug19952_ghost.vtk")
with open(DB, "w") as fp:
    fp.write(VTK_TEXT)

CloseDatabase(DB)
OpenDatabase(DB)
AddPlot("Pseudocolor", "zvar")
DrawPlots()

Query("Statistics")
print("STAT_VALS=%s" % str(GetQueryOutputValue()))
print(GetQueryOutputString().strip())

Query("Sample Statistics")
print("SAMPLE_MEAN=%s" % str(GetQueryOutputValue()[0]))

Query("Average Value (Unweighted)")
print("UNWEIGHTED=%s" % str(GetQueryOutputValue()))

sys.stdout.flush()
sys.stderr.flush()
os._exit(0)
