# ----------------------------------------------------------------------------
#  MODES: serial parallel
#  CLASSES: nightly
#
#  Test Case:  statistics_ghosts.py
#
#  Tests:      That the statistics queries exclude ghost cells.  Regression for
#              VisIt issue #19952 (the Sample/Population Statistics queries used
#              to double-count ghost cells, skewing the mean/variance).
#
#  Notes:      Self-contained -- generates a VTK mesh with one cell flagged as
#              ghost via the standard vtkGhostType array (which VisIt maps to
#              avtGhostZones).  Uses inline TestValueEQ checks.
#
#  Programmer: VisIt issue #19952
#  Date:       2026
# ----------------------------------------------------------------------------

import os

# 3 cells, zvar = [10, 20, 30]; the third cell is a ghost.
#   ghost-EXCLUDED mean = (10 + 20) / 2 = 15   (correct, post-fix)
#   ghost-INCLUDED mean = (10 + 20 + 30) / 3 = 20  (the old, wrong value)
VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "statistics_ghosts\n"
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

dbname = os.path.join(os.getcwd(), "statistics_ghosts.vtk")
with open(dbname, "w") as fp:
    fp.write(VTK_TEXT)

TestSection("Statistics queries exclude ghost cells (#19952)")

OpenDatabase(dbname)
AddPlot("Pseudocolor", "zvar")
DrawPlots()

# Sample Statistics: values are mean,stddev,variance,skewness,kurtosis.
Query("Sample Statistics")
TestValueEQ("sample_stats_mean_ghost_excluded",
            GetQueryOutputValue()[0], 15.0)

# New Statistics query: values[0]=count, values[5]=mean.
Query("Statistics")
sv = GetQueryOutputValue()
TestValueEQ("statistics_count_ghost_excluded", sv[0], 2.0)
TestValueEQ("statistics_mean_ghost_excluded",  sv[5], 15.0)

# The unweighted average also excludes the ghost cell.
Query("Average Value (Unweighted)")
TestValueEQ("unweighted_mean_ghost_excluded", GetQueryOutputValue(), 15.0)

Exit()
