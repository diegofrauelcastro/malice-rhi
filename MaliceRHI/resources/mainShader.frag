#version 450

// Inputs
layout(location = 0) in vec3 fragColor;

layout(set = 1, binding = 0) uniform UniformColor
{
    vec4 col;
} colorModifier;

// Outputs
layout(location = 0) out vec4 outColor;


void main()
{
    outColor = vec4(fragColor, 1.0) * colorModifier.col;
}