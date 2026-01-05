#version 330 core

in vec3 exPosition;      // Usado para LUZ (Mundo)
in vec3 exLocalPosition; // Usado para TEXTURA (Local - Colado ao objeto)
in vec3 exNormal;

out vec4 FragmentColor;

// Uniforms
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

// [CORREÇÃO] vec4 para aceitar RGB + Alpha (Transparência)
uniform vec4 uColor;             
uniform int uMaterialType;       
uniform sampler2D uNoiseTexture; 

void main(void)
{
    // Usamos apenas RGB para os cálculos de cor base
    vec3 finalAlbedo = uColor.rgb; 
    float shininess = 32.0;

    // --- 1. MADEIRA ---
    if (uMaterialType == 1) {
        vec3 woodLight = uColor.rgb;       
        vec3 woodDark  = uColor.rgb * 0.4; 
        
        // [CORREÇÃO] Usamos exLocalPosition! 
        // A textura agora usa as coordenadas do próprio objeto, não as da sala.
        float noise = texture(uNoiseTexture, exLocalPosition.xz * 0.5).r;
        
        // Riscas verticais baseadas no X local
        float dist = exLocalPosition.x * 10.0 + noise * 1.5;
        
        float pattern = sin(dist);
        pattern = (pattern + 1.0) * 0.5;
        
        finalAlbedo = mix(woodDark, woodLight, pattern);
        shininess = 16.0; 
    }

    // --- 2. MÁRMORE ---
    else if (uMaterialType == 2) {
        vec3 marbleBase = vec3(0.98, 0.98, 0.98); 
        vec3 veinColor  = uColor.rgb;      

        // [CORREÇÃO] Triplanar com coordenadas Locais
        float nX = texture(uNoiseTexture, exLocalPosition.yz * 0.2).r;
        float nY = texture(uNoiseTexture, exLocalPosition.xz * 0.2).r; 
        float nZ = texture(uNoiseTexture, exLocalPosition.xy * 0.2).r;
        
        vec3 blend = abs(normalize(exNormal));
        blend /= (blend.x + blend.y + blend.z);
        float noise = nX * blend.x + nY * blend.y + nZ * blend.z;
        
        float veins = 1.0 - abs(noise - 0.5) * 2.0;
        veins = pow(veins, 20.0); 

        finalAlbedo = mix(marbleBase, veinColor, veins);
        shininess = 128.0; 
    }

    // --- 3. CERA ---
    else if (uMaterialType == 3) {
        // [CORREÇÃO] Noise local
        float noise = texture(uNoiseTexture, exLocalPosition.xy * 2.0).r;
        finalAlbedo = uColor.rgb * (0.9 + 0.1 * noise); 
        shininess = 20.0;
    }

    // --- CÁLCULO DA LUZ (Blinn-Phong) ---
    // IMPORTANTE: A luz continua a usar exPosition (Mundo)
    float ambientStrength = 0.4;
    float specularStrength = 0.5;

    vec3 ambient = ambientStrength * uLightColor;

    vec3 norm = normalize(exNormal);
    vec3 lightDir = normalize(uLightPos - exPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    vec3 viewDir = normalize(uViewPos - exPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0); 

    // Resultado final
    vec3 result = (ambient + diffuse) * finalAlbedo + specular;
    
    // Usamos o Alpha original para permitir transparência no espelho
    FragmentColor = vec4(result, uColor.a);
}