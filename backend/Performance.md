# Babylon.js Performance & Browser Freeze Analysis

## System & Environment Profile

| Component    | Specification                         |
| :----------- | :------------------------------------ |
| **GPU**      | NVIDIA GeForce RTX 4080               |
| **CPU**      | Intel Core i9 (13th Gen)              |
| **RAM**      | 16 GB DDR5                            |
| **Storage**  | 1 TB SSD                              |
| **OS**       | Windows 11 / 10                       |
| **Browsers** | Brave, Microsoft Edge (Chromium Core) |

---

## Preliminary Hardware Assessment

> **Conclusion:** **No hardware bottleneck exists.**
>
> An Intel Core i9 13th Gen paired with an RTX 4080 and DDR5 memory provides massive compute and rasterization throughput. This hardware configuration is capable of rendering high-complexity 3D scenes at native frame rates.
>
> The symptoms described (**"Page Not Responding"**, freeze on scenario selection, frame drops) indicate **software-level bottlenecks**: main-thread blocking in JavaScript, improper graphics layer configuration, or inefficient asset/DOM management.

---

## 🛑 COMPLETE LINE-BY-LINE PROBLEM AUDIT

Every single performance problem found in `visualizer_template.html`, organized by root cause category.

---

### CATEGORY 1 — Canvas & CSS Isolation (Missing GPU Layer Promotion)

#### P-001 · `#renderCanvas` missing `will-change: transform` (L25-29)
```css
#renderCanvas {
  width: 100vw; height: 100vh;
  position: absolute; top: 0; left: 0; z-index: 1;
  touch-action: none; outline: none;
}
```
**Problem:** Without `will-change: transform`, Chromium does NOT promote the canvas to its own compositor layer. Every DOM reflow from the React overlay forces the browser to re-composite the canvas alongside the HTML, blocking the GPU pipeline.

**Fix:** Add `will-change: transform;` to force a discrete compositor layer.

---

#### P-002 · `#renderCanvas` missing `contain: strict` (L25-29)
**Problem:** Without `contain: strict`, CSS layout/paint/style calculations from parent DOM elements (React HUD panels) bleed into the canvas element's layout box. This causes layout thrashing every time the React HUD updates text content.

**Fix:** Add `contain: strict;` to fully isolate canvas layout and paint calculations from the parent DOM tree.

---

#### P-003 · `#renderCanvas` missing `user-select: none` (L25-29)
**Problem:** The canvas can still trigger text selection handling in some Chromium edge cases (drag interactions), adding unnecessary event processing overhead.

**Fix:** Add `user-select: none;` to the canvas rule.

---

#### P-004 · `.glass-panel` forces `backdrop-filter: blur(24px)` on EVERY HUD panel (L31)
```css
backdrop-filter: blur(24px);
```
**Problem:** `backdrop-filter: blur()` is one of the most GPU-expensive CSS properties. It forces a full-area pixel readback + Gaussian blur on every frame for EACH `.glass-panel` element (there are **4** visible simultaneously: top-left HUD, top-right controls, bottom-right diagnostics, bottom carousel). Combined with `transform: skewX(-2deg)` on L34, this creates **4 separate compositing passes per frame**, each requiring a full-screen readback from the canvas underneath.

**Fix:** Reduce blur radius (e.g. `blur(8px)`), or replace with a solid semi-transparent background on low-end scenarios. Consider using `backdrop-filter` only on the 1-2 most important panels.

---

#### P-005 · `.glass-panel` `transform: skewX(-2deg)` forces anti-aliased compositing (L34)
```css
transform: skewX(-2deg);
```
**Problem:** The non-axis-aligned transform forces Chromium's compositor to use anti-aliased rendering for every `.glass-panel` element. Combined with `backdrop-filter: blur(24px)`, this doubles the compositing cost because the blurred region must be sampled at non-integer pixel boundaries.

**Fix:** This is cosmetic. If performance is critical, remove the skew or apply it only to the panel border, not the whole panel.

---

#### P-006 · `.glass-panel:hover` triggers expensive re-composite on mouse hover (L37-40)
```css
.glass-panel:hover {
  border-color: rgba(56, 189, 248, 0.65);
  box-shadow: inset 0 0 30px rgba(56,189,248,0.35), 0 20px 50px rgba(0,0,0,0.85);
}
```
**Problem:** Hovering any HUD panel triggers a CSS transition (L35: `transition: border-color 0.3s ease, box-shadow 0.3s ease`) which forces the compositor to animate `box-shadow` — a paint-triggering property. On 144Hz monitors this runs 144 transition frames per hover event while the 3D scene is also rendering.

**Fix:** Use `transition: border-color 0.3s ease` only. Remove `box-shadow` from the transition, or replace `box-shadow` animation with `filter: drop-shadow()` which can be compositor-accelerated.

---

#### P-007 · `animate-pulse` on multiple elements runs infinite CSS animations (L46-51, L160, L263, L271)
```css
.pulse-active { animation: pulseGlow 1.5s infinite; }
```
Used on: L160 (`.animate-pulse` Tailwind class), L263 (`.animate-pulse`), L271 (`.animate-ping`).

**Problem:** Infinite CSS `box-shadow` animations (`pulseGlow`) are paint-triggering. `animate-ping` from Tailwind uses `transform: scale(2)` + `opacity: 0` which is compositor-friendly, but `pulseGlow` animates `box-shadow` which is NOT. Every 1.5s cycle forces a full paint of the affected element.

