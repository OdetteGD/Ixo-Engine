# Ixo Engine Vulkan 3D test scene

This scene describes a deterministic mobile test world for the native Vulkan renderer: a perspective camera, ground plane, grid, directional light and a colored test cube. Scene coordinates are right-handed, +Y up. The renderer must consume these node transforms and camera values; this file is not itself a rendering implementation.

## Shader contract
- `shaders/mesh.vert`: position + vertex color, camera view-projection and model matrices.
- `shaders/mesh.frag`: outputs interpolated vertex color.
- Vulkan vertex input: binding 0, stride 24 bytes; location 0 = vec3 position at offset 0, location 1 = vec3 color at offset 12.
- Push constants: 128 bytes, vertex stage; viewProjection mat4 then model mat4.
- Grid: line-list geometry on the XZ plane at y=0, minor/major colors, depth test enabled.

## Asset pipeline
Higgsfield/Blender-generated GLB assets should be exported as glTF 2.0, validated for scale/origin/normals, and converted to a runtime mesh representation before adding to the scene. Keep visual reference images in `assets/references`; do not treat a reference image as a 3D model.

## Current implementation boundary
The Android app already creates an Android Vulkan surface and swapchain. These shader sources and scene contract are groundwork; a shader compiler and runtime pipeline/buffer binding must be integrated before this viewport draws meshes. Do not claim mesh rendering is complete until the Android workflow builds and the device viewport visibly shows the grid and cube.
