# Ixo Engine

**Ixo Engine** is a mobile-first 3D engine/editor bootstrap for Android. The repository is organized around a native Vulkan layer, a touch-friendly Android editor shell, scene/node data, and Lua-based `.Ixo.scripting` assets.

## Build an APK
Push to `main` or run **Actions → Android APK**. The workflow is configured to build a debug APK and upload it as `ixo-engine-debug-apk`. Verify the workflow run before assuming an artifact was produced.

## Current bootstrap
- Android editor shell with project-root display, script editing, and save-to-app-storage.
- Native C++/Vulkan capability probe (not yet a complete swapchain renderer).
- Starter scene and node naming conventions.
- `.Ixo.scripting` Lua-style script convention and sample; LuaJIT execution is not yet integrated.
- Box2D dependency manifest and optional CMake FetchContent integration; scene physics is not yet wired up.
- GitHub Actions Android APK workflow definition.

## Development planning
- [Full development roadmap](docs/ROADMAP.md)
- [Milestone execution plan and acceptance gates](docs/MILESTONE_EXECUTION_PLAN.md)
- [Architecture notes](docs/ARCHITECTURE.md)

## Vulkan rendering requirement
The native Vulkan renderer must render through Vulkan surfaces, swapchains, pipelines, command buffers, and shaders. Android `setBackgroundColor()` must **not** be used in Vulkan renderer code or as a substitute for rendering a frame. Android UI styling remains separate from the native renderer. The current native code only probes Vulkan availability.

## Status
This is an initial foundation, not yet a production engine. Full Vulkan frame rendering, LuaJIT VM integration, scene serialization, asset import, and complete hierarchy/inspector tooling remain future milestones. Features must only be marked complete after implementation and verification.
