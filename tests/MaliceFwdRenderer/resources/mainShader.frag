#version 450

// Inputs
layout(location = 0) in vec3 fragColor;

// Outputs
layout(location = 0) out vec4 outColor;

// Uniforms
layout(set = 1, binding = 0) uniform UniformColor
{
    vec4 col;
} colorModifier;


void main()
{
    outColor = vec4(fragColor, 1.0) * colorModifier.col;
}