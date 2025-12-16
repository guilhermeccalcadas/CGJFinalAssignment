#version 330 core

in vec3 exPosition;
flat in vec3 exNormal; // recebe a normal "flat" do vertex shader

out vec4 FragmentColor;

uniform vec3 uColor;

void main(void)
{
    // usa a normal flat para gerar variação
    vec3 variation = 0.15 * (exNormal); 

    // combina com a cor base
    vec3 finalColor = clamp(uColor + variation, 0.0, 1.0);

    FragmentColor = vec4(finalColor, 1.0);
}