**Fix:** Replace `box-shadow` animation with `filter: drop-shadow()` or `outline` which are cheaper, or use `opacity` animation instead.

---

### CATEGORY 2 — Script Loading & Parse Blocking

#### P-008 · Development builds of React loaded in production (L12-13)
```html
<script src="https://unpkg.com/react@18/umd/react.development.js" crossorigin></script>
<script src="https://unpkg.com/react-dom@18/umd/react-dom.development.js" crossorigin></script>
```
**Problem:** `react.development.js` and `react-dom.development.js` include extensive runtime validation, prop-type checking, and console warnings that do NOT exist in production builds. This adds **~30-40%** overhead to every React render cycle (every `setFrameData` call).

**Fix:** Use `react.production.min.js` and `react-dom.production.min.js`.

---

#### P-009 · Babel standalone loaded for in-browser JSX transpilation (L15)
```html
<script src="https://unpkg.com/@babel/standalone/babel.min.js"></script>
```
**Problem:** Babel standalone is **~800KB** and performs full AST parsing + code generation of the `<script type="text/babel">` block at page load. This blocks the main thread for **200-500ms** on first load while it transpiles JSX to JavaScript. The transpiled output is also slower because Babel's runtime helpers add overhead to every function call.

**Fix:** Pre-compile the JSX to plain JavaScript at build time and serve the compiled output directly. Remove Babel standalone entirely.

---

#### P-010 · Tailwind CSS CDN loaded as render-blocking script (L9)
```html
<script src="https://cdn.tailwindcss.com"></script>
```
**Problem:** The Tailwind CDN script scans the entire DOM for utility classes and generates CSS dynamically. This is a **development-only** tool that adds ~100ms of parse time and runs a MutationObserver on the DOM, reacting to every DOM change React makes. On every React re-render, the Tailwind observer re-scans changed elements for new utility classes.

**Fix:** Pre-build Tailwind CSS to a static `.css` file at compile time. Or at minimum, add `<script>tailwind.config = { corePlugins: { preflight: false } }</script>` to reduce scan scope.

---

#### P-011 · Google Fonts loaded without `preconnect` (L17-20)
```html
<link href="https://fonts.googleapis.com/css2?family=..." rel="stylesheet" />
```
**Problem:** Missing `<link rel="preconnect" href="https://fonts.googleapis.com">` and `<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>`. The browser must wait for DNS+TLS to `fonts.googleapis.com`, download the CSS, parse it, then initiate a second connection to `fonts.gstatic.com` for the actual font files. This adds 200-400ms of render blocking.

**Fix:** Add preconnect hints before the font stylesheet link.

---

#### P-012 · All `<script>` tags are render-blocking (L9, L12, L13, L15, L316, L319)
**Problem:** None of the external scripts use `async` or `defer`. The browser must download and execute each script sequentially before parsing subsequent HTML. Total blocking: React (~130KB) + ReactDOM (~1MB dev) + Babel (~800KB) + Babylon.js (~4MB) + ECharts (~1MB) = **~7MB of synchronous parse-blocking JavaScript**.

**Fix:** Add `defer` to scripts that don't need immediate execution. Load Babylon.js and ECharts with `defer` since they're only needed after DOM is ready.

---

### CATEGORY 3 — React HUD Layout Thrashing

#### P-013 · `setFrameData` creates a new object spread on every call (L99)
```javascript
setFrameData: (data) => setFrameData(prev => ({...prev, ...data})),
```
**Problem:** Every call to `setFrameData` creates a new object via spread, which triggers React's reconciliation. Even with the 50ms throttle at L804, this still runs ~20 times/second. Each call spreads **28+ properties** (L805-L816), creating garbage objects for V8 to collect.

**Fix:** Use `useRef` for frame data that doesn't need to trigger re-renders, or batch updates with `unstable_batchedUpdates`.

---

#### P-014 · `renderStatRow` creates new JSX elements for ALL 68 STAT_KEYS on every render (L111-125, L285)
```javascript
{STAT_KEYS.map(key => renderStatRow(key))}
```
**Problem:** `STAT_KEYS` has **68 entries** (L67-86). Every time `setFrameData` or `setActiveScenarioIdx` triggers a re-render, React creates 68 new JSX element trees, diffs them against the previous 68, and patches the DOM. The `renderStatRow` function is not memoized.

**Fix:** Wrap `renderStatRow` in `React.memo` or `useMemo`. Since static scenario data doesn't change during animation, these rows should never re-render during playback.

---

#### P-015 · `renderTransientRow` creates new JSX elements for ALL 21 TRANSIENT_KEYS on every render (L132-145, L287)
```javascript
{TRANSIENT_KEYS.map(key => renderTransientRow(key))}
```
**Problem:** Same as P-014 but for 21 transient keys. These DO change every frame, but the render function is not memoized and creates new string concatenations (`val.toString()`) and template literal class strings on every call.

**Fix:** Memoize, or better: update transient values via direct DOM manipulation (`textContent`) instead of React state, since these are simple text updates.

---

#### P-016 · `scenarios.map()` in carousel recreates ALL scenario buttons on every render (L295-304)
```javascript
{scenarios.map((s, i) => (
  <button key={i} onClick={() => handleScenarioClick(i)} ...>
```
**Problem:** Every render creates new arrow function closures for `onClick` on every scenario button. This prevents React from skipping re-renders because the function reference changes every time.

