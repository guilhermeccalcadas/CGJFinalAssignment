////////////////////////////////////////////////////////////////////////////////
//
//  Loading meshes from external files
//
// Copyright (c) 2023-25 by Carlos Martinho
//
// INTRODUCES:
// MODEL DATA, ASSIMP, mglMesh.hpp
//
////////////////////////////////////////////////////////////////////////////////

#include "../mgl/mgl.hpp"
#include "../mgl/mglSceneNode.hpp"
#include <iostream>
#include <SceneObject.hpp>


////////////////////////////////////////////////////////////////////////// MYAPP

class MyApp : public mgl::App {
public:
    void initCallback(GLFWwindow* win) override;
    void displayCallback(GLFWwindow* win, double elapsed) override;
    void windowSizeCallback(GLFWwindow* win, int width, int height) override;
    void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;
    void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) override;
    void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) override;
    void cursorCallback(GLFWwindow* window, double xpos, double ypos) override;

private:
    // Camera control parameters
    mgl::Camera* Camera = nullptr;
    const float zoomSpeed = 1.0f;
    const float minRadius = 2.0f;
    const float maxRadius = 50.0f;
    struct CameraInfo {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::quat rotation;
        float radius;
        bool isOrtho;
    };
    CameraInfo cam1;
    CameraInfo* activeCam = nullptr;
    glm::vec3 target = glm::vec3(0.0f, 0.5f, 0.0f);

    // Mouse control parameters
    bool rightMousePressed = false;
    bool leftMousePressed = false;
    float panSpeed = 0.01f;
    double lastCameraPosX = 0.0f;
    double lastCameraPosY = 0.0f;
    float rotationSpeed = 0.006f;
    float pitchLimit = glm::radians(89.0f);

    // Shader program
    const GLuint UBO_BP = 0;
    mgl::ShaderProgram* Shaders = nullptr;

    // Model matrix uniform location
    GLint ModelMatrixId;
    GLint LightPosId;
    GLint ViewPosId;
    GLint LightColorId;

    // Meshes
    std::vector<mgl::Mesh*> MeshesList;
	mgl::Mesh* woodenSwordMesh = nullptr;
    mgl::Mesh* candleMesh = nullptr;
    mgl::Mesh* pedestalMesh = nullptr;

    GLuint texMadeiraID;
    GLuint texMarmoreID;
    GLuint texVelaID;

    // Scene Graph
    mgl::SceneNode* root = nullptr;
    mgl::SceneNode* woodenSwordNode = nullptr;
    mgl::SceneNode* candleNode = nullptr;
    mgl::SceneNode* pedestalNode = nullptr;
    std::vector<mgl::SceneNode*> nodes;

    // Modos de Edição
    enum OpMode { NONE, TRANSLATE, ROTATE, SCALE };
    enum Axis { AXIS_X, AXIS_Y, AXIS_Z };

    OpMode currentMode = NONE;      // Começa sem fazer nada
    Axis currentAxis = AXIS_X;      // Eixo default
    mgl::SceneNode* selectedNode = nullptr; // Quem está selecionado?

    // Rato
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    void createMeshes();
    void createShaderPrograms();
    void createCamera();
    void drawScene();
    void updateCamera();
    glm::mat4 getModel(glm::vec3 pos, float rotX, float rotY, float rotZ, float scal);
    void drawMesh(mgl::Mesh* m, glm::vec3 pos, float rotX, float rotY, float rotZ, float scal);
    void createSceneGraph();
    static void calculateProjection(CameraInfo& cam, int width, int height);
    int pickObject(GLFWwindow* win, double mouseX, double mouseY);
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
    std::string mesh_dir = "./assets/models/";
    /*
    std::vector<std::string> files = {
        "Table.obj",
        "Triangle1.obj",
        "Triangle2.obj",
        "Triangle4.obj",
        "Triangle6.obj",
        "Triangle7.obj",
        "Paralelogram3a.obj",
        "Cube5.obj"
    };

    for (const auto& f : files) {
        mgl::Mesh* m = new mgl::Mesh();
        m->joinIdenticalVertices();
        m->create(mesh_dir + f);
        MeshesList.push_back(m);
    }
    */

    woodenSwordMesh = new mgl::Mesh();
    woodenSwordMesh->joinIdenticalVertices();
    woodenSwordMesh->create(mesh_dir + "wooden_sword.obj");

    candleMesh = new mgl::Mesh();
    candleMesh->joinIdenticalVertices();
    candleMesh->create(mesh_dir + "candle.obj");

    pedestalMesh = new mgl::Mesh();
    pedestalMesh->joinIdenticalVertices();
    pedestalMesh->create(mesh_dir + "pedestal.obj");
}


