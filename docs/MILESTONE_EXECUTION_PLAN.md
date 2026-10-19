# Ixo Engine Milestone Execution Plan

This file turns the roadmap into an implementation order. Milestones are delivered incrementally; later work depends on the acceptance gates below. Do not mark a milestone complete until its tests and evidence exist.

## Phase A — Build reliability
Deliverables:
- Reproducible Android debug APK build from a clean checkout.
- CI artifact upload and failure logs.
- CMake configure/build checks and basic native test target.
- CI concurrency cancels superseded runs for the same ref.

Acceptance gate:
- GitHub Actions build is green on the current main commit.
- APK artifact exists and can be downloaded.
- No unreviewed build errors remain.

## Phase B — Real Vulkan frame renderer
Deliverables:
- Android native-window lifecycle and Vulkan surface.
- Device/queue selection, logical device, swapchain, swapchain image views.
- Frame synchronization, command recording/submission, depth buffer, resize handling.
- Minimal triangle test followed by indexed geometry and a camera.
- Device capability checks and a clean non-Vulkan fallback state.

Acceptance gate:
- A real frame is presented by Vulkan on a physical Android Vulkan device.
- Validation/debug logs contain no known renderer lifecycle errors.
- Pause/resume, rotation, surface loss, and swapchain recreation are tested.

Renderer implementation rule:
- Rendering belongs to the native Vulkan layer and shader pipeline.
- Do not call Android setBackgroundColor() to render, clear, or fake a Vulkan viewport.
- Any Android UI styling must be isolated from renderer code; it is not a rendering fallback.
- Until swapchain presentation is implemented and tested, describe the current code as a Vulkan capability probe only.

## Phase C — Scene and resource core
Deliverables:
- Stable node IDs, parent/child graph, local/global transforms.
- Versioned scene serialization, validation, save/load, and migration.
- Camera, mesh, and light components plus resource references.
- Project-root path resolver and file/folder destinations.
- Undo/redo-ready command architecture.

Acceptance gate:
- Sample scene round-trips through save/load without losing node hierarchy or properties.
- Invalid scene files return useful errors rather than crashing.

## Phase D — LuaJIT and Ixo scripting
Deliverables:
- Pinned LuaJIT dependency and supported Android ABI build.
- .Ixo.scripting loader/compiler and lifecycle scheduler.
- Node/transform/input/time/logging bindings.
- Exported properties and custom signals with type checks.
- Runtime error panel, stack traces, safe reload, and script sandbox limits.

Acceptance gate:
- A script can change a real node transform in a running scene.
- Lifecycle callbacks, exported properties, and custom signals have automated tests.
- Invalid scripts fail with diagnostics without taking down the editor.

## Phase E — Editor workflows
Deliverables:
- Project creation/opening and file/folder browser.
- Scene hierarchy, node creation and reparenting, inspector.
- Real Vulkan viewport, camera navigation, selection and transform gizmos.
- Script tabs, syntax highlighting, diagnostics, autosave and safe file operations.
- Adaptive mobile layouts for portrait, landscape, and tablets.

Acceptance gate:
- A user can create a project, add a camera/mesh/light, attach a script, save, close, and reopen it on-device.

## Phase F — Assets, materials, and animation
Deliverables:
- glTF 2.0/GLB import, texture/material processing, import cache and reimport.
- Mobile PBR materials, lights/shadows, tone mapping and environment options.
- Skeletal animation and animation state controls.
- Optional toon/anime shader path and water effects behind quality settings.

Acceptance gate:
- A licensed sample GLB imports, renders, animates, and reloads reliably.
- Performance tiers respect device capability and memory budgets.

## Phase G — Box2D and game systems
Deliverables:
- Verified Box2D Android build and fixed-step world.
- Body/shape/sensor components, collision filters and contact callbacks.
- Physics-to-scene synchronization and script signal integration.
- Input action maps, mobile controls, audio, pause/resume and save data.

Acceptance gate:
- Collision and sensor tests pass in CI and on Android.
- Physics remains stable across variable render frame rates and lifecycle pauses.

## Phase H — Release readiness
Deliverables:
- Native/scene/script tests, APK build, install/launch smoke tests.
- Device tests on at least one physical Vulkan Android device.
- Dependency/license/security audit and signed release pipeline using CI secrets.
- Versioned APK/AAB, checksums, release notes and known-limitations report.

Acceptance gate:
- Release artifact is installable, starts correctly, and passes smoke tests.
- Signing credentials are never committed to the repository.

## Current status snapshot
The repository currently has an Android editor shell, script text save, starter scene/configuration, a Vulkan instance/device probe, and an APK workflow definition. It does not yet have a full Vulkan renderer, an embedded LuaJIT runtime, a functioning Box2D scene integration, or a production hierarchy/inspector. CI success and physical-device behavior must be verified before being marked complete.