**Fix:** Use `useCallback` with the index, or a single delegated click handler.

---

#### P-017 · `regimeText` computation runs on every render, not memoized (L147-149)
```javascript
let regimeText = currentData.is_kinetic ? "HYPERVELOCITY KINETIC ROD" : ...
```
**Problem:** This runs on every re-render (20Hz from `setFrameData`), but `currentData` only changes on scenario selection. Minor but contributes to unnecessary computation during animation frames.

**Fix:** Wrap in `useMemo(() => ..., [activeScenarioIdx])`.

---

#### P-018 · `useCallback` and `useMemo` imported but never used (L64)
```javascript
const { useState, useEffect, useCallback, useMemo } = React;
```
**Problem:** `useCallback` and `useMemo` are imported but never applied to any of the handler functions or render functions. This is the exact situation they were designed for.

**Fix:** Apply `useCallback` to `handleScenarioClick`, `handleCameraMove`, `toggleFollowCamBtn`, `changeSpeed`. Apply `useMemo` to `renderStatRow` results and `regimeText`.

---

### CATEGORY 4 — Babylon.js Engine Configuration

#### P-019 · Engine constructor missing `preserveDrawingBuffer: false` (L386)
```javascript
engine = new BABYLON.Engine(canvas, false, { powerPreference: "high-performance" }, true);
```
**Problem:** When `preserveDrawingBuffer` is not explicitly `false`, some Chromium versions default to `true`, which prevents the GPU from discarding the backbuffer after presentation. This forces a full-frame copy on every swap, doubling VRAM bandwidth usage.

**Fix:** Add `preserveDrawingBuffer: false` to the options object.

---

#### P-020 · Engine constructor missing `doNotHandleTouchAction: true` (L386)
**Problem:** Without this flag, Babylon.js intercepts all touch events and applies CSS `touch-action` manipulation on the canvas, triggering style recalculations on every pointer event.

**Fix:** Add `doNotHandleTouchAction: true` since touch-action is already set in CSS (L28).

---

#### P-021 · Engine constructor missing `stencil: false` (L386)
**Problem:** The stencil buffer is allocated by default but never used in this scene (no stencil-based effects like masks or outlines). This wastes 8 bits per pixel of GPU memory.

**Fix:** Add `stencil: false` to disable stencil buffer allocation.

---

#### P-022 · Parallel shader compilation not explicitly enabled (missing after L386)
**Problem:** `engine.getCaps().parallelShaderCompile` is never checked or utilized. When available, this WebGL extension allows shader compilation to happen in parallel on a background thread, preventing the main-thread freeze during scenario selection when new materials are compiled.

**Fix:** Add `if (engine.getCaps().parallelShaderCompile) { /* already enabled by the extension */ }` as a validation check, and ensure the engine is configured to use it.

---

### CATEGORY 5 — Static Mesh Overhead (Missing freeze calls)

#### P-023 · 202 grid line meshes never frozen (L468-484, `setupEnvironment`)
```javascript
for (let i = 0; i <= divs; i++) {
  BABYLON.MeshBuilder.CreateLines("gH"+i, ...);
  BABYLON.MeshBuilder.CreateLines("gV"+i, ...);
}
```
**Problem:** Creates **202 line meshes** (101 horizontal + 101 vertical). None of these are ever moved, rotated, or scaled, yet Babylon.js recalculates their world matrices every frame because `freezeWorldMatrix()` is never called. At 202 meshes x matrix calculation per frame = **202 wasted matrix multiplications per frame**.

**Fix:** After the loop, iterate all grid meshes and call `mesh.freezeWorldMatrix()`. Also call `mesh.isPickable = false` to exclude them from picking ray calculations.

---

#### P-024 · Grid line meshes are individually created instead of merged (L468-484)
**Problem:** 202 separate draw calls for static grid lines. Each mesh is a separate draw call to the GPU, creating massive CPU-side overhead from WebGL state changes.

**Fix:** Merge all grid lines into a single `BABYLON.MeshBuilder.CreateLineSystem()` call, then freeze the single resulting mesh.

---

#### P-025 · Ruler meshes never frozen after creation (L506-557, `rebuildRulers`)
**Problem:** `rebuildRulers` creates **150+ cylinder meshes** (tick marks) + **30+ text sprite planes** + **2 line meshes**. None are frozen. All undergo per-frame world matrix recalculation despite being static.

**Fix:** After building all ruler meshes, call `mesh.freezeWorldMatrix()` and `mesh.isPickable = false` on each.

---

#### P-026 · Text sprites create individual `DynamicTexture` + `StandardMaterial` per label (L487-503)
```javascript
function createTextSprite(msg, colorHex) {
  const dt = new BABYLON.DynamicTexture("dt"+uid, {width:256, height:128}, scene, false);
  ...
  const mat = new BABYLON.StandardMaterial("lblm"+uid, scene);
```
**Problem:** Each text sprite allocates a separate 256x128 GPU texture AND a separate material. With ~30 ruler labels, that's **30 texture units + 30 materials + 30 draw calls** for static text. Each `DynamicTexture` also creates a hidden HTML canvas element in the DOM.

**Fix:** Use a single texture atlas for all ruler labels, or use SPS (SolidParticleSystem) with a shared material.

---

#### P-027 · Ruler text sprite materials never frozen (L497-499)
```javascript
mat.disableLighting = true; mat.backFaceCulling = false;
mat.useAlphaFromDiffuseTexture = true;
```
**Problem:** `material.freeze()` is never called on any text sprite material. Babylon.js re-evaluates material uniforms every frame for unfrozen materials.