///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    Shaders = new mgl::ShaderProgram();
    Shaders->addShader(GL_VERTEX_SHADER, "a5-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "a5-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);


    Shaders->addUniform(mgl::MODEL_MATRIX);
    Shaders->addUniform("uColor");
    Shaders->addUniform("uLightPos");
    Shaders->addUniform("uViewPos");
    Shaders->addUniform("uLightColor");
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);

    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
    LightPosId = Shaders->Uniforms["uLightPos"].index;
    ViewPosId = Shaders->Uniforms["uViewPos"].index;
    LightColorId = Shaders->Uniforms["uLightColor"].index;
}


void MyApp::createSceneGraph() {
    root = new mgl::SceneNode(nullptr, nullptr);

    // --- 1. PEDESTAL (Mármore - ID 2) ---
    // Cor escura para os veios (0.2), ID 2
    SceneObject* pedestal = new SceneObject(pedestalMesh, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), texMarmoreID, 2);
    pedestalNode = new mgl::SceneNode(pedestal, Shaders);
    pedestalNode->transform = glm::mat4(1.0f);
    pedestalNode->transform[3] = glm::vec4(5.0f, 1.0f, 3.0f, 1.0f);
    root->addChild(pedestalNode);

    // --- 2. ESPADA (Madeira - ID 1) ---
    // [CORREÇÃO]: Mudei o último número de 2 para 1
    SceneObject* woodenSword = new SceneObject(woodenSwordMesh, glm::vec4(0.60f, 0.35f, 0.15f, 1.0f), texMadeiraID, 1);
    woodenSwordNode = new mgl::SceneNode(woodenSword, Shaders);
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(0.0f, 10.0f, 0.0f));
    m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    woodenSwordNode->setTransform(m);
    pedestalNode->addChild(woodenSwordNode);

    // --- 3. VELA (Cera - ID 3) ---
    // [CORREÇÃO]: Mudei o último número de 2 para 3
    SceneObject* candle = new SceneObject(candleMesh, glm::vec4(0.92f, 0.88f, 0.70f, 1.0f), texVelaID, 3);
    candleNode = new mgl::SceneNode(candle, Shaders);
    candleNode->setTransform(glm::mat4(1.0f));
    root->addChild(candleNode);
}

///////////////////////////////////////////////////////////////////////// CAMERA


void MyApp::createCamera() {
    Camera = new mgl::Camera(UBO_BP);

    // --- SETUP CAM 1 ---
    glm::vec3 eye1(0.0f, 10.0f, 0.0f);
    glm::vec3 up1(0.0f, 0.0f, -1.0f);

    cam1.radius = glm::length(eye1 - target);
    cam1.rotation = glm::quatLookAt(glm::normalize(target - eye1), up1);
    cam1.viewMatrix = glm::lookAt(eye1, target, up1);
    cam1.isOrtho = false;

    calculateProjection(cam1, 800, 600);
    activeCam = &cam1;
    Camera->setViewMatrix(activeCam->viewMatrix);
    Camera->setProjectionMatrix(activeCam->projectionMatrix);
}


/////////////////////////////////////////////////////////////////////////// DRAW

glm::mat4 ModelMatrix;

glm::mat4 MyApp::getModel(glm::vec3 pos, float rotX, float rotY, float rotZ, float scal) {
    glm::mat4 M = glm::translate(glm::mat4(1.0f), pos)
        * glm::rotate(glm::mat4(1.0f), glm::radians(rotX), glm::vec3(1, 0, 0))
        * glm::rotate(glm::mat4(1.0f), glm::radians(rotY), glm::vec3(0, 1, 0))
        * glm::rotate(glm::mat4(1.0f), glm::radians(rotZ), glm::vec3(0, 0, 1))
        * glm::scale(glm::mat4(1.0f), glm::vec3(scal, scal, 1.0f));
    return M;
}

void MyApp::drawMesh(mgl::Mesh* m, glm::vec3 pos, float rotX, float rotY, float rotZ, float scal) {
    ModelMatrix = getModel(pos, rotX, rotY, rotZ, scal);
    glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    m->draw();
}

