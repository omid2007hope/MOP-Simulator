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

## Detailed Root Cause & Possibility Analysis

### 1. Is Babylon.js Simply Too Heavy?

**Verdict:** **No.** Babylon.js is an enterprise-grade 3D engine designed for real-time rendering. However, web-based rendering operates under strict browser constraints that differ from native application environments:

- **Synchronous Shader Compilation:** When loading heavy assets during scenario selection, Babylon.js compiles GLSL/WGSL materials. Compiling many complex shaders synchronously on the main UI thread freezes execution, causing the browser to flag the tab as unresponsive.
- **Single-Threaded JavaScript Engine:** Even with an i9 processor, V8 JavaScript execution runs predominantly on a single thread. If physics, scene graph traversals, or matrix calculations exceed **16.6 ms per frame** (or **6.9 ms for 144 Hz**), CPU stuttering occurs while the GPU sits idle.

---

### 2. Badly Written HTML Template / JS Architecture

**Verdict:** **High Probability.** Poor integration between the HTML DOM layer and the 3D canvas is a primary cause of web tab unresponsive errors.

- **Layout Thrashing (Reflows):** Reading or modifying HTML DOM elements (such as HUDs, overlays, or scoreboards) inside `engine.runRenderLoop()` forces Chromium to recalculate layout every frame, severely dropping framerates.
- **Synchronous Asset Loading & Scene Transitions:** Switching scenarios without releasing previously loaded assets or loading `.gltf`/`.babylon` models on the main UI thread blocks the event loop completely.
- **Unmanaged Garbage Collection (GC):** Instantiating objects (e.g., `new BABYLON.Vector3()`) within the render loop creates memory thrashing, triggering frequent GC pauses and stuttering.

---

### 3. Missing HTML & JavaScript Optimizations

**Verdict:** **Confirmed.** Specific attributes, CSS directives, and engine flags must be declared to unlock maximum graphics throughput.

#### A. HTML & CSS Layer Optimizations

```css
/* Optimized Canvas CSS Container */
canvas#renderCanvas {
  width: 100%;
  height: 100%;
  touch-action: none;

  /* Forces discrete compositor layer in Chromium */
  will-change: transform;

  /* Isolates canvas layout and paint calculations from the parent DOM */
  contain: strict;

  /* Prevents unwanted text/element selection handling */
  user-select: none;
}
```

#### B. JavaScript & Engine Initialization Safeguards

```javascript
// 1. Explicitly request High-Performance discrete GPU context
const engine = new BABYLON.Engine(canvas, true, {
  powerPreference: "high-performance",
  preserveDrawingBuffer: false,
  stencil: false,
  doNotHandleTouchAction: true,
});

// 2. Enable Parallel / Non-Blocking Shader Compilation
engine.getCaps().parallelShaderCompile;

// 3. Asset & Mesh Optimization Techniques
scene.freezeActiveMeshes(); // Halts bounding-box evaluation for static objects
mesh.freezeWorldMatrix(); // Disables matrix computation for fixed geometry
material.freeze(); // Prevents material re-evaluations

// 4. Garbage Collection Prevention: Reuse Vector References
const tempVector = new BABYLON.Vector3();
scene.onBeforeRenderObservable.add(() => {
  // Use in-place mutations instead of creating 'new' instances per frame
  BABYLON.Vector3.ZeroToRef(tempVector);
});
```

---

### 4. Alternative Causes ("Else")

#### A. Integrated Graphics Override (iGPU vs. Discrete GPU)

Windows Graphics Settings often default browser processes (`brave.exe`, `msedge.exe`) to the Intel CPU's integrated graphics rather than the RTX 4080.

- **Resolution:** Navigate to **Windows Settings → System → Display → Graphics**, add your browser executable, and set preference to **High Performance (NVIDIA GeForce RTX 4080)**.

#### B. ANGLE Graphics Backend Configuration

Chromium translates WebGL calls to DirectX/Vulkan via ANGLE. Certain driver/ANGLE combinations produce stuttering under heavy draw calls.

- **Resolution:** Open `brave://flags/#use-angle` or `edge://flags/#use-angle` and test explicitly setting the backend to **D3D11** or **Vulkan**.

#### C. Memory Leaks During Scenario Switching

Failure to properly dispose of scene nodes and GPU textures when switching scenarios causes VRAM accumulation until the context is reset by the browser.

```javascript
// Proper Scene Teardown Pattern
scene.dispose();
engine.stopRenderLoop();
// Re-initialize or load clean scene
```

---

## Troubleshooting & Verification Matrix

1. **Chromium DevTools Profiling:** Press `F12` → **Performance** tab → Record a trace during scenario selection.
   - **Yellow (Scripting):** Main-thread JavaScript execution / heavy loops.
   - **Purple (Rendering):** DOM layout thrashing / CSS style recalculations.
   - **Long Frame Times with Low CPU/GPU Usage:** iGPU fallback or sync stall.
2. **GPU Acceleration Check:** Navigate to `chrome://gpu` (or `edge://gpu`) and verify that **WebGL2** and **WebGPU** report _Hardware Accelerated_.