**Fix:** Call `mat.freeze()` after setting all material properties.

---

#### P-028 · Lighting objects never excluded from unnecessary meshes (L444-465)
**Problem:** The `HemisphericLight`, `DirectionalLight`, and `PointLight` illuminate ALL meshes including grid lines, rulers, and text sprites that use `disableLighting = true`. The light calculations still run on the CPU side for culling.

**Fix:** Use `light.excludedMeshes` to exclude meshes that don't need lighting, or use `light.includedOnlyMeshes` to limit lights to relevant meshes.

---

#### P-029 · Shadow generator casts shadows from ALL shadow casters without range limit (L456-458)
```javascript
const sg = new BABYLON.ShadowGenerator(2048, dir);
sg.usePoissonSampling = true;
```
**Problem:** 2048x2048 shadow map with Poisson sampling is expensive. The shadow generator has no `shadowMinZ`/`shadowMaxZ` bounds set on the directional light, so it must calculate shadow frustum for the entire scene including far-away grid lines.

**Fix:** Set `dir.shadowMinZ` and `dir.shadowMaxZ` to limit the shadow calculation range. Consider `sg.useBlurExponentialShadowMap = true` instead of Poisson for better performance.

---

### CATEGORY 6 — Garbage Collection Thrashing (Hot-Path `new` Allocations)

#### P-030 · `buildCraterMesh` allocates 3 `new BABYLON.Vector3` per call (L717-721)
```javascript
const path = [
  new BABYLON.Vector3(0, 0.1, 0),
  new BABYLON.Vector3(0, -sd, 0),
  new BABYLON.Vector3(0, -d, 0)
];
```
**Problem:** `buildCraterMesh` is called every frame during the `PENETRATING` state (L1097). Each call creates 3 new `Vector3` objects + 1 new `Array`. At 60fps, that's **180 Vector3 + 60 Arrays per second** of garbage for V8 to collect.

**Fix:** Pre-allocate a reusable `_craterPath` array with 3 pre-allocated `Vector3` instances. Mutate `.x`, `.y`, `.z` in-place using `.copyFromFloats()`.

---

#### P-031 · `boomRing.scaling = new BABYLON.Vector3(s,s,s)` in render loop (L1005)
```javascript
boomRing.scaling = new BABYLON.Vector3(s,s,s);
```
**Problem:** Creates a new `Vector3` every frame while `boomRing.isVisible`. This is pure GC thrash.

**Fix:** Use `boomRing.scaling.setAll(s)` or `boomRing.scaling.copyFromFloats(s,s,s)`.

---

#### P-032 · `shockwave.scaling = new BABYLON.Vector3(sw,sw,sw)` in render loop (L1163)
```javascript
shockwave.scaling = new BABYLON.Vector3(sw,sw,sw);
```
**Problem:** Same as P-031. New `Vector3` every frame during shockwave animation.

**Fix:** Use `shockwave.scaling.copyFromFloats(sw,sw,sw)`.

---

#### P-033 · `explosionSphere.scaling = new BABYLON.Vector3(ns,ns,ns)` in render loop (L1157)
```javascript
explosionSphere.scaling = new BABYLON.Vector3(ns,ns,ns);
```
**Problem:** Same as P-031. New `Vector3` every frame during explosion animation.

**Fix:** Use `explosionSphere.scaling.copyFromFloats(ns,ns,ns)`.

---

#### P-034 · `boomRing.scaling = new BABYLON.Vector3(1,1,1)` in state transitions (L1039)
```javascript
boomRing.scaling = new BABYLON.Vector3(1,1,1);
```
**Problem:** Allocates a new `Vector3` each time a sonic boom is triggered (potentially many times during a drop sequence).

**Fix:** Use `boomRing.scaling.copyFromFloats(1,1,1)`.

---

#### P-035 · `machCone.scaling = new BABYLON.Vector3(ma,1.0,ma)` in render loop (L1053)
```javascript
machCone.scaling = new BABYLON.Vector3(ma,1.0,ma);
```
**Problem:** New `Vector3` every frame while Mach cone is visible.

**Fix:** Use `machCone.scaling.copyFromFloats(ma,1.0,ma)`.

---

#### P-036 · `projectile.scaling = new BABYLON.Vector3(1,1,1)` in selectScenario (L967)
```javascript
projectile.scaling = new BABYLON.Vector3(1,1,1);
```
**Problem:** Unnecessary allocation during scenario selection.

**Fix:** Use `projectile.scaling.copyFromFloats(1,1,1)`.

---

#### P-037 · `projectile.position = new BABYLON.Vector3(...)` in selectScenario (L968)
```javascript
projectile.position = new BABYLON.Vector3(0, maxAlt>0?maxAlt:50, 0);
```
**Problem:** Creates a new Vector3 and reassigns the position reference, orphaning the old one.

**Fix:** Use `projectile.position.copyFromFloats(0, maxAlt>0?maxAlt:50, 0)`.

---

#### P-038 · `projectile.rotation = new BABYLON.Vector3(...)` in selectScenario (L969)
```javascript
projectile.rotation = new BABYLON.Vector3(0,0,Math.PI/2.0);
```
**Fix:** Use `projectile.rotation.copyFromFloats(0,0,Math.PI/2.0)`.

---

