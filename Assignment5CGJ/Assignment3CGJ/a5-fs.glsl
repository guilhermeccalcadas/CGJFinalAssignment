#version 330 core

in vec3 exPosition;
in vec3 exNormal;

out vec4 FragmentColor;

// Uniforms
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

uniform vec3 uColor;             // A COR PRINCIPAL (Madeira Clara / Cor do Veio)
uniform int uMaterialType;       // 1=Madeira, 2=Mármore, 3=Cera
uniform sampler2D uNoiseTexture; // A textura de ruído

void main(void)
{
    vec3 finalAlbedo = uColor;
    float shininess = 32.0;

    // --- 1. MADEIRA (Riscas Verticais + 2 Cores) ---
    if (uMaterialType == 1) {
        // 1. Definir as DUAS CORES
        vec3 woodLight = uColor;          // A cor que definiste no C++
        vec3 woodDark  = uColor * 0.4;    // Uma versão mais escura da mesma cor
        
        // 2. Ler o ruído para distorcer as linhas
        // Multiplicamos por valores diferentes para esticar o noise
        float noise = texture(uNoiseTexture, exPosition.xz * 0.5).r;

        // 3. Criar o padrão de TÁBUAS VERTICAIS
        // Usamos exPosition.x para criar riscas ao longo da largura
        // O noise * 2.0 faz com que as linhas não sejam perfeitamente retas
        float dist = exPosition.x * 10.0 + noise * 1.5;
        
        // A função sin() cria a repetição suave (onda)
        float pattern = sin(dist);
        
        // Normalizar de [-1, 1] para [0, 1]
        pattern = (pattern + 1.0) * 0.5;
        
        // 4. A MAGIA: INTERPOLAÇÃO SUAVE (mix) [cite: 1]
        // Onde o pattern é 0, usa woodDark. Onde é 1, usa woodLight.
        // O meio é a transição suave que o teu professor mostrou.
        finalAlbedo = mix(woodDark, woodLight, pattern);
        
        shininess = 16.0; // Madeira pouco brilhante
    }

    // --- 2. MÁRMORE (Nuvens + Fundo Branco) ---
    else if (uMaterialType == 2) {
        // 1. Definir as cores baseadas na foto de referência
        vec3 marbleBase = vec3(0.98, 0.98, 0.98); // Fundo Branco Puro
        vec3 veinColor  = uColor;                 // A cor escura que vem do C++

        // 2. Ler o ruído 3D (Triplanar simplificado para evitar riscas nos lados)
        // Usamos uma escala menor (* 0.2) para os veios serem grandes e espalhados
        float nX = texture(uNoiseTexture, exPosition.yz * 0.2).r;
        float nY = texture(uNoiseTexture, exPosition.xz * 0.2).r; // Topo
        float nZ = texture(uNoiseTexture, exPosition.xy * 0.2).r;
        
        // Misturar baseado na normal (para o padrão fluir nos lados e no topo)
        vec3 blend = abs(normalize(exNormal));
        blend /= (blend.x + blend.y + blend.z);
        float noise = nX * blend.x + nY * blend.y + nZ * blend.z;
        
        // 3. A NOVA MATEMÁTICA DE VEIOS (Sharp Veins)
        // Esta fórmula cria linhas finas onde o ruído cruza o valor médio (0.5)
        float veins = 1.0 - abs(noise - 0.5) * 2.0;

        // 4. Afiar os veios
        // Pow elevado (ex: 20.0) torna os veios muito finos e definidos.
        // Ajusta este número: 10.0 = veios grossos, 30.0 = veios muito finos.
        veins = pow(veins, 20.0); 

        // 5. Misturar
        // Onde 'veins' é alto (branco), usamos a cor escura do veio.
        // Onde é baixo (preto), usamos a base branca.
        finalAlbedo = mix(marbleBase, veinColor, veins);
        
        shininess = 128.0; // Mármore polido
    }

    // --- 3. CERA (Vela) ---
    else if (uMaterialType == 3) {
        // Apenas ruído suave sobre a cor original
        float noise = texture(uNoiseTexture, exPosition.xy * 2.0).r;
        finalAlbedo = uColor * (0.9 + 0.1 * noise);
        shininess = 20.0;
    }

    // --- CÁLCULO DA LUZ (Blinn-Phong) ---
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
    vec3 specular = specularStrength * spec * vec3(1.0); // Brilho branco

    // Resultado final
    vec3 result = (ambient + diffuse) * finalAlbedo + specular;
    FragmentColor = vec4(result, 1.0);
}