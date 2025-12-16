#version 330 core

layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexcoord;

out vec3 exPosition; // Posição no mundo
out vec3 exNormal;   // Normal no mundo
out vec2 exTexcoord;

uniform mat4 ModelMatrix;

uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main(void)
{
    // Calcular posição no mundo (World Space) para a luz funcionar corretamente
    exPosition = vec3(ModelMatrix * vec4(inPosition, 1.0));

    // Calcular Normal Matrix para corrigir distorções de escala e rotação
    // (Nota: Em aplicações reais, calcula-se isso no CPU e passa-se como uniform para performance)
    exNormal = mat3(transpose(inverse(ModelMatrix))) * inNormal;

    exTexcoord = inTexcoord;

    // Posição final do vértice na tela
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(inPosition, 1.0);
}