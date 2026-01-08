#version 330 core


in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexcoord;


out vec3 exPosition;
out vec3 exLocalPosition;
out vec3 exNormal;
out vec2 exTexcoord;
out vec3 exLocalNormal;

uniform mat4 ModelMatrix;

layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main(void)
{
    exLocalPosition = inPosition; 

    exPosition = vec3(ModelMatrix * vec4(inPosition, 1.0));
    
    mat3 NormalMatrix = transpose(inverse(mat3(ModelMatrix)));
    exNormal = NormalMatrix * inNormal;

    exTexcoord = inTexcoord;
    exLocalNormal = inNormal;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(inPosition, 1.0);
}