#version 450

// Inputs
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

// Outputs
layout(location = 0) out vec3 fragColor;

// Uniforms
layout(set = 0, binding = 0) uniform Model
{
    mat4 model;
} modelUbo;

layout(set = 0, binding = 1) uniform ViewProj
{
    mat4 view;
    mat4 proj;
} camUbo;


void main() {
    gl_Position = camUbo.proj * camUbo.view * modelUbo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
