#version 450

// Inputs
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uvCoords;

// Outputs
layout(location = 0) out vec4 outColor;

// Uniforms
layout(set = 1, binding = 0) uniform UniformColor
{
    vec4 col;
} colorModifier;
layout(set = 1, binding = 1) uniform sampler2D tex;

void main()
{
    outColor = vec4(colorModifier.col.rgb * fragColor * texture(tex, uvCoords).rgb, 1.0);
}