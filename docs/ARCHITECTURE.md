# Ixo Engine architecture

## Repository destinations
- `app/`: Android app and mobile editor UI.
- `engine/`: C++20 native module and Vulkan capability probe.
- `project/`: starter scene and `.Ixo.scripting` assets.
- `config/ixo-project.yml`: project paths, renderer, script runtime, and physics configuration.
- `.github/workflows/`: reproducible Android APK CI.

## Runtime boundaries
The Android activity currently provides a basic touch-friendly script editor and saves scripts under the app's private `files/IxoProject/Scripts` directory. The native library checks Vulkan instance/device availability; it does not yet create a swapchain or render frames. Box2D is an opt-in CMake dependency, not yet exposed as scene-node physics components. The script editor currently edits Lua-style text; LuaJIT execution and engine bindings are not yet wired in.

## Planned engine layers
1. Platform/window and Vulkan swapchain, command buffers, depth, frame synchronization.
2. Scene graph: Node3D, Camera3D, MeshInstance3D, DirectionalLight3D, physics bodies.
3. Asset pipeline: glTF/GLB, materials, textures, import cache.
4. Ixo scripting: LuaJIT VM, lifecycle callbacks, node API, signals, exported properties.
5. Editor: project browser, hierarchy, inspector, viewport, transform gizmos, scene save/load.
