#version 330 core

layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexcoord;

out vec3 exPosition;
out vec2 exTexcoord;
flat out vec3 exNormal; // flat para manter a mesma normal por face

uniform mat4 ModelMatrix;

uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main(void)
{
    exPosition = inPosition;
    exNormal = inNormal; // a normal "flat" por face
    exTexcoord = inTexcoord;

    vec4 MCPosition = vec4(inPosition, 1.0);
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * MCPosition;
}
