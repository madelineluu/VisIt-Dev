# ---------------------------------------------------------------------------
#  Repro script for VisIt issue #18516
#  "Unable to read UTF-16 Little-endian ultra files"
#
#  Run from build/bin/ with:
#      ./run_visit -s ../../docs/bugs/18516-utf16-curve/repro/bug_18516.py
#  (run_visit already supplies -cli -nowin; do NOT add them again)
#
#  Before the fix: only the UTF-8 variant opens correctly.
#  After  the fix: all four encoding variants open with the same curve names.
# ---------------------------------------------------------------------------

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(SCRIPT_DIR, "output")
os.makedirs(OUT_DIR, exist_ok=True)

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


def write_variant(name, encoding, bom=b""):
    path = os.path.join(OUT_DIR, name)
    with open(path, "wb") as fp:
        fp.write(bom)
        fp.write(CURVE_TEXT.encode(encoding))
    return path


def list_curves(path):
    OpenDatabase(path)
    md = GetMetaData(path)
    names = []
    for i in range(md.GetNumCurves()):
        names.append(md.GetCurves(i).name)
    CloseDatabase(path)
    return names


def check(label, path, expected):
    try:
        got = list_curves(path)
    except Exception as e:
        print("%-22s  FAIL  exception: %s" % (label, e))
        return False
    ok = got == expected
    status = "PASS" if ok else "FAIL"
    print("%-22s  %s  curves=%s" % (label, status, got))
    return ok


variants = [
    ("utf-8",              "bug_18516_utf8.ultra",       "utf-8",     b""),
    ("utf-8-with-bom",     "bug_18516_utf8_bom.ultra",   "utf-8",     b"\xEF\xBB\xBF"),
    ("utf-16-le-with-bom", "bug_18516_utf16le.ultra",    "utf-16-le", b"\xFF\xFE"),
    ("utf-16-be-with-bom", "bug_18516_utf16be.ultra",    "utf-16-be", b"\xFE\xFF"),
]

expected = ["curve_a", "curve_b"]

all_ok = True
for label, name, enc, bom in variants:
    path = write_variant(name, enc, bom)
    ok = check(label, path, expected)
    all_ok = all_ok and ok

if all_ok:
    print("RESULT: all variants opened correctly -- bug #18516 is fixed")
else:
    print("RESULT: at least one variant failed -- bug #18516 is present")

sys.stdout.flush()
sys.stderr.flush()
os._exit(0 if all_ok else 1)
