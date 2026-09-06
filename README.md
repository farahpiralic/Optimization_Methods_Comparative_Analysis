# OptimCompare

**Project #16 - Comparative Analysis of First- and Second-Order Unconstrained
Optimization Methods**
Student: F. Piralić, index 20106
Framework: natID (C++), build system: CMake

The application implements **steepest descent**, **Newton's method** and
**BFGS** from scratch, each combined with two step-size strategies
(**exact line search** and **Armijo backtracking**), and compares them on a
benchmark suite of strongly convex, ill-conditioned quadratic and non-convex
(saddle-point) functions. The GUI shows the three panels from the proposal
simultaneously: a contour plot with the optimization trajectories, a
log-scale convergence curve of ||grad f(x_k)||, and the condition-number
study (iterations vs kappa).

---

## 1. Project structure

```
OptimCompare/
|-- CMakeLists.txt        solution file (standard natID convention)
|-- optCompare.cmake      the single GUI target
|-- res/
|   |-- DevRes.xml        languages + resource configs (EN / BA)
|   |-- main.xml          toolbar icons (from the SDK common set)
|   `-- tr/EN, tr/BA      translations for every UI string
`-- src/                  ONE flat source folder (like the SDK examples)
    |-- Objective.h       objective interface + the 7 benchmark functions
    |-- LineSearch.h      exact line search + Armijo backtracking
    |-- Optimizer.h       SD / Newton / BFGS, shared driver, iterate traces
    |-- CondStudy.h       kappa = 1 ... 1e4 study for the third panel
    |-- ContourCanvas.h   contour plot (marching squares) + trajectories
    |-- ChartCanvas.h     line chart with optional log axes (2 instances)
    |-- ControlsView.h    left control panel (proposal section 4.1)
    |-- MainView.h        nested splitters, runs the solvers, feeds panels
    |-- MenuBar.h / ToolBar.h / Constants.h
    |-- ViewSettings.h / DialogSettings.h   language switcher (EN/BA)
    |-- MainWindow.h / Application.h / main.cpp
    |-- Info.plist / macApp.entitlements    macOS bundle files
```

Everything algorithmic lives in the first four headers and has **no GUI
dependencies** (only `dense::Matrix` from the natID MatrixLib), so the
optimizer backend and the GUI frontend are cleanly separated, exactly as the
proposal describes.

## 2. Building

The natID SDK must be reachable at `~/natID.SDK` and the RAM-disk working
folder at `~/natID.RAMDisk` (standard course setup). If your SDK lives
elsewhere, a symlink is enough:

```
ln -s /path/to/natID.SDK   ~/natID.SDK
ln -s /path/to/natID.RAMDisk ~/natID.RAMDisk
```

### macOS (Xcode)
```
cd OptimCompare
cmake -G Xcode -B ~/natID.RAMDisk/build/optCompare .
open ~/natID.RAMDisk/build/optCompare/optCompareSol.xcodeproj
```
Build and run the `optCompare` scheme (Debug or Release).

### Windows (Visual Studio)
```
cd OptimCompare
cmake -G "Visual Studio 17 2022" -B %USERPROFILE%\natID.RAMDisk\build\optCompare .
```
Open the generated `optCompareSol.sln`, set `optCompare` as the startup
project, build and run. (`%USERPROFILE%` plays the role of `~`; the solution
CMakeLists handles the path conversion automatically.)

### Linux
```
cd OptimCompare
cmake -B ~/natID.RAMDisk/build/optCompare .
cmake --build ~/natID.RAMDisk/build/optCompare
```
Note: configuring and compiling works on any platform, but the **all-in-one
SDK archive used during development ships macOS arm64 binaries only**, so the
final *link* step succeeds only on macOS unless you have a Linux/Windows
build of the SDK libraries. All sources are platform-neutral (no OS-specific
calls; `M_PI` is guarded for MSVC).

If CMake complains about a stale cache after moving/renaming the project
folder, delete `~/natID.RAMDisk/build/optCompare` and configure again.

## 3. Using the application

The window starts with the first benchmark selected and all three methods
already executed, so all panels are populated immediately.

**Control panel (left)**
* *Function* - picks one of the 7 benchmarks (categories A/B/C below).
  Changing the function loads its recommended starting point, re-frames the
  contour view and re-runs automatically.
* *Method* - steepest descent, Newton, BFGS, or **All three (compare)**
  (default), which overlays all trajectories and convergence curves.
* *Step-size strategy* - exact line search or Armijo backtracking; applies
  to every method identically.
* *x0, Max iterations, Tolerance* - the run parameters from the proposal.
* *Animate steps* (on by default) - reveals the results step by step
  instead of showing the finished picture: the trajectories draw themselves
  iterate by iterate with a moving dot at each method's current position,
  and the convergence curves grow in sync. Untick for instant results.
* *Run* - executes and refreshes the contour + convergence panels and the
  log (iterations, final f, final ||g||, status, Newton fallbacks, time).
* *Run kappa study* - fills the third panel (details in section 5).
* *Reset* - clean slate: clears the trajectories, the contour display, both
  charts and the log, exactly like the panels before anything ran. Press
  Run (or pick a function, or right-click the canvas) to start fresh; the
  next run also re-frames the contour view, so Reset followed by Run is
  the way back after panning or zooming far away.

**Contour panel (top right)**
* drag = pan, scroll/pinch = zoom, **right-click = set a new x0 and re-run**
  (fastest way to explore basins of attraction, e.g. on Himmelblau).
* Gold circles mark known minima, gray crosses mark saddle points, the green
  dot is x0, each method's path ends with a ring once it has fully played
  out. During the animation each method shows a moving dot at its current
  iterate - Newton visibly arrives first, then BFGS, while steepest descent
  is still zigzagging.