void MyApp::drawScene() {
    Shaders->bind();

    glm::vec3 camPos;
    if (activeCam) {

        glm::vec3 initialPos(0.0f, 0.0f, activeCam->radius);
        camPos = activeCam->rotation * initialPos + target;
    }
    glUniform3fv(ViewPosId, 1, glm::value_ptr(camPos));

    glm::vec4 localFlamePos = glm::vec4(0.0f, 0.75f, 0.0f, 1.0f);
    glm::vec3 globalFlamePos = glm::vec3(candleNode->getTransform() * localFlamePos);


    glUniform3fv(LightPosId, 1, glm::value_ptr(globalFlamePos));
    glUniform3f(LightColorId, 0.9f, 0.9f, 0.9f);

    root->draw();
}

/////////////////////////////////////////////////////////////////////////// Auxiliary Methods

void MyApp::updateCamera() {
    glm::vec3 initialPos(0.0f, 0.0f, activeCam->radius);
    glm::vec3 rotatedPos = activeCam->rotation * initialPos;
    glm::vec3 localUp = activeCam->rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(rotatedPos + target, target, glm::normalize(localUp));
    activeCam->viewMatrix = view;

    Camera->setViewMatrix(activeCam->viewMatrix);
}

void MyApp::calculateProjection(CameraInfo& cam, int width, int height) {
    float aspect = (float)width / (float)height;
    if (height == 0) aspect = 1.0f;

    if (cam.isOrtho) {
        float orthoSize = 2.0f;
        cam.projectionMatrix = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 1.0f, 50.0f);
    }
    else {
        cam.projectionMatrix = glm::perspective(glm::radians(30.0f), aspect, 1.0f, 50.0f);
    }
}

