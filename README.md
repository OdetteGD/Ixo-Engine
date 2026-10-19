# Ixo Engine

**Ixo Engine** is a mobile-first 3D engine/editor bootstrap for Android. The repository is organized around a Vulkan native layer, a touch-friendly Android editor shell, scene/node data, and Lua-based `.Ixo.scripting` assets.

## Build an APK
Push to `main` or run **Actions → Android APK**. The workflow builds a debug APK and uploads it as `ixo-engine-debug-apk`. Download it from the workflow run's Artifacts section.

## Current bootstrap
- Android editor shell with project-root display, script editing, and save-to-app-storage.
- Native C++/Vulkan capability probe (the current renderer bootstrap is not yet a complete swapchain renderer).
- Scene node model and starter scene.
- `.Ixo.scripting` Lua-style script convention and sample.
- Box2D dependency manifest and optional CMake FetchContent integration.
- GitHub Actions Android APK build.

## Important status
This is an initial, build-oriented foundation—not yet a production engine. Full Vulkan swapchain rendering, LuaJIT VM integration, scene serialization, asset import, and a complete hierarchy/inspector are follow-up milestones. See [the roadmap](docs/ROADMAP.md).
