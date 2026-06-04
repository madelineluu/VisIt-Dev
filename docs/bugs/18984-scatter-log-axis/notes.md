# Bug #18984 - Scatter plot with Log scaling shows the wrong axis labels

Status: **DIAGNOSED, NOT FIXED (pending).** Root cause is understood and a fix
direction is identified, but the fix could not be implemented and validated in
this build because the compute engine cannot render images here (see
"Rendering failures" below). This file records the diagnosis, the rendering
problems we hit, and exactly what can be resumed later.

GitHub issue: [https://github.com/visit-dav/visit/issues/18984](https://github.com/visit-dav/visit/issues/18984)

---

## 1. What the bug looks like

In a Scatter plot, if you switch a coordinate axis (X, Y, or Z) to **Log**
scaling, the *picture* (the relative spacing of the points) is correct, but the
numeric **axis tick labels are wrong**. You see `0, 1, 2, 3` instead of the real
values `1, 10, 100, 1000`.

The reporter also notes two things:

- A working **workaround**: leave the scatter scaling on **Linear**, then switch
the **2D View** to **Log** and turn **Full Frame On**. That produces both the
correct picture *and* the correct axis labels.
- A separate minor issue: "Full Frame: Auto" does not auto-activate. This is a
different bug and is **deferred** (not addressed here).

---

## 2. Root cause

When a coordinate axis is set to Log, the Scatter filter takes `log10()` of
every coordinate value and stores those logged numbers as the **actual point
coordinates**. The axis annotation system then labels the axes from the data it
actually has - the logged coordinates - so the ticks read `0, 1, 2, 3`.

Concretely, in `src/plots/Scatter/avtScatterFilter.C`:

- The per-axis log transform is applied in `PointMeshFromVariables()` around
lines 807-913 via the macros `LOG10_X`, `LOG10_Y`, `LOG10_Z`:

```807:817:src/plots/Scatter/avtScatterFilter.C
#define LOG10_X(X) log10(X)
#define LOG10_Y(Y) log10(Y)
#define LOG10_Z(Z) log10(Z)

        if(arr3 == 0)
        {
            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                // Scale X coordinate
                if(d1scale == 1)
                    coord[0] = LOG10_X(coord[0]);
```

- The min/max values used for the spatial extents are transformed the same way
(around lines 880-906), so the reported spatial extents are also in log space.
- The code already acknowledges the problem in a comment right after the
transform (around lines 951-956): it notes there is no way to push corrected
labels into the axes because the points themselves have been transformed.

### Why this is fundamentally a viewer-side problem

VisIt already has a correct, general mechanism for log axes, and the Scatter
plot is *bypassing* it:

- The window's 2D / curve **scale mode** (LINEAR vs LOG) is owned by the viewer
and pushed from the view attributes. See
`src/viewer/core/ViewerWindowManager.C`:

```3086:3089:src/viewer/core/ViewerWindowManager.C
    if (updateScaleMode)
    {
        windows[activeWindow]->SetScaleMode(view2d.xScale, view2d.yScale, WINMODE_2D);
    }
```

- When the window scale mode is LOG, `avtPlot` runs `avtMeshLogFilter` to
log-transform the geometry **and** the annotation system labels the axes with
the real values (1, 10, 100). This is the exact path the reporter's workaround
uses (`src/avt/Plotter/avtPlot.C` `SetScaleMode`, and
`src/avt/Filters/avtMeshLogFilter.C`).

So there are two independent log transforms in the code:

1. The **correct** one: viewer view scale (`View2D.xScale = LOG`) ->
  `avtMeshLogFilter` + log-aware axis labels.
2. The **broken** one: the Scatter filter's own in-filter `log10()`, which
  transforms the geometry but never tells the viewer to label logarithmically.

The numeric tick labels are generated at **render time** from the spatial
extents plus the window scale mode. There is **no per-plot "tick label string"
override** in `avtDataAttributes` (the only related setter is
`SetXLabel/SetYLabel`, which sets the axis *title*, not the tick numbers). That
is why a clean fix has to go through the viewer's log-axis machinery rather than
being patched purely inside the plugin.

---

## 3. Fix direction (for whoever resumes this)

Two options were considered (matching the plan):

- **Option A (recommended, correct, but viewer-coordinated):** Stop applying
`log10` to the coordinates inside `avtScatterFilter`. Instead, when a
coordinate uses Log scaling, drive the window's 2D view scale to LOG for that
axis (the same path the workaround uses) so the standard
`avtMeshLogFilter` + annotation code produces both correct spacing and correct
labels. This requires coordinating the Scatter attributes with `View2D`
(`xScale`/`yScale`) in the viewer / GUI - it is not a plugin-only change.
- **Option B (cosmetic, plugin-only):** Keep the in-filter `log10` transform and
relabel the axes to show `10^0, 10^1, ...`. This needs a tick-label override
that does not currently exist, and the plan itself notes it breaks zooming and
the min/max input boxes because everything internally stays in log space.

**Recommendation:** Option A. It reuses code that already works and avoids a
second, conflicting log transform. The smaller sub-steps:

1. In `avtScatterFilter::PointMeshFromVariables`, skip the `LOG10_`* transform
  for any coordinate whose `scale == Log` (keep Skew and ScaleCube paths
   untouched - they are independent and must keep working).
2. In the viewer (`avtScatterPlot` / the scatter GUI window), translate a
  coordinate's Log scaling into `View2D.xScale/yScale = LOG` so the viewer's
   log-axis labeling activates automatically (this is the part the reporter does
   by hand today).
3. Leave the "Full Frame: Auto" auto-activation issue as a separate deferred
  ticket.

---

## 4. Rendering failures encountered (why this is not validated here)

The plan's validation step is image-based (`Test(...)` / before-after PNGs).
**Image rendering does not work in this build**, so #18984 could not be
validated end-to-end. Details so future work does not repeat the investigation:

### Symptom

Running `build/bin/bug_18984.py` (Scatter + Log + `SaveWindow`) via
`./run_visit` produced no PNG and the engine died:

```
VisIt: Message - Rendering window 1...
VisIt: Error - The compute engine running on ix-dev.cs.uoregon.edu has exited abnormally.
VisIt: Error - Scalable Render Request Failed (VisItException)
VisIt: Error - No image was saved.
```

### Root cause of the rendering failure

This is **not** related to the missing GUI. Image creation needs offscreen
*rendering*, not the Qt GUI, and the `cli` front-end replaces the GUI fine.

The real problem is the **engine's** offscreen OpenGL context. From the debug
logs in `build/bin/`:

- The **viewer** brings up offscreen software GL successfully:
`A.viewer.5.vlog:341` -> `render window is a vtkOSOpenGLRenderWindow`,
`GLVersion: 3.3 (Core Profile) Mesa 17.3.9`.
- Every **engine** instance fails to initialize GL:
`A.engine_ser.*.vlog` -> `vtkOSOpenGLRenderWindow: GLEW could not be initialized.` (VTK 8.1.0 `vtkOpenGLRenderWindow.cxx:785`), and when a render
is pushed to the engine it crashes: `A.cli.*.vlog` ->
`signalhandler_core: SIGSEGV! (Segmentation fault)`.

So engine-side (scalable) rendering is broken against this OSMesa 17.3.9 / LLVM
6.0.1 stack (see `build/bin/run_visit` for the `LD_LIBRARY_PATH` it sets), while
viewer-side offscreen rendering works.

### Things to try when resuming (to get images)

1. Force **non-scalable** rendering so the viewer (which works) draws instead of
  the engine. Before `DrawPlots()`/`SaveWindow()`:
2. If `SaveWindow` still routes to the engine, try nudging the software GL stack:
  `LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe`, or
   `MESA_GL_VERSION_OVERRIDE=3.3 MESA_GLSL_VERSION_OVERRIDE=330`.
3. As a longer-term fix, rebuild/point VisIt at a newer OSMesa/LLVM so the engine
  can initialize GLEW.

### CLI gotchas found

- End CLI scripts with VisIt's `Exit()`, **not** `sys.exit()`. `sys.exit()`
leaves the CLI sitting at the interactive `>>>` prompt forever (this is why an
earlier run appeared to "hang" for 8+ minutes).
- `run_visit` already appends `-cli -nowin`, so call it as
`./run_visit -s bug_18984.py` (do not add another `-cli -nowin`).

---

## 5. Non-rendering check that DOES work here

Even without images, the *root cause* is observable without rendering: under Log
scaling the plotted point coordinates are the `log10` values. `bug_18984.py`
includes a `Query("SpatialExtents")` / node `Pick` path that prints the
coordinates so you can see they come back in log space (e.g. an X extent of
`[0, 3]` for data that really spans `[1, 1000]`). This is the reproducible,
no-image signature of the bug. It confirms the diagnosis but does **not** verify
a fix (a fix's effect is in the rendered labels / the view scale mode).

---

## 6. Files

- Diagnosis target: `src/plots/Scatter/avtScatterFilter.C` (lines ~807-913,
~880-906, ~951-956), `src/plots/Scatter/avtScatterPlot.C`.
- Supporting/correct mechanism: `src/avt/Filters/avtMeshLogFilter.C`,
`src/avt/Plotter/avtPlot.C`, `src/viewer/core/ViewerWindowManager.C`.
- Repro / non-image check: `build/bin/bug_18984.py`.
- No source files were modified for this bug.