// Devolve: 0 (Nada), 1 (Vela), 2 (Pedestal)
// Devolve: 0 (Nada), 1 (Vela), 2 (Pedestal)
int MyApp::pickObject(GLFWwindow* win, double mouseX, double mouseY) {
    // 1. Limpar Buffers (incluindo o Stencil!)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // 2. Ativar Stencil e Configurar
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Precisamos do shader ativo
    Shaders->bind();

    // IMPORTANTE: Se o shader precisar da ViewMatrix atualizada, 
    // garante que a câmara enviou os dados (normalmente o updateCamera trata disso).

    // --- DESENHAR VELA (ID 1) ---
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    if (candleNode) candleNode->draw();

    // --- DESENHAR PEDESTAL (ID 2) ---
    glStencilFunc(GL_ALWAYS, 2, 0xFF);
    if (pedestalNode) pedestalNode->draw();

    // 3. Ler o ID no pixel do rato
    int width, height;
    glfwGetWindowSize(win, &width, &height); // Usa o 'win' que recebemos

    int x = (int)mouseX;
    int y = height - (int)mouseY; // Inverter Y

    unsigned int index = 0;
    glReadPixels(x, y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

    // 4. Limpar
    glDisable(GL_STENCIL_TEST);

    return index;
}

// 1. GERADOR DE MADEIRA (Gera riscas/fibras verticais)
GLuint createWoodTexture(int seed) {
    srand(seed);
    const int WIDTH = 128;
    const int HEIGHT = 128;
    unsigned char data[WIDTH * HEIGHT * 4];

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Truque: O valor muda muito no X (largura), mas pouco no Y (altura)
            // Isso cria o efeito de "fibras" longas
            float scaleX = 0.5f;
            float scaleY = 0.05f; // Esticado verticalmente

            // Simulação simples de perlin noise no CPU
            int noiseVal = rand() % 255;

            // Mas vamos aldrabar: riscas verticais misturadas com noise
            // (x * 10) cria o padrão de repetição de tábuas
            int stripPattern = (int)(abs(sin(x * 0.2f)) * 255);

            // Mistura o padrão de riscas com o ruído
            unsigned char finalVal = (unsigned char)((stripPattern * 0.7) + (noiseVal * 0.3));

            data[(y * WIDTH + x) * 4 + 0] = finalVal;
            data[(y * WIDTH + x) * 4 + 1] = finalVal;
            data[(y * WIDTH + x) * 4 + 2] = finalVal;
            data[(y * WIDTH + x) * 4 + 3] = 255;
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

// 2. GERADOR DE MÁRMORE (Nuvens suaves, clássico)
GLuint createMarbleTexture(int seed) {
    srand(seed);
    const int SIZE = 64; // Menor para ficar mais "borrado" (suave)
    unsigned char data[SIZE * SIZE * 4];

    for (int i = 0; i < SIZE * SIZE; i++) {
        unsigned char val = rand() % 255;
        data[i * 4 + 0] = val;
        data[i * 4 + 1] = val;
        data[i * 4 + 2] = val;
        data[i * 4 + 3] = 255;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // OBRIGATÓRIO PARA MÁRMORE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

// 3. GERADOR DE VELA (Grão muito fino)
GLuint createCandleTexture(int seed) {
    srand(seed);
    const int SIZE = 256; // Maior resolução para grão fino
    unsigned char data[SIZE * SIZE * 4];

    for (int i = 0; i < SIZE * SIZE; i++) {
        // Ruído subtil (entre 200 e 255) para não ter pontos pretos
        unsigned char val = 200 + (rand() % 55);
        data[i * 4 + 0] = val;
        data[i * 4 + 1] = val;
        data[i * 4 + 2] = val;
        data[i * 4 + 3] = 255;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}


////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createMeshes();
    createShaderPrograms();
    createCamera();
    texMadeiraID = createWoodTexture(100);    // Gera fibras
    texMarmoreID = createMarbleTexture(200);  // Gera nuvens
    texVelaID = createCandleTexture(300);  // Gera grão fino
    createSceneGraph();
}

void MyApp::windowSizeCallback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
    calculateProjection(cam1, width, height);
    if (Camera && activeCam) {
        Camera->setProjectionMatrix(activeCam->projectionMatrix);
    }
}

void MyApp::displayCallback(GLFWwindow* win, double elapsed) {
    drawScene();
}

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_G) { currentMode = TRANSLATE; std::cout << ">> Mode: TRANSLATE" << std::endl; }
        if (key == GLFW_KEY_R) { currentMode = ROTATE;    std::cout << ">> Mode: ROTATE" << std::endl; }
        if (key == GLFW_KEY_S) { currentMode = SCALE;     std::cout << ">> Mode: SCALE" << std::endl; }

        // --- SELECIONAR EIXO ---
        if (key == GLFW_KEY_X) { currentAxis = AXIS_X; std::cout << ">> Axis: X" << std::endl; }
        if (key == GLFW_KEY_Y) { currentAxis = AXIS_Y; std::cout << ">> Axis: Y" << std::endl; }
        if (key == GLFW_KEY_Z) { currentAxis = AXIS_Z; std::cout << ">> Axis: Z" << std::endl; }

        // Reset (ESC)
        if (key == GLFW_KEY_ESCAPE) {
            currentMode = NONE;
            selectedNode = nullptr;
            std::cout << ">> Mode: NONE (Deselected)" << std::endl;
        }
        switch (key) {
        case GLFW_KEY_P:
            activeCam->isOrtho = !activeCam->isOrtho;
            int w, h;
            glfwGetWindowSize(win, &w, &h);
            calculateProjection(*activeCam, w, h);
            Camera->setProjectionMatrix(activeCam->projectionMatrix);

            //std::cout << "Projection: " << (activeCam->isOrtho ? "Ortho" : "Perspective") << std::endl;
            break;
        default:
            break;
        }
    }



}

void MyApp::scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    if (!activeCam->isOrtho) {
        activeCam->radius -= yoffset * zoomSpeed;

        if (activeCam->radius < minRadius) activeCam->radius = minRadius;
        if (activeCam->radius > maxRadius) activeCam->radius = maxRadius;

        updateCamera();
    }
}

void MyApp::mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            glfwGetCursorPos(win, &lastCameraPosX, &lastCameraPosY);
        }
        else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {

            leftMousePressed = true; // [NOVO] Faltava isto!

            double x, y;
            glfwGetCursorPos(win, &x, &y);

            // 1. Fazer o Picking
            int id = pickObject(win, x, y);

            // 2. Atualizar quem está selecionado
            if (id == 1) {
                selectedNode = candleNode;
                std::cout << "Selecionado: VELA" << std::endl;
            }
            else if (id == 2) {
                selectedNode = pedestalNode;
                std::cout << "Selecionado: PEDESTAL" << std::endl;
            }
            else {
                selectedNode = nullptr;
                std::cout << "Selecionado: NADA" << std::endl;
            }

            // Iniciar arrasto
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (action == GLFW_RELEASE) {
            isDragging = false;
            leftMousePressed = false; // [NOVO] Faltava isto!
        }
    }
}

void MyApp::cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    // 1. Se nenhum botão estiver pressionado, apenas atualiza a posição e sai
    if (!rightMousePressed && !leftMousePressed) {
        lastCameraPosX = xpos;
        lastCameraPosY = ypos;
        return;
    }

    // 2. Calcular quanto o rato mexeu desde o último frame
    double distanceX = xpos - lastCameraPosX;
    double distanceY = ypos - lastCameraPosY;

    // --- BLOCO DA CÂMARA (TEU CÓDIGO ANTIGO) ---
    // Funciona quando carregas no botão DIREITO
    if (rightMousePressed) {
        float yawAngle = -distanceX * rotationSpeed;
        float pitchAngle = -distanceY * rotationSpeed;

        glm::vec3 localUp = activeCam->rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        glm::quat yaw = glm::angleAxis(yawAngle, glm::normalize(localUp));
        glm::vec3 localRight = activeCam->rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::quat pitch = glm::angleAxis(pitchAngle, glm::normalize(localRight));

        glm::quat orientation = glm::normalize(pitch * yaw * activeCam->rotation);
        activeCam->rotation = orientation;

        updateCamera();
    }

    // --- BLOCO DE MANIPULAÇÃO DE OBJETOS ---
    if (leftMousePressed && selectedNode != nullptr && currentMode != NONE) {

        float sensitivity = 0.0115f; // Ajusta a gosto

        // 1. Definir o vetor do Eixo no Mundo (Global)
        glm::vec3 axisVector(0.0f);
        if (currentAxis == AXIS_X) axisVector.x = 1.0f;
        if (currentAxis == AXIS_Y) axisVector.y = 1.0f;
        if (currentAxis == AXIS_Z) axisVector.z = 1.0f;

        // 2. CORREÇÃO DA ROTAÇÃO (O "Truque" do Blender)
        // Transformamos o eixo do Mundo para o espaço da Câmara (View Space)
        // O glm::vec4(axisVector, 0.0f) garante que só rodamos a direção (ignoramos translação)
        glm::vec3 axisInView = glm::vec3(activeCam->viewMatrix * glm::vec4(axisVector, 0.0f));

        // Agora axisInView.x diz-nos se o eixo aponta para a direita ou esquerda do ecrã
        // e axisInView.y diz-nos se aponta para cima ou baixo.

        // 3. Projetar o movimento do rato nesse eixo visual
        // Basicamente: "Quanto do meu movimento de rato coincide com a direção visual deste eixo?"
        float screenMoveX = (float)distanceX;
        float screenMoveY = -(float)distanceY; // Inverter Y do rato (baixo é positivo em pixel, mas negativo em 3D)

        // Produto Interno (Dot Product) 2D
        float alignment = (screenMoveX * axisInView.x) + (screenMoveY * axisInView.y);

        // Se o eixo estiver "de ponta" para nós (Z), usamos o movimento Y do rato como fallback
        // para parecer zoom in/out intuitivo
        if (glm::abs(axisInView.z) > 0.7f && currentAxis == AXIS_Z) {
            // Caso especial para eixo Z quando olhamos de frente
            alignment = screenMoveY * ((axisInView.z > 0) ? -1.0f : 1.0f);
        }

        float value = alignment * sensitivity;

        // 4. Aplicar a Transformação
        if (currentMode == TRANSLATE) {
            selectedNode->transform = glm::translate(selectedNode->transform, axisVector * value);
        }
        else if (currentMode == ROTATE) {
            selectedNode->transform = glm::rotate(selectedNode->transform, value * 5.0f, axisVector);
        }
        else if (currentMode == SCALE) {
            float scaleVal = 1.0f + (value * 0.5f);
            selectedNode->transform = glm::scale(selectedNode->transform, glm::vec3(scaleVal));
        }
    }

    // 3. Atualizar a última posição para o próximo frame
    lastCameraPosX = xpos;
    lastCameraPosY = ypos;
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char* argv[]) {
    mgl::Engine& engine = mgl::Engine::getInstance();
    engine.setApp(new MyApp());
    engine.setOpenGL(4, 6);
    engine.setWindow(800, 600, "Create Pickagram 3D", 0, 1);
    engine.init();
    engine.run();
    exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
