#version 450

// Inputs
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

// Outputs
layout(location = 0) out vec3 fragColor;

// Uniforms
layout(set = 0, binding = 0) uniform GlobalUBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} global;

void main() {
    gl_Position = global.proj * global.view * global.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
