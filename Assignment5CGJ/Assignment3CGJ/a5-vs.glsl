#version 330 core

layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexcoord;

out vec3 exPosition;
out vec3 exNormal;
out vec2 exTexcoord;

uniform mat4 ModelMatrix;

uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main(void)
{

    exPosition = vec3(ModelMatrix * vec4(inPosition, 1.0));


    exNormal = mat3(transpose(inverse(ModelMatrix))) * inNormal;

    exTexcoord = inTexcoord;


    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(inPosition, 1.0);
}