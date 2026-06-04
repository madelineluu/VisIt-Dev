# ----------------------------------------------------------------------------
#  MODES: serial parallel
#  CLASSES: nightly
#
#  Test Case:  statistics_query.py
#
#  Tests:      The new ghost-aware "Statistics" query (avtStatisticsQuery),
#              which reports count/min/max/range/sum/mean/stddev/variance/
#              skewness/kurtosis in one pass.  Added for VisIt issue #19952.
#
#  Notes:      Self-contained -- generates a small VTK mesh and uses inline
#              TestValueEQ checks; no Silo data or text baselines required.
#
#  Programmer: VisIt issue #19952
#  Date:       2026
# ----------------------------------------------------------------------------

import os

VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "statistics_query\n"
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

dbname = os.path.join(os.getcwd(), "statistics_query.vtk")
with open(dbname, "w") as fp:
    fp.write(VTK_TEXT)

TestSection("Statistics query (#19952)")

OpenDatabase(dbname)

# zvar = [10,20]: count 2, min 10, max 20, range 10, sum 30, mean 15.
AddPlot("Pseudocolor", "zvar")
DrawPlots()
Query("Statistics")
# Result values order:
#   count,min,max,range,sum,mean,stddev,variance,skewness,kurtosis
vals = GetQueryOutputValue()
TestValueEQ("zvar_count",   vals[0], 2.0)
TestValueEQ("zvar_minimum", vals[1], 10.0)
TestValueEQ("zvar_maximum", vals[2], 20.0)
TestValueEQ("zvar_range",   vals[3], 10.0)
TestValueEQ("zvar_sum",     vals[4], 30.0)
TestValueEQ("zvar_mean",    vals[5], 15.0)

# nvar = [1..6]: mean 3.5, min 1, max 6, range 5, sum 21, count 6.
DeleteAllPlots()
AddPlot("Pseudocolor", "nvar")
DrawPlots()
Query("Statistics")
vals = GetQueryOutputValue()
TestValueEQ("nvar_count",   vals[0], 6.0)
TestValueEQ("nvar_minimum", vals[1], 1.0)
TestValueEQ("nvar_maximum", vals[2], 6.0)
TestValueEQ("nvar_range",   vals[3], 5.0)
TestValueEQ("nvar_sum",     vals[4], 21.0)
TestValueEQ("nvar_mean",    vals[5], 3.5)

Exit()
