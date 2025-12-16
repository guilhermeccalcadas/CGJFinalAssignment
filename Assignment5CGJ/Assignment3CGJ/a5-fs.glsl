#version 330 core

in vec3 exPosition;
in vec3 exNormal;
// in vec2 exTexcoord; // Se fores usar texturas depois, descomenta

out vec4 FragmentColor;

uniform vec3 uColor;     // Cor do Objeto (Material)
uniform vec3 uLightPos;  // Posição da Luz (Chama da vela)
uniform vec3 uViewPos;   // Posição da Câmera

void main(void)
{
    // 1. Configurações da Luz (Podes passar por uniform depois se quiseres)
    vec3 lightColor = vec3(1.0, 0.9, 0.8); // Luz levemente amarela (vela)
    float ambientStrength = 0.2;
    float specularStrength = 0.5;

    // 2. Ambient (Luz base)
    vec3 ambient = ambientStrength * lightColor;

    // 3. Diffuse (Luz direcional básica)
    vec3 norm = normalize(exNormal);
    vec3 lightDir = normalize(uLightPos - exPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 4. Specular (Brilho reflexivo)
    vec3 viewDir = normalize(uViewPos - exPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 é o "shininess"
    vec3 specular = specularStrength * spec * lightColor;

    // 5. Combinação Final
    vec3 result = (ambient + diffuse + specular) * uColor;
    
    FragmentColor = vec4(result, 1.0);
}