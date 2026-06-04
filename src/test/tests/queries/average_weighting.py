# ----------------------------------------------------------------------------
#  MODES: serial parallel
#  CLASSES: nightly
#
#  Test Case:  average_weighting.py
#
#  Tests:      The "Average Value" (size-weighted) vs "Average Value
#              (Unweighted)" queries, and that the weighted-average result text
#              names its weighting basis.  Regression for VisIt issue #19952.
#
#  Notes:      Self-contained -- generates a small rectilinear VTK mesh with
#              deliberately unequal cell sizes so the size-weighted mean differs
#              from the unweighted arithmetic mean.  Uses inline TestValueEQ
#              checks, so it does not depend on Silo data or text baselines.
#
#  Programmer: VisIt issue #19952
#  Date:       2026
# ----------------------------------------------------------------------------

import os

# A 2D rectilinear mesh, XY:
#   X = 0,1,3 -> two cells of width 1 and 2; Y = 0,1 -> one cell tall.
#   2 cells with areas 1 and 2; 6 nodes.
#   zvar (cell)  = [10, 20]
#       unweighted mean    = 15.0
#       area-weighted mean = (10*1 + 20*2)/3 = 16.66667
#   nvar (node)  = [1..6], unweighted mean = 3.5
VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "avg_weighting\n"
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

dbname = os.path.join(os.getcwd(), "average_weighting.vtk")
with open(dbname, "w") as fp:
    fp.write(VTK_TEXT)

TestSection("Average Value weighting (#19952)")

OpenDatabase(dbname)

# Size-weighted average (the historical "Average Value").
AddPlot("Pseudocolor", "zvar")
DrawPlots()
Query("Average Value")
TestValueEQ("zvar_area_weighted_mean", GetQueryOutputValue(), 16.66667)
msg = GetQueryOutputString()
TestValueEQ("zvar_message_states_area_weighted",
            "area-weighted" in msg, True)

# Unweighted arithmetic mean of the same zonal variable.
DeleteAllPlots()
AddPlot("Pseudocolor", "zvar")
DrawPlots()
Query("Average Value (Unweighted)")
TestValueEQ("zvar_unweighted_mean", GetQueryOutputValue(), 15.0)

# Unweighted arithmetic mean of a nodal variable.
DeleteAllPlots()
AddPlot("Pseudocolor", "nvar")
DrawPlots()
Query("Average Value (Unweighted)")
TestValueEQ("nvar_unweighted_mean", GetQueryOutputValue(), 3.5)

Exit()
