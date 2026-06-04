# ---------------------------------------------------------------------------
#  Repro / diagnosis script for VisIt issue #18984
#  "Scatter plot log scaling leaves axis labels incorrect"
#
#  Run from build/bin/ with:
#      ./run_visit -s ../../docs/bugs/18984-scatter-log-axis/repro/bug_18984.py
#
#  STATUS: DIAGNOSED, NOT FIXED. See ../notes.md
# ---------------------------------------------------------------------------

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(SCRIPT_DIR, "data", "kitchen1.vtk")


def spatial_extents():
    Query("SpatialExtents")
    try:
        return list(GetQueryOutputValue())
    except Exception:
        return None


def setup_plot(x_scaling):
    DeleteAllPlots()
    AddPlot("Scatter", "ke")
    s = ScatterAttributes()
    s.var1 = "ke"
    s.var1Role = s.Coordinate0
    s.var2 = "ep"
    s.var2Role = s.Coordinate1
    s.var3Role = s.NONE
    s.var4Role = s.NONE
    s.scaleCube = 0
    s.colorType = 1
    s.singleColor = (0, 200, 0, 255)
    s.legendFlag = 0
    s.var1Scaling = x_scaling
    SetPlotOptions(s)
    DrawPlots()


OpenDatabase(DB)

setup_plot(0)
linear_ext = spatial_extents()

setup_plot(1)
log_ext = spatial_extents()

print("=" * 70)
print("VisIt #18984 - non-rendering root-cause check")
print("=" * 70)
print("Linear X scaling, SpatialExtents =", linear_ext)
print("Log    X scaling, SpatialExtents =", log_ext)
print("-" * 70)
if linear_ext and log_ext:
    print("Linear X range : [%g, %g]" % (linear_ext[0], linear_ext[1]))
    print("Log    X range : [%g, %g]  <- log10 of real values"
          % (log_ext[0], log_ext[1]))
    print("")
    print("Axis ticks would label the Log range (small integers), not the")
    print("real data values. See ../notes.md for the proposed fix.")
else:
    print("Could not obtain spatial extents (engine/query issue).")
print("=" * 70)

sys.stdout.flush()
sys.stderr.flush()
os._exit(0)