#### P-039 · `projectile.scaling = new BABYLON.Vector3(1.8,0.4,1.8)` in EXPLODING state (L1134)
```javascript
projectile.scaling = new BABYLON.Vector3(1.8,0.4,1.8);
```
**Fix:** Use `projectile.scaling.copyFromFloats(1.8,0.4,1.8)`.

---

#### P-040 · `explosionSphere.position = projectile.position.clone()` allocates (L1143)
```javascript
explosionSphere.position = projectile.position.clone();
```
**Problem:** `.clone()` creates a new `Vector3`. This runs once per explosion, minor but unnecessary.

**Fix:** Use `projectile.position.cloneToRef(explosionSphere.position)` or `explosionSphere.position.copyFrom(projectile.position)`.

---

#### P-041 · `shockwave.scaling = new BABYLON.Vector3(1,1,1)` in selectScenario (L974)
```javascript
shockwave.scaling = new BABYLON.Vector3(1,1,1);
```
**Fix:** Use `shockwave.scaling.copyFromFloats(1,1,1)`.

---

#### P-042 · `explosionSphere.scaling = new BABYLON.Vector3(0.1,0.1,0.1)` in selectScenario (L975)
```javascript
explosionSphere.scaling = new BABYLON.Vector3(0.1,0.1,0.1);
```
**Fix:** Use `explosionSphere.scaling.copyFromFloats(0.1,0.1,0.1)`.

---

#### P-043 · `boomRing.scaling = new BABYLON.Vector3(1,1,1)` in selectScenario (L976)
```javascript
boomRing.scaling = new BABYLON.Vector3(1,1,1);
```
**Fix:** Use `boomRing.scaling.copyFromFloats(1,1,1)`.

---

#### P-044 · `projectile.material.albedoColor = new BABYLON.Color3(1,1,1)` in render loop (L1092)
```javascript
projectile.material.albedoColor = new BABYLON.Color3(1,1,1);
```
**Problem:** Creates a new `Color3` on ~20% of frames during the `PENETRATING` state (gated by `Math.random()>0.8`). At 60fps that's ~12 new Color3 objects per second.

**Fix:** Pre-allocate a `const WHITE = new BABYLON.Color3(1,1,1)` constant and reuse it.

---

#### P-045 · `c3()` helper creates a new `BABYLON.Color3` on every call (L373-379)
```javascript
function c3(hex) {
  return new BABYLON.Color3(...);
}
```
**Problem:** `c3()` is called extensively throughout the codebase, including inside `computePlanckBlackbodyColor` which runs every frame during penetration (L1084). Each `computePlanckBlackbodyColor` call invokes `c3()` 2-3 times + `BABYLON.Color3.Lerp` which creates another `Color3`. Total: **~5 new Color3 per frame** during penetration.

**Fix:** Cache commonly used colors as constants (e.g., `const C3_CYAN = c3(0x00ffff)`). Use `Color3.LerpToRef()` with a pre-allocated output color.

---

#### P-046 · `getInterpolatedFrame` creates a new object with 30+ properties per call (L866-877)
```javascript
return {
  t, y:L(f0.y,f1.y), v:L(f0.v,f1.v), m:L(f0.m,f1.m), ...
};
```
**Problem:** Called every frame during DROPPING and PENETRATING states. Creates a new object with **30+ interpolated properties** per frame. At 60fps = 60 garbage objects/second, each with 30+ property slots.

**Fix:** Pre-allocate a single `_interpFrame` object and mutate its properties in-place.

---

#### P-047 · `L()` lambda inside `getInterpolatedFrame` creates a closure per call (L865)
```javascript
const L = (a,b) => (a!==undefined && b!==undefined) ? a+(b-a)*fac : undefined;
```
**Problem:** This arrow function is re-created on every call to `getInterpolatedFrame`. V8 can optimize this, but the function still captures `fac` as a closure variable, preventing some JIT optimizations.

**Fix:** Make `L` a module-level function that takes `fac` as a parameter, or inline the interpolation.

---

### CATEGORY 7 — Scenario Selection Main-Thread Blocking

#### P-048 · `selectScenario` performs synchronous heavy work (L881-980)
**Problem:** `selectScenario` does ALL of the following synchronously on the main thread in a single call:
1. Stops all camera animations (L883)
2. Stops anthem audio (L885)
3. Disposes all target layer meshes (L893) — iterates and disposes N meshes
4. Disposes all air meshes (L894) — iterates and disposes N meshes
5. Disposes ambient dust mesh (L905)
6. Creates 2500 Float32Array positions (L909-916)
7. Builds new ambient dust mesh with VertexData (L917-924)
8. Creates N target layer box meshes with PBR materials (L928-940)
9. Rebuilds ALL rulers: 150+ cylinders + 30+ text sprites (L942)
10. Disposes and rebuilds crater mesh (L946-950)
11. Disposes and rebuilds projectile mesh (L953-958) — lathe + merge
12. Compiles new PBR materials (triggers shader compilation)
13. Resets all effect meshes (L973-976)

This entire block can take **200-500ms**, freezing the browser and triggering "Page Not Responding".

**Fix:** Break into async chunks using `setTimeout(fn, 0)` or `requestIdleCallback`. At minimum, defer ruler/grid rebuilds. Better: reuse meshes and just update their properties instead of dispose+recreate.

---

