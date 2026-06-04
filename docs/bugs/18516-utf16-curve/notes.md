# Issue #18516 - UTF-16 LE curve / ultra files fail to open

GitHub issue: <https://github.com/visit-dav/visit/issues/18516>

## What was broken

When a `.curve` or `.ultra` file is saved with a byte-order mark
(BOM) other than plain ASCII / UTF-8, the Curve2D reader returns
zero curves and the database open fails with an `InvalidFiles`
exception. This is the encoding PowerShell's `cat` produces by
default, so users on Windows hit it whenever they concatenate
ultra files.

## Why it happened

`avtCurve2DFileFormat::ReadFile()` opens the file with
`visit_ifstream` and parses it one 8-bit line at a time
(`getline(line, 256, '\n')`). It does no encoding detection. A
UTF-16 LE file looks like this in raw bytes:

```
FF FE 23 00 20 00 63 00 75 00 72 00 76 00 65 00 ...
   ^^^^^ BOM
            ^ '#'
                  ^ ' '
                        ^ 'c'
                              ...
```

Every visible character is followed by a `0x00` byte. `getline`
reads up to the first newline (`0x0A`), so the first "line" ends
up being `0xFF 0xFE 0x23 0x00 ...`. The 0x00 terminates the C
string at the second character, so the parser sees something like
`"\xFF\xFE#"`, gives up, and produces no curves. UTF-16 BE has
the same problem with the bytes swapped. UTF-8 with BOM is a
milder version of the same thing - the leading
`EF BB BF` becomes spurious characters that defeat the header
parser.

## What I changed

Single-file change in
[src/databases/Curve2D/avtCurve2DFileFormat.C](../../../src/databases/Curve2D/avtCurve2DFileFormat.C):

1. Added a free helper `SlurpAndDecodeCurveStream(std::istream &in)`
   that:
   - reads the whole stream into a `std::string`,
   - detects a leading BOM,
   - if UTF-8 BOM (`EF BB BF`): strips three bytes,
   - if UTF-16 LE BOM (`FF FE`) or BE BOM (`FE FF`): walks the
     remaining 16-bit code units, keeps the low byte when it is
     ASCII (high byte 0 and low byte < 0x80), and substitutes a
     space otherwise,
   - returns the decoded text.
2. `ReadFile()` now slurps the stream once, wraps the decoded
   string in a `std::istringstream`, and runs the existing
   `GetPoint(...)` parsing loop against that stringstream instead
   of the file stream directly. Behavior for normal ASCII /
   UTF-8 files is identical because no BOM is detected and the
   slurped bytes are passed through unchanged.

I went with this fix rather than
modifying `visit_ifstream` itself because (a) it is
fully contained to one file, (b) it does not risk regressing the
many other text plugins that also use `visit_ifstream`, and (c)
the `.curve` / `.ultra` format is the only one mentioned in the
bug report.

## BOM lookup table

| Encoding         | BOM bytes      | Handling                              |
|------------------|----------------|---------------------------------------|
| UTF-8 (no BOM)   | (none)         | passed through unchanged              |
| UTF-8 with BOM   | `EF BB BF`     | BOM stripped, body passed through     |
| UTF-16 LE w/ BOM | `FF FE`        | transcoded to ASCII (low byte kept)   |
| UTF-16 BE w/ BOM | `FE FF`        | transcoded to ASCII (low byte kept)   |

UTF-32 BOMs (`FF FE 00 00`, `00 00 FE FF`) are not handled; that
matches the scope of the original bug report.

## Files touched

- `src/databases/Curve2D/avtCurve2DFileFormat.C` - the fix.
- `repro/bug_18516.py` - standalone repro script (run via `build/bin/run_visit -s ...`).
- `src/test/tests/databases/curve_utf16.py` - nightly regression
  test that writes one file in each of the four encodings,
  records the curve names with `TestText`, and renders one of
  the recovered curves with `Test`.

## Validation

`repro/bug_18516.py` synthesizes one file per encoding and
prints PASS / FAIL per variant.

Before the fix
(`evidence/before.txt`):

```
utf-8                   PASS  curves=['curve_a', 'curve_b']
utf-8-with-bom          FAIL  curves=[]
utf-16-le-with-bom      FAIL  curves=[]
utf-16-be-with-bom      FAIL  curves=[]
RESULT: at least one variant failed -- bug #18516 is present
```

After the fix
(`evidence/after.txt`):

```
utf-8                   PASS  curves=['curve_a', 'curve_b']
utf-8-with-bom          PASS  curves=['curve_a', 'curve_b']
utf-16-le-with-bom      PASS  curves=['curve_a', 'curve_b']
utf-16-be-with-bom      PASS  curves=['curve_a', 'curve_b']
RESULT: all variants opened correctly -- bug #18516 is fixed
```
