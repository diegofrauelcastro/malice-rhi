#version 450

// Inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUVCoords;

// Outputs
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uvCoords;

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

// Push constants
layout( push_constant ) uniform constants
{
    vec3 colorPushConst;
} PushConstants;
layout( push_constant ) uniform constants2
{
    layout(offset = 12) float alpha;
} PushConstants2;

void main()
{
    gl_Position = camUbo.proj * camUbo.view * modelUbo.model * vec4(inPosition, 1.0);
    fragColor = inColor * PushConstants.colorPushConst;
    uvCoords = inUVCoords;
}