#### P-049 · `clearMeshArray` disposes meshes AND materials synchronously (L735-738)
```javascript
function clearMeshArray(arr) {
  arr.forEach(m => { if(m && !m.isDisposed()) { if(m.material) m.material.dispose(); m.dispose(); } });
  arr.length = 0;
}
```
**Problem:** Material disposal triggers GPU resource deallocation which can stall the WebGL context. Disposing many meshes in a tight loop blocks the main thread.

**Fix:** Batch disposals across multiple frames, or reuse meshes by hiding them (`isVisible = false`) instead of disposing.

---

#### P-050 · Projectile mesh disposed and recreated on every scenario switch (L953-958)
```javascript
if(projectile){
  const om = projectile.material; projectile.material = null; projectile.dispose();
  projectile = createGBU57Mesh(currentData.proj_diameter/2.0, currentData.proj_length);
  projectile.material = om;
}
```
**Problem:** `createGBU57Mesh` performs a lathe operation (L626), creates 8 boxes (L629-646), and merges them all (L648). This is a heavy geometry operation that blocks the main thread for 50-100ms.

**Fix:** If projectile dimensions don't change between scenarios, reuse the existing mesh. If they do change, use `scaling` to approximate size differences instead of rebuilding geometry.

---

#### P-051 · `rebuildRulers` destroys and recreates ALL ruler meshes on every scenario switch (L506-557)
**Problem:** Rulers are destroyed and rebuilt from scratch even if the altitude hasn't changed. Creates 150+ cylinder meshes + text sprites synchronously.

**Fix:** Only rebuild if `maxAlt` actually changed from the previous scenario. Cache the last `maxAlt` value and skip if identical.

---

### CATEGORY 8 — Render Loop Inefficiencies

#### P-052 · Ambient dust loop iterates 2500 vertices every frame (L988-1000)
```javascript
for(let i=1; i<ambientDustPositions.length; i+=3){
  ...
  ambientDustPositions[i] -= dt*(3.0+6.0*(1.0-rho));
  ...
}
ambientDustMesh.updateVerticesData(BABYLON.VertexBuffer.PositionKind, ambientDustPositions, false, false);
```
**Problem:** Iterates **2500 positions** (7500 float values) every frame AND uploads the entire Float32Array to the GPU via `updateVerticesData`. At 60fps = 150,000 float operations + 150,000 GPU uploads per second.

**Fix:** Reduce particle count (500 is visually sufficient). Use a GPU particle system or a vertex shader with time-based offset instead of CPU-side position updates. At minimum, only upload if positions actually changed visibly.

---

#### P-053 · `dustSPS.setParticles()` called every frame during penetration (L1113)
```javascript
dustSPS.setParticles();
```
**Problem:** `setParticles()` recalculates ALL 1000 particle transforms and uploads them to the GPU. Only 20 particles are updated per frame (L1104), but the entire SPS buffer is re-uploaded.

**Fix:** Use `dustSPS.setParticles(startIdx, endIdx)` to only update the modified range, or track dirty particles and batch updates.

---

#### P-054 · `Math.random()` called in tight render loop for jitter (L1089-1090, L1091, L1105)
```javascript
projectile.position.x = (Math.random()-0.5)*ji;
projectile.position.z = (Math.random()-0.5)*ji;
```
**Problem:** `Math.random()` is called 4+ times per frame inside the penetration loop (L1089, L1090, L1091, L1105 x 20 iterations = 40 calls). While individual calls are fast, combined with other allocations this adds to GC pressure because V8's random number generator may allocate intermediate values.

**Fix:** Use a pre-seeded PRNG or a lookup table for jitter values.

---

#### P-055 · `updateHUD` pushes to arrays every frame even outside throttle (L821-823)
```javascript
chartDataTime.push(frame.t!==undefined?frame.t.toFixed(2):"");
chartDataVel.push(frame.v);
if(chartDataTime.length>100){chartDataTime.shift();chartDataVel.shift();}
```
**Problem:** `push` and `shift` on arrays run EVERY FRAME regardless of the throttle check. `Array.shift()` is O(n) — it copies all elements down by one index. At 60fps with 100-element arrays, that's **6000 element shifts per second**. Also, `frame.t.toFixed(2)` creates a new string every frame.

**Fix:** Use a circular buffer (ring buffer) with a fixed-size array and an index pointer. Replace `push/shift` with index wraparound.

---

#### P-056 · ECharts `updateChart` runs at 10Hz with full `setOption` call (L795-798, L825-828)
```javascript
function updateChart() {
  if(!telemetryChart) return;
  telemetryChart.setOption({xAxis:{data:chartDataTime}, series:[{data:chartDataVel}]});
}
```
**Problem:** `setOption` performs a deep merge of the new options with existing ones, re-validates the entire chart configuration, and triggers a full chart redraw. At 10Hz this is 10 full chart redraws per second, each potentially taking 5-10ms.

**Fix:** Use ECharts' `appendData` API for incremental updates, or reduce update frequency to 2-3Hz since the chart is small and telemetry doesn't need 10Hz visual updates.

---

#### P-057 · `animationLoop` has no early return for `IDLE` or `FINISHED` states (L983-1182)
**Problem:** When `animState === "IDLE"` or `animState === "FINISHED"`, the function still executes the ambient dust loop (L988-1000), boom ring check (L1003-1008), and shockwave check (L1011). The dust loop alone processes 2500 vertices for no visual benefit when the simulation is idle.

**Fix:** Add an early return after ambient dust for `IDLE`/`FINISHED` states, or gate the dust update behind an `isAnimating` flag.

---

### CATEGORY 9 — Memory Leaks & Resource Management

#### P-058 · `activeCrater.material` set to `null` before dispose, material NEVER disposed (L946)
```javascript
if(activeCrater && !activeCrater.isDisposed()){activeCrater.material=null; activeCrater.dispose();}
```
**Problem:** Setting `material = null` before `dispose()` orphans the PBR material. The material and its compiled shaders remain in GPU memory but are no longer referenced. Each scenario switch leaks one PBR material.

**Fix:** Dispose the material first: `activeCrater.material.dispose(); activeCrater.dispose();`

---

#### P-059 · Old projectile material reused but old mesh's geometry leaks (L953-958)
```javascript
const om = projectile.material; projectile.material = null; projectile.dispose();
projectile = createGBU57Mesh(...);
projectile.material = om;
```
**Problem:** `projectile.dispose()` disposes the mesh but may not dispose the merged geometry buffers created by `BABYLON.Mesh.MergeMeshes` (L648). The `MergeMeshes` with `disposeSource = true` disposes source meshes but the merged result's geometry is only released if `dispose()` is called with proper flags.

**Fix:** Call `projectile.dispose(false, true)` to also dispose geometry. Or better: reuse the mesh.

---

#### P-060 · `ambientDustMesh` disposed but its material is NOT disposed (L905)
```javascript
if(ambientDustMesh && !ambientDustMesh.isDisposed()) ambientDustMesh.dispose();
```
**Problem:** The `StandardMaterial` "adm" created at L921 is not disposed, leaking a material per scenario switch.

**Fix:** Dispose material before mesh: `ambientDustMesh.material.dispose(); ambientDustMesh.dispose();`

---

#### P-061 · Target layer PBR materials leaked on `clearMeshArray` — partial fix (L736)
```javascript
arr.forEach(m => { if(m && !m.isDisposed()) { if(m.material) m.material.dispose(); m.dispose(); } });
```
**Problem:** This correctly disposes materials, but the PBR material textures (if any internal textures were generated by Babylon) are not explicitly disposed. `material.dispose(true, true)` should be used to also dispose sub-textures.

**Fix:** Use `m.material.dispose(true, true)` for full texture cleanup.

---

#### P-062 · Audio nodes accumulated in `anthemNodes` array without cleanup (L560-596)
```javascript
let anthemNodes = [];
```
**Problem:** `stopAnthem` iterates `anthemNodes` to stop and disconnect, but if `playAnthem` is called rapidly (fast scenario switching), the array grows with orphaned oscillator/gain nodes that may still hold references to the closed AudioContext.

**Fix:** Clear the array after stopping: already done at L594, but `audioCtx.close()` should be awaited or error-handled more robustly. Consider reusing a single `AudioContext` instead of creating/closing one per anthem play.

---

#### P-063 · `window._shadowGen` stored as global, never cleaned up (L458)
```javascript
window._shadowGen = sg;
```
**Problem:** The shadow generator is stored on `window` and never updated or disposed during scenario switching. While the shadow generator itself persists, shadow casters accumulate because `addShadowCaster(mesh)` is called for each new projectile (L957) without removing the old one.

**Fix:** Call `window._shadowGen.removeShadowCaster(projectile)` before disposing the old projectile mesh.

---

#### P-064 · ECharts instance never resized on window resize (L426, L781)
```javascript
window.addEventListener("resize", () => engine.resize());
```
**Problem:** The Babylon.js engine is resized on window resize, but the ECharts instance (`telemetryChart`) is not. The chart element is inside the React DOM which reflows, but ECharts needs an explicit `.resize()` call.

**Fix:** Add `if(telemetryChart) telemetryChart.resize();` to the resize handler.

---

### CATEGORY 10 — Draw Call & Scene Overhead

#### P-065 · `scene.freezeActiveMeshes()` never called (entire file)
**Problem:** Babylon.js re-evaluates which meshes are in the camera frustum every frame. With 200+ grid lines + 150+ ruler meshes + effect meshes, this is 350+ frustum checks per frame. `freezeActiveMeshes()` caches the active mesh list and skips frustum evaluation.

**Fix:** Call `scene.freezeActiveMeshes()` after initial setup and after each `selectScenario` finishes building meshes. Unfreeze before rebuilding.

---

#### P-066 · No mesh instancing used — each grid tick is a unique mesh (L517-533, L542-556)
**Problem:** 150 underground depth ruler ticks and 50+ altitude ticks are individual `CreateCylinder` meshes. These have identical geometry but different positions and materials. Using instances or thin instances would reduce draw calls from 200+ to 2 (one per unique geometry).

**Fix:** Use `BABYLON.Mesh.createInstance()` or thin instances for ruler tick marks.

---

#### P-067 · `pipeline.bloomKernel = 64` is very high (L417)
```javascript
pipeline.bloomKernel = 64;
```
**Problem:** A bloom kernel of 64 means a 64x64 Gaussian blur pass on the bloom texture. This is extremely expensive. Typical values are 16-32.

**Fix:** Reduce to `32` or `24`. The visual difference is negligible but the GPU cost is halved.

---

#### P-068 · All meshes are pickable by default (entire file)
**Problem:** Babylon.js performs picking ray intersection tests on ALL meshes unless `isPickable = false` is set. Grid lines, ruler ticks, dust particles, effect meshes — none of these need to be pickable. With 400+ meshes, picking overhead is significant.

**Fix:** Set `mesh.isPickable = false` on all non-interactive meshes (everything except the canvas pointer interactions).

---

#### P-069 · `scene.fogMode = BABYLON.Scene.FOGMODE_LINEAR` applied to ALL materials (L390)
**Problem:** Linear fog calculation is applied per-vertex for every mesh in the scene, including meshes that will never be at fog distances (close-up HUD elements, crater, projectile). This adds unnecessary GPU shader instructions.

**Fix:** Exclude close-range meshes from fog by setting `material.fogEnabled = false` on meshes that are always within fog start distance.

---

### CATEGORY 11 — Miscellaneous Performance Issues

#### P-070 · `listAllData()` runs on page load, logging ALL scenario data to console (L328-331)
```javascript
function listAllData() {
  scenarios.forEach((d, i) => console.log("Scenario " + i + ":", d));
}
listAllData();
```
**Problem:** `console.log` with large objects triggers V8 to serialize the entire object tree for DevTools. If DevTools is open, this can take 50-100ms per scenario. Even if DevTools is closed, V8 still serializes the objects into an internal buffer.

**Fix:** Remove or gate behind a `DEBUG` flag: `if (window.DEBUG) listAllData();`

---

#### P-071 · `initECharts` uses `setTimeout(initECharts, 50)` retry polling (L778)
```javascript
if(!chartEl) {
  setTimeout(initECharts, 50);
  return;
}
```
**Problem:** If the chart element doesn't exist yet (React hasn't rendered), this retries every 50ms. Minor, but uses polling instead of event-driven initialization.

**Fix:** Use `MutationObserver` or initialize ECharts from inside the React `useEffect` after the chart element is mounted.

---

#### P-072 · `camera.inertia = 0.9` is very high, causes sluggish response (L408)
```javascript
camera.inertia = 0.9;
```
**Problem:** While not a performance issue per se, high inertia means the camera continues computing interpolated positions for many frames after user input stops, keeping the scene "dirty" and preventing frame-skip optimizations.

**Fix:** Reduce to `0.85` or `0.8` for snappier response and fewer trailing update frames.

---

#### P-073 · `camera.target.clone()` in `smoothCameraMove` creates unnecessary allocation (L770)
```javascript
push(BABYLON.Animation.CreateAndStartAnimation("ct",camera,"target",F,D,camera.target.clone(),new BABYLON.Vector3(tx,ty,tz),0,ease));
```
**Problem:** Two allocations: `.clone()` + `new BABYLON.Vector3`. These run on every camera move button click.

**Fix:** Minor impact since it's user-triggered, not per-frame. But could use pre-allocated vectors.

---

#### P-074 · `createGBU57Mesh` creates 12 `new BABYLON.Vector3` for nose curve (L612-615)
```javascript
const pts = [new BABYLON.Vector3(0,yN,0)];
for(let i=1; i<=12; i++){
  const t=i/12;
  pts.push(new BABYLON.Vector3(pRadius*Math.sin(t*Math.PI*0.5), yN+t*nL, 0));
}
```
**Problem:** 21+ `new BABYLON.Vector3` allocations for the lathe profile. Runs on every scenario switch.

**Fix:** Pre-allocate the profile array once and reuse with `copyFromFloats`.

---

---

## Troubleshooting & Verification Matrix

1. **Chromium DevTools Profiling:** Press `F12` -> **Performance** tab -> Record a trace during scenario selection.
   - **Yellow (Scripting):** Main-thread JavaScript execution / heavy loops.
   - **Purple (Rendering):** DOM layout thrashing / CSS style recalculations.
   - **Long Frame Times with Low CPU/GPU Usage:** iGPU fallback or sync stall.
2. **GPU Acceleration Check:** Navigate to `chrome://gpu` (or `edge://gpu`) and verify that **WebGL2** and **WebGPU** report _Hardware Accelerated_.
3. **Windows Graphics Settings:** Navigate to **Windows Settings -> System -> Display -> Graphics**, add your browser executable, and set preference to **High Performance (NVIDIA GeForce RTX 4080)**.
4. **ANGLE Backend:** Open `brave://flags/#use-angle` or `edge://flags/#use-angle` and test **D3D11** or **Vulkan**.

---

## Summary Statistics

| Category | Count | Severity |
|:---|:---:|:---:|
| Canvas & CSS Isolation | 7 | HIGH |
| Script Loading & Parse Blocking | 5 | HIGH |
| React HUD Layout Thrashing | 6 | HIGH |
| Babylon.js Engine Configuration | 4 | MEDIUM |
| Static Mesh Overhead (Missing freeze) | 7 | HIGH |
| GC Thrashing (Hot-path `new`) | 18 | CRITICAL |
| Scenario Selection Blocking | 4 | CRITICAL |
| Render Loop Inefficiencies | 6 | MEDIUM |
| Memory Leaks & Resource Management | 7 | MEDIUM |
| Draw Call & Scene Overhead | 5 | MEDIUM |
| Miscellaneous | 5 | LOW |
| **TOTAL** | **74** | |

> **Top 3 Impact Fixes (in order):**
> 1. **P-048**: Break `selectScenario` into async chunks — eliminates "Page Not Responding"
> 2. **P-030 to P-046**: Pre-allocate all hot-path Vector3/Color3 — eliminates GC spikes
> 3. **P-008 to P-010**: Switch to production React + pre-built Tailwind + remove Babel — reduces parse time by ~70%
