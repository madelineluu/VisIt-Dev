# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cycle_from_filename.py
#
#  Tests:      databases - VTK
#              behavior  - cycle numbers guessed from file names in a series
#
#  Purpose:    Regression test for VisIt issue #20096
#              ("avtFileFormat::GuessCycle() called for every file name in a
#              series except the first"). When a series of files encodes its
#              cycle number in the file name (series_0100.vtk, series_0200.vtk,
#              series_0300.vtk) the time slider must report the guessed cycle
#              for EVERY state, including the first. Before the fix the first
#              state came back as 0 because the format interface's capability
#              probe (GetCycleFromFilename("")) clobbered the first timestep's
#              cached cycle.
#
#  Programmer: Madeline Luu
#  Date:       June 2, 2026
# ----------------------------------------------------------------------------
import os

# Minimal ASCII legacy VTK structured-points file with one nodal scalar.
VTK_TEXT = (
    "# vtk DataFile Version 3.0\n"
    "cycle_from_filename\n"
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

# Pick a scratch directory: the test framework's run dir if available,
# otherwise the current working directory.
scratch = os.getcwd()
try:
    scratch = TestEnv.params["run_dir"]  # noqa: F821 - injected by harness
except Exception:
    pass


def write_series():
    for n in NAMES:
        with open(os.path.join(scratch, n), "w") as fp:
            fp.write(VTK_TEXT)
    visit_path = os.path.join(scratch, "cycle_from_filename.visit")
    with open(visit_path, "w") as fp:
        for n in NAMES:
            fp.write(os.path.join(scratch, n) + "\n")
    return visit_path


db = write_series()
OpenDatabase(db)
md = GetMetaData(db)
cycles = [md.cycles[i] for i in range(len(NAMES))]
CloseDatabase(db)

summary = "filename-cycle series\n"
summary += "expected cycles : %s\n" % EXPECTED
summary += "reported cycles : %s\n" % cycles
summary += "match           : %s\n" % (cycles == EXPECTED)

TestText("cycle_from_filename_metadata", summary)

Exit()