* Contour levels are quantiles of the visible values, so both flat quadratic
  bowls and Rosenbrock's extreme range render sensibly; levels recompute on
  every pan/zoom.

**Convergence panel (bottom left)** - ||grad f(x_k)|| per iteration on a
logarithmic y axis, one line per method: linear convergence appears as a
straight line, Newton's quadratic convergence as a steep drop.

**Condition panel (bottom right)** - iterations to reach ||g|| <= 1e-6 vs
condition number kappa, log-log: steepest descent is a straight line of
slope ~1 (iterations grow linearly with kappa), Newton and BFGS stay flat.

Settings (gear icon) switch the UI language (English / Bosanski).

## 4. Methods (what is implemented)

All methods share the update x_{k+1} = x_k + alpha_k d_k and stop when
||grad f|| <= eps (or maxIter / stall / divergence, reported in the log).

* **Steepest descent**: d = -grad f.
* **Newton**: solves H d = -grad f with `dense::Matrix::solve` (the
  framework's dense LU). If H is singular or indefinite so that d is not a
  descent direction (which happens near saddle points - see the double-well
  benchmark), the step falls back to -grad f and the iteration is flagged;
  fallback counts appear in the log and the contour legend.
* **BFGS**: maintains the inverse-Hessian approximation B (starting from I)
  with the standard update B+ = (I - r s y^T) B (I - r y s^T) + r s s^T,
  r = 1/(y^T s), skipping updates that violate the curvature condition
  y^T s > 0.
* **Exact line search**: closed form alpha = -(g^T d)/(d^T H d) on
  quadratics; expanding bracket + golden-section refinement otherwise.
* **Armijo backtracking**: alpha0 = 1, rho = 1/2, c1 = 1e-4 (Nocedal &
  Wright defaults).

**Benchmarks** (ordered as in the proposal):
A) convex quadratic (kappa = 2), strongly convex quadratic + exponential;
B) ill-conditioned quadratics (kappa = 100 rotated 15 deg, kappa = 2000
rotated 30 deg);
C) double well (saddle at the origin - the Newton-fallback demo),
Rosenbrock, Himmelblau (4 minima).

## 5. The worst-case starting point (why x0 looks "special")

A subtle and important experimental detail: on a quadratic, steepest descent
from a *generic* start can converge deceptively fast even for huge kappa,
because the slow eigen-mode is barely excited (verified numerically: at
kappa = 1e4 the rotated quadratic converges in 7 exact-line-search steps
from (1,1)). The classical rate ((kappa-1)/(kappa+1))^2 per step - the
famous zigzag - appears only from the **eigen-balanced worst case**, with
eigencoordinates (a, a/kappa) so both gradient components are equal.
`Quadratic2D::suggestedStart()` therefore returns exactly this point, and
the condition study uses it for every kappa. That is what makes the third
panel reproduce the textbook picture:

| kappa = 1e4, exact LS | iterations |
|---|---|
| steepest descent | 74 277 |
| Newton | 1 |
| BFGS | 2 |

(BFGS terminating in ~n steps on an n-dimensional quadratic with exact line
search, and Newton in one step, are the classical finite-termination
results.)

## 6. Validation

The algorithmic core was verified with a standalone test harness (73 checks,
all passing) against known analytic results, independently cross-checked
with NumPy:

* Newton converges in <= 2 iterations on every quadratic, any kappa, both
  strategies; BFGS in <= 4 with exact LS.
* Rosenbrock from (-1.2, 1): Newton ~8 iterations, BFGS ~35, steepest
  descent ~8000 (Armijo) - the classic ordering.
* Himmelblau from (0,0): all methods reach a minimum with f* ~ 0.
* Double well from (0.08, 1.6): the Newton descent safeguard fires near the
  saddle and the method still converges to a well.
* Strongly convex quad+exp: all three methods agree on x* to 1e-8 with the
  1D-Newton reference root.
* Condition study: SD iterations grow linearly with kappa (74 277 at 1e4,
  matching NumPy exactly), Newton flat at 1, BFGS flat at 2.
* Full GUI compiles cleanly (`-fsyntax-only`, C++20) against the unmodified
  SDK headers; `-Wall -Wextra` reports nothing in project sources.

## 7. Implementation notes

* The step animation uses the framework's Canvas animation frames
  (`startAnimation()` / `stopAnimation()`): while active, `onDraw` is called
  at 30-60 fps and simply draws a time-based prefix of each recorded trace.
  Each method reveals at ~240 ms per iteration, clamped between 1.5 s and
  8 s per run (constants kRevealMsPerStep / kRevealMinMs / kRevealMaxMs at
  the top of ContourCanvas.h and ChartCanvas.h if you want a different
  tempo), so single Newton steps are readable while thousands of
  steepest-descent steps stream smoothly. The solvers themselves still
  finish instantly - only the drawing is progressive, so no timers or
  worker threads are involved and the GUI stays fully responsive.
* The condition study runs *synchronously* on the GUI thread: all 27 runs
  (9 kappa values x 3 methods, up to ~75k iterations each) finish in under
  10 ms, so no worker threads are needed anywhere.
* Contours: marching squares on a 101x101 grid over the visible rectangle,
  12 quantile levels, ambiguous cases resolved by the cell-center average;
  segments are cached in world coordinates and recomputed only when the
  function or the view changes.
* Iterate traces store (x, f, ||g||, alpha, fallback-flag) per iteration and
  drive both the trajectory overlay and the convergence curves; the study
  disables trace recording since only iteration counts are needed.
