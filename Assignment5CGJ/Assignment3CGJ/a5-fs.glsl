#version 330 core

in vec3 exPosition;
in vec3 exNormal;

out vec4 FragmentColor;

uniform vec3 uColor;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

void main(void)
{
    float ambientStrength = 0.2;
    float specularStrength = 0.5;
    float shininess = 128.0;


    vec3 ambient = ambientStrength * uLightColor;


    vec3 norm = normalize(exNormal);
    vec3 lightDir = normalize(uLightPos - exPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;


    vec3 viewDir = normalize(uViewPos - exPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * uLightColor;

    vec3 result = (ambient + diffuse + specular) * uColor;
    FragmentColor = vec4(result, 1.0);
}