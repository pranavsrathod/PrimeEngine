# PrimeEngine UI System Overhaul
## From Custom UIButton to Dear ImGui

---

## Overview

This document covers the replacement of PrimeEngine's original custom 2D UI system with **Dear ImGui**, an industry-standard immediate-mode GUI library. The goal was to implement a working Pause/Resume/Exit HUD as a foundation for future UI work, then extend it with live in-game controls for the particle system and 3D audio.

---

## The Old System — What Existed and Why It Was Broken

### Files
- `Code/PrimeEngine/Scene/UIButton.cpp / .h`
- `Code/PrimeEngine/Scene/UIManager.cpp / .h`
- `Code/PrimeEngine/Scene/ButtonSceneNode.cpp / .h`
- `Code/PrimeEngine/Scene/UIElement.h`
- `Code/PrimeEngine/Scene/UI_Events.h / .cpp`

### Problem 1 — Buttons Were Created Every Frame

`UIButton::Construct()` was called from inside the per-frame render block in `GameThreadJob.cpp`. This meant a brand new button object was allocated and added to the scene graph **every single frame** instead of once at startup. The scene graph would grow unboundedly.

```cpp
// Inside the per-frame block — WRONG
UIButton::Construct(*m_pContext, m_arena,
    Vector2(0.05f, 0.5f), 2.0f, 1.5f, "Pause");
```

### Problem 2 — Hit Testing Was Hardcoded and Global

The click handler in `UIButton::do_TEST_ONCLICK` did not check the button's own position or size. Instead it used a hardcoded screen Y threshold:

```cpp
// Every button on screen used this same check — completely wrong
if (normalizedY < 0.55) {
    // fire pause
} else {
    // fire resume
}
```

This means no matter how many buttons existed, they all shared the same hit region. There was no per-button bounds check.

### Problem 3 — Two Conflicting Construction Paths

`UIButton` had two separate constructors:
1. One taking `Event_CREATE_BUTTON*` — used by `UIManager` (the Lua/event path)
2. One taking raw `pos/width/height` — used directly from `GameThreadJob`

They set up state differently and neither was fully wired end-to-end.

### Problem 4 — Visuals Were Built on DebugRenderer

The button rectangle and label were drawn using `DebugRenderer::createRectangleMesh()` and `DebugRenderer::createTextMesh()`. These are temporary debug overlays with TTL (time-to-live) semantics — they are not meant for persistent interactive UI and have no concept of click interaction.

### Problem 5 — All Pause Logic Was Commented Out

Every meaningful line of code — button creation, event dispatch, `isPaused = true/false`, and `m_frameTime = 0` — was commented out across `UIButton.cpp`, `GameThreadJob.cpp`, and `StandardGameEvents.h`. The system was structurally incomplete.

### Problem 6 — Misspelled Event Name

`Event_RESUME_GAME` was misspelled as `Event_RESUEME_GAME` consistently across three files (`StandardGameEvents.h`, `StandardGameEvents.cpp`, `UIButton.cpp`).

---

## The New System — Dear ImGui

### What is Dear ImGui?

