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

    // Meshes
    std::vector<mgl::Mesh*> MeshesList;
	mgl::Mesh* woodenSwordMesh = nullptr;
    mgl::Mesh* candleMesh = nullptr;
    mgl::Mesh* pedestalMesh = nullptr;

    // Scene Graph
    mgl::SceneNode* root = nullptr;
    mgl::SceneNode* woodenSwordNode = nullptr;
    mgl::SceneNode* candleNode = nullptr;
    mgl::SceneNode* pedestalNode = nullptr;
    std::vector<mgl::SceneNode*> nodes;

    // Animation parameters
    float t = 0.0f;
    float animSpeed = 0.5f;
    int animDirection = 0;

    void createMeshes();
    void createShaderPrograms();
    void createCamera();
    void drawScene();
    void updateCamera();
    glm::mat4 getModel(glm::vec3 pos, float rotX, float rotY, float rotZ, float scal);
    void drawMesh(mgl::Mesh* m, glm::vec3 pos, float rotX, float rotY, float rotZ, float scal);
    void createSceneGraph();
    static void calculateProjection(CameraInfo& cam, int width, int height);
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
    Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);


    Shaders->addUniform(mgl::MODEL_MATRIX);
    Shaders->addUniform("uColor");
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);

    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
}


void MyApp::createSceneGraph() {
    root = new mgl::SceneNode(nullptr, nullptr);

    TangramPiece* pedestal = new TangramPiece(pedestalMesh, glm::vec4(0.6f, 0.4f, 0.2f, 1.0f));
    pedestalNode = new mgl::SceneNode(pedestal, Shaders);
    pedestalNode->transform = glm::mat4(1.0f);
    root->addChild(pedestalNode);

    TangramPiece* woodenSword = new TangramPiece(woodenSwordMesh, glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));
    woodenSwordNode = new mgl::SceneNode(woodenSword, Shaders);
    woodenSwordNode->transform = glm::mat4(1.0f);
    pedestalNode->addChild(woodenSwordNode);

    TangramPiece* candle = new TangramPiece(candleMesh, glm::vec4(0.1f, 0.5f, 0.2f, 1.0f));
    candleNode = new mgl::SceneNode(candle, Shaders);
    candleNode->transform = glm::mat4(1.0f);
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


////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createMeshes();
    createShaderPrograms();
    createCamera();
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
            leftMousePressed = true;
            glfwGetCursorPos(win, &lastCameraPosX, &lastCameraPosY);
        }
        else if (action == GLFW_RELEASE) {
            leftMousePressed = false;
        }
    }
}

void MyApp::cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!rightMousePressed && !leftMousePressed) {
        lastCameraPosX = xpos;
        lastCameraPosY = ypos;
        return;
    }

    double distanceX = xpos - lastCameraPosX;
    double distanceY = ypos - lastCameraPosY;

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
