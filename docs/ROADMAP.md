# Ixo Engine Development Roadmap

This roadmap tracks the full mobile-first engine/editor scope. A checkbox means the work is implemented and verified—not merely planned. Current code is a bootstrap; Vulkan rendering, LuaJIT execution, Box2D integration, and full editor tooling remain incomplete.

## M0 — Repository and build foundation
- [x] Android Gradle/CMake project and debug APK workflow definition
- [x] Touch-friendly starter screen and basic script save
- [x] Vulkan instance/device capability probe
- [x] Starter scene, node naming, and project YAML paths
- [ ] Verify clean Android APK build in GitHub Actions and retain build logs/artifact
- [ ] Add automated native, scene-format, script-format, and workflow checks
- [ ] Add build matrix for Android arm64-v8a; add x86_64 only where supported/tested
- [ ] Add crash reporting/debug logging policy and reproducible build notes

## M1 — Native platform and Vulkan renderer
- [ ] Add Android Surface/ANativeWindow lifecycle and surface-loss recovery
- [ ] Create Vulkan instance with Android surface extensions and validation in debug builds
- [ ] Select physical device and graphics/present queue families; validate required features
- [ ] Create logical device, queues, swapchain, image views, and framebuffers
- [ ] Add command pools/buffers, fences/semaphores, and frames-in-flight synchronization
- [ ] Add depth format selection, depth images, depth testing, and resize/recreation
- [ ] Add render pass or dynamic-rendering path based on Android device support
- [ ] Render a real triangle, then indexed meshes and a camera-driven 3D scene
- [ ] Add MSAA capability detection and configurable anti-aliasing
- [ ] Add GPU resource lifetime tracking, device-lost handling, and validation-clean shutdown
- [ ] Profile frame time, CPU/GPU usage, memory, thermal behavior, and battery impact
- [ ] Add fallback messaging when Vulkan is unavailable; never silently claim a frame was rendered

### Vulkan renderer rule
The Vulkan renderer must use Vulkan resources, render passes/dynamic rendering, pipelines, descriptors, and shader outputs for rendering. **Do not use Android setBackgroundColor() as a rendering method or as a substitute for Vulkan frame rendering.** UI background styling may exist only in the editor UI layer and must remain separate from the native Vulkan renderer. The current native code is only a capability probe, not a renderer.

## M2 — Scene graph and project files
- [ ] Define versioned, validated .IxoScene schema and migration strategy
- [ ] Implement Node3D transforms, parent/child ownership, traversal, and stable node IDs
- [ ] Implement Camera3D, MeshInstance3D, DirectionalLight3D, OmniLight3D, SpotLight3D
- [ ] Implement resources, scene instancing, node paths, and safe reference resolution
- [ ] Implement create/open/save/reload and atomic scene writes
- [ ] Implement project root, res:// paths, file/folder browser, import/export destinations
- [ ] Add undo/redo commands and dirty-scene state
- [ ] Add scene validation and useful error messages for malformed files

## M3 — Ixo scripting and LuaJIT
- [ ] Integrate a pinned, license-reviewed LuaJIT source/build for supported Android ABIs
- [ ] Implement .Ixo.scripting load, compile, diagnostics, and runtime error reporting
- [ ] Define lifecycle callbacks (on_ready, on_update, on_physics_update, on_exit)
- [ ] Bind safe node lookup, transforms, camera access, input, timers, and logging
- [ ] Implement exported properties with type validation, defaults, and inspector metadata
- [ ] Implement custom signals: declaration, connect/disconnect, emit, and argument checks
- [ ] Implement resource loading and script-to-node attachment
- [ ] Add debugger console, stack traces, script reload, and syntax diagnostics
- [ ] Sandbox script execution with memory/instruction limits and controlled file access
- [ ] Test LuaJIT behavior on real arm64 Android devices; document unsupported architectures

## M4 — Mobile editor UX
- [ ] Build project open/create screen and file/folder destination browser
- [ ] Add scene tree with node creation, rename, duplicate, reparent, and delete
- [ ] Add inspector for transforms, exported properties, resources, and physics
- [ ] Add actual Vulkan viewport embedded in the editor surface
- [ ] Add touch orbit/pan/zoom, selection, transform gizmos, snapping, and focus
- [ ] Add split layouts/panels that adapt to portrait, landscape, and tablet screens
- [ ] Add script tabs, syntax highlighting, line numbers, search, diagnostics, and autosave
- [ ] Add resource picker, import progress, asset thumbnails, and missing-resource warnings
- [ ] Add keyboard/IME handling, accessibility labels, and large-touch-target mode

## M5 — 3D asset pipeline and rendering features
- [ ] Import glTF 2.0/GLB meshes, nodes, PBR materials, textures, skins, and animations
- [ ] Add mesh/resource cache, import settings, dependency tracking, and reimport
- [ ] Add physically based material model and shader variants for mobile
- [ ] Add directional/point/spot lighting and configurable shadows
- [ ] Add environment lighting, sky/background, fog, tone mapping, and color-space handling
- [ ] Add skeletal animation, animation player/state machine, and blend controls
- [ ] Add particles and simple procedural meshes
- [ ] Add toon/anime shading as a separate material/shader feature
- [ ] Add optional water shader and screen-space effects only after performance baselines
- [ ] Add render-quality presets and GPU capability-based feature tiers

## M6 — Physics with Box2D
- [ ] Pin Box2D version and verify Android NDK build for target ABI(s)
- [ ] Create physics world lifecycle and fixed-timestep stepping
- [ ] Add body, shape/fixture, sensor, collision filter, and material components
- [ ] Bind collision/contact callbacks to engine events and custom script signals
- [ ] Synchronize 2D physics transforms with scene nodes without frame jitter
- [ ] Add inspector controls and debug draw overlay
- [ ] Add deterministic tests for collisions, sensors, filtering, and scene reload
- [ ] Keep Box2D optional until dependency and runtime tests pass in CI

## M7 — Input, audio, and game runtime
- [ ] Add touch, keyboard, controller, and action-map input abstraction
- [ ] Add mobile virtual controls and configurable touch zones
- [ ] Add audio playback, buses, volume controls, and resource lifecycle
- [ ] Add game pause/resume, focus changes, lifecycle callbacks, and save data
- [ ] Add exportable runtime/player mode separate from editor mode
- [ ] Add project export presets and packaging metadata

## M8 — Quality, security, and release
- [ ] Add unit tests, native tests, scene/script fixtures, and smoke tests
- [ ] Add CI checks for formatting, CMake configure/build, APK build, artifact presence, and size
- [ ] Run install/launch tests on emulator and at least one physical Vulkan Android device
- [ ] Test rotation, background/resume, low memory, Vulkan device loss, and app termination
- [ ] Audit native dependencies, licenses, permissions, and supply-chain pins
- [ ] Add release signing through GitHub Actions secrets (never commit signing keys)
- [ ] Produce versioned APK/AAB artifacts, checksums, changelog, and rollback notes
- [ ] Publish performance and known-limitations report for each release

## Definition of done for a milestone
1. Implementation is committed with focused changes.
2. Build and relevant tests pass in CI.
3. Logs/errors are reviewed; no success is claimed without evidence.
4. Documentation and this roadmap reflect actual behavior.
5. Device-dependent features are tested on hardware or explicitly marked unverified.
