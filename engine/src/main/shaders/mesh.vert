#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 color;
layout(push_constant) uniform Camera {
    mat4 viewProjection;
    mat4 model;
} camera;
void main() {
    gl_Position = camera.viewProjection * camera.model * vec4(inPosition, 1.0);
    color = inColor;
}