[Dear ImGui](https://github.com/ocornut/imgui) is a bloat-free, immediate-mode GUI library for C++. It is widely used in game engines, editors, and debug tools. Key properties:

- **Immediate mode**: no persistent button objects, no lifetime management, no hit-testing code to write
- **Self-contained**: ships as plain `.h / .cpp` source files — no separate build, no DLL
- **Official backends** for DirectX 9, DirectX 11, and Win32 — matching PrimeEngine's exact targets
- A button is literally one line: `if (ImGui::Button("Pause")) { ... }`

### Files Added

**Core ImGui** (`External/DownloadedLibraries/imgui/`):
- `imgui.cpp`
- `imgui_draw.cpp`
- `imgui_tables.cpp`
- `imgui_widgets.cpp`
- `imconfig.h`, `imgui.h`, `imgui_internal.h`, `imstb_*.h`

**Backends** (`External/DownloadedLibraries/imgui/backends/`):
- `imgui_impl_dx9.cpp / .h`
- `imgui_impl_win32.cpp / .h`

All `.cpp` files were registered as `ClCompile` items in `PrimeEngine-win32d3d9.vcxproj`. Include paths added to both Debug and Release configurations:

```xml
..\External\DownloadedLibraries\imgui
..\External\DownloadedLibraries\imgui\backends
```

---

## Integration — The 4 Hook Points

### 1. Initialization — `D3D9Renderer.cpp`

Called once at the end of the `D3D9Renderer` constructor, after the device and window handle are fully ready:

```cpp
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

// At end of D3D9Renderer constructor:
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGui_ImplWin32_Init(pWinApp->m_windowHandle);
ImGui_ImplDX9_Init(m_pD3D9Device);
```

### 2. Win32 Message Forwarding — `WinApplication.cpp`

ImGui needs to receive Win32 mouse/keyboard messages to know when buttons are clicked. Added at the top of `windowsEventHandler`:

```cpp
#include "imgui.h"
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WinApplication::windowsEventHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    // ... rest of handler
}
```

### 3. Per-Frame UI — `GameThreadJob.cpp`

Called each frame inside the render block. Starts a new ImGui frame, declares the UI, then finalizes the draw data:

```cpp
ImGui_ImplDX9_NewFrame();
ImGui_ImplWin32_NewFrame();
ImGui::NewFrame();

ImGui::Begin("Game Controls");
// ... UI widgets ...
ImGui::End();

ImGui::Render();
```

### 4. Render Draw Data — `RenderJob.cpp`

Called in `runDrawThreadSingleFrame()` between `endFrame()` and `swap()`, so ImGui renders on top of the 3D scene before the frame is presented:

```cpp
// Between endFrame() and swap():
ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
```

---

## Pause/Resume/Exit Logic

### The Core Problem with the Old Approach

Setting `m_frameTime = 0` alone was not enough to pause the scene. `m_frameTime` is calculated at the **end** of a frame, but the update events (`Event_UPDATE`, `Event_SCENE_GRAPH_UPDATE`, `Event_PHYSICS_START`) are pushed at the **beginning** of the next frame using the already-stored value. Even with correct timing, systems like OpenAL (sound) run entirely independently and ignore `m_frameTime`.

### The Fix — Block Update Events When Paused

When `isPaused` is true, the three update events are simply not pushed onto the event queue. The scene graph, physics, and all game objects receive no update signal and stay frozen. Draw events are still pushed so the screen continues to render the frozen frame.

```cpp
// In GameThreadJob.cpp, at the top of runGameFrame():
if (!isPaused)
{
    // Push Event_PHYSICS_START
    // Push Event_UPDATE
    // Push Event_SCENE_GRAPH_UPDATE
}
```

### Sound Pause/Resume

OpenAL audio plays on its own thread and is completely unaffected by blocking update events. Two methods were added to `SoundComponent`:

```cpp
void SoundComponent::pause()  { alSourcePause(m_source); }
void SoundComponent::resume() { alSourcePlay(m_source);  }
```

A `Handle m_hSound` member was added to `ClientGame` to hold a reference to the active sound component, allowing the ImGui buttons to call `pause()` / `resume()` directly.

### The Particle System Fix

The original code used `isPaused` as a "spawn particles once" flag, setting it to `true` immediately after creating the particle system. This meant every Resume click would spawn new particles and instantly re-set `isPaused = true`. Fixed by introducing a dedicated flag:

```cpp
static bool particlesSpawned = false;
if (!particlesSpawned) {
    // ... create particle system ...
    particlesSpawned = true;
}
```

---

## Extended UI — Particle System Controls

A collapsible **Particle System** panel was added to the ImGui window. It exposes live control over the `ParticleSystemCPU::m_particleTemplate` struct, which is read every frame by `updateParticleBuffer()` — so all changes take effect on newly spawned particles immediately.

A `Handle m_hParticleMesh` was added to `ClientGame` to provide access from the UI block.

| Control | Field modified | Effect |
|---|---|---|
| Type (combo) | `m_systemType` | Switches between Sphere / Burst / Fountain / Spiral / Fire |
| Speed | `m_particleTemplate.m_speed` | Changes velocity magnitude on spawn |
| Duration | `m_particleTemplate.m_duration` | Changes particle lifetime |
| Size | `m_particleTemplate.m_size` | Uniform particle scale |
| Rate | `m_particleTemplate.m_rate` | Particles spawned per second |

---

## Extended UI — Sound Controls

A collapsible **Sound** panel was added below the particle panel. It exposes live OpenAL source properties via new methods on `SoundComponent`.

### New SoundComponent API

```cpp
void setGain(float gain);          // AL_GAIN
void setMute(bool mute);           // sets gain to 0, restores on unmute
void setMinDistance(float dist);   // AL_REFERENCE_DISTANCE
void setMaxDistance(float dist);   // AL_MAX_DISTANCE
void setRolloff(float rolloff);    // AL_ROLLOFF_FACTOR
```

Member variables (`m_gain`, `m_muted`, `m_minDistance`, `m_maxDistance`, `m_rolloff`) are stored on `SoundComponent` as public floats/bools so ImGui sliders can hold direct pointers to them.

| Control | What it does |
|---|---|
| Mute / Unmute | Silences audio via `AL_GAIN = 0`, restores saved gain on unmute |
| Volume | 0–1 gain slider, greyed out while muted |
| Min Distance | Radius within which sound plays at full volume (`AL_REFERENCE_DISTANCE`) |
| Max Distance | Distance beyond which no further attenuation occurs (`AL_MAX_DISTANCE`) |
| Rolloff | How fast volume falls off between min and max (`AL_ROLLOFF_FACTOR`) |

### Distance-Based Restart Bug Fix

`SoundComponent::do_UPDATE` previously restarted the sound any time it wasn't playing and the camera was in range — including after the sound naturally ended. This made it impossible to stop the sound intentionally. Fixed with a `m_stoppedByDistance` flag:

```cpp
if (distance > cutoffDistance && state == AL_PLAYING)
{
    alSourceStop(m_source);
    m_stoppedByDistance = true;
}
else if (distance <= cutoffDistance && m_stoppedByDistance && state != AL_PLAYING)
{
    alSourcePlay(m_source);
    m_stoppedByDistance = false;
}
```

The restart now only fires if the distance system was the one that stopped it.

---

## Full Feature Summary

| Feature | Before | After |
|---|---|---|
| Buttons | Spawned every frame, never worked | Immediate-mode ImGui, zero allocation overhead |
| Hit testing | Hardcoded Y threshold, broken | Built into ImGui |
| Pause | Commented out | Blocks update/physics/scene graph events |
| Resume | Commented out | Re-enables update events |
| Sound pause | Not implemented | `alSourcePause` / `alSourcePlay` |
| Exit | Commented out | `m_runGame = false`, clean shutdown |
| Particle type | Hardcoded | Live combo: Sphere / Burst / Fountain / Spiral / Fire |
| Particle speed | Hardcoded | Live slider |
| Particle duration | Hardcoded | Live slider |
| Particle size | Hardcoded | Live slider |
| Particle rate | Hardcoded | Live slider |
| Sound volume | Hardcoded 1.0 | Live slider with mute |
| 3D attenuation | Hardcoded | Live min/max distance + rolloff sliders |

---

## What's Next

- **More HUD elements** — health bars, ammo counts, score display (`ImGui::ProgressBar`, `ImGui::Text`)
- **Debug panels** — toggle rendering modes, tweak physics values, inspect scene graph live
- **Multiple sound sources** — extend the Sound panel to list and control multiple `SoundComponent` instances
- **Settings menu** — resolution, keybindings (`ImGui::Combo`, `ImGui::InputText`)
- **Level/game state UI** — main menu, game over screen using `ImGui::SetNextWindowPos`

---

## Resume Bullet Points

These are concrete, specific points from this project suitable for a software engineering resume.

- **Diagnosed and replaced a broken retained-mode UI system** in a custom C++ game engine; identified 6 root-cause defects including per-frame heap allocation of scene objects, global hit-test logic with no per-element bounds checking, and dead code throughout — replaced entirely with Dear ImGui
- **Integrated Dear ImGui into a custom DirectX 9 / Win32 game engine** from scratch, including device initialization, Win32 WndProc message forwarding, per-frame immediate-mode rendering, and correct placement in a multi-threaded game/draw thread pipeline
- **Debugged a game engine pause system** by tracing event queue ordering across frame boundaries; identified that `m_frameTime = 0` had no effect because update events were enqueued at the start of the frame using the prior frame's value — fixed by conditionally suppressing `Event_UPDATE`, `Event_PHYSICS_START`, and `Event_SCENE_GRAPH_UPDATE`
- **Extended a CPU particle system** with live runtime controls; exposed `ParticleSystemCPU::m_particleTemplate` fields (speed, rate, size, duration, system type) through an ImGui panel — changes apply immediately to newly spawned particles without restarting the system
- **Extended an OpenAL 3D audio component** with a full runtime control panel; added gain, mute (with saved-gain restore), and spatial attenuation parameters (`AL_REFERENCE_DISTANCE`, `AL_MAX_DISTANCE`, `AL_ROLLOFF_FACTOR`) exposed as live ImGui sliders
- **Fixed a state synchronization bug** in an OpenAL sound component where `do_UPDATE` unconditionally restarted stopped sources, interfering with intentional mute/stop controls — introduced a `m_stoppedByDistance` flag to distinguish distance-triggered stops from natural playback end
