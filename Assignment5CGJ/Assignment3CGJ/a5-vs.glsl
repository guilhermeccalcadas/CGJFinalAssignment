#version 330 core

// Inputs que vêm do C++
in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexcoord;

// Outputs para o Fragment Shader
out vec3 exPosition;      // Posição no Mundo (para a LUZ)
out vec3 exLocalPosition; // <--- NOVO: Posição Local (para a TEXTURA/NOISE)
out vec3 exNormal;
out vec2 exTexcoord;

// Uniforms
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

    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(inPosition, 1.0);
}