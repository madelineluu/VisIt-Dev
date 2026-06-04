# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  curve_utf16.py
#
#  Tests:      databases - Curve2D
#              encodings - UTF-8, UTF-8 with BOM, UTF-16 LE, UTF-16 BE
#
#  Purpose:    Regression test for VisIt issue #18516. The Curve2D reader
#              must successfully open ultra/curve files saved with any of
#              the common text-file encodings. This test writes one tiny
#              two-curve file in each encoding to a scratch directory and
#              verifies the curve list reported by the database metadata
#              matches the UTF-8 baseline.
#
#  Programmer: Madeline Luu
#  Date:       June 1, 2026
# ----------------------------------------------------------------------------
import os

CURVE_TEXT = (
    "# curve_a\n"
    "0 0\n"
    "1 1\n"
    "2 4\n"
    "3 9\n"
    "# curve_b\n"
    "0 0\n"
    "1 2\n"
    "2 4\n"
    "3 6\n"
)

# Pick a scratch directory: the test framework's run dir if available,
# otherwise the current working directory.
scratch = os.getcwd()
try:
    scratch = TestEnv.params["run_dir"]  # noqa: F821 - injected by harness
except Exception:
    pass


def write_variant(name, encoding, bom=b""):
    path = os.path.join(scratch, name)
    with open(path, "wb") as fp:
        fp.write(bom)
        fp.write(CURVE_TEXT.encode(encoding))
    return path


def describe(path):
    """Open `path`, return one line summarizing the curves we got."""
    OpenDatabase(path)
    md = GetMetaData(path)
    names = [md.GetCurves(i).name for i in range(md.GetNumCurves())]
    CloseDatabase(path)
    return "%s -> %s" % (os.path.basename(path), names)


variants = [
    ("utf8",        "curve_utf16_utf8.ultra",     "utf-8",     b""),
    ("utf8_bom",    "curve_utf16_utf8_bom.ultra", "utf-8",     b"\xEF\xBB\xBF"),
    ("utf16_le",    "curve_utf16_le.ultra",       "utf-16-le", b"\xFF\xFE"),
    ("utf16_be",    "curve_utf16_be.ultra",       "utf-16-be", b"\xFE\xFF"),
]

lines = []
for label, name, enc, bom in variants:
    path = write_variant(name, enc, bom)
    try:
        lines.append(describe(path))
    except Exception as e:
        lines.append("%s -> EXCEPTION: %s" % (os.path.basename(path), e))

TestText("curve_utf16_metadata", "\n".join(lines) + "\n")

# Also draw one of the recovered curves to prove the points round-tripped.
path = os.path.join(scratch, "curve_utf16_le.ultra")
OpenDatabase(path)
AddPlot("Curve", "curve_a")
DrawPlots()
ResetView()
Test("curve_utf16_le_plot")

Exit()
