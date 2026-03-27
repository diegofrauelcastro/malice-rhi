#version 450

// Inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUVCoords;

// Outputs
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uvCoords;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    fragColor = vec3(1.0);
    uvCoords = inUVCoords;
}
