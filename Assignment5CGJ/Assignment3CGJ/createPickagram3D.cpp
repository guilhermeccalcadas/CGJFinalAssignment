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
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) override;
  void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) override;
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
  CameraInfo cam2;
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
  mgl::ShaderProgram *Shaders = nullptr;

  // Model matrix uniform location
  GLint ModelMatrixId;

  // Meshes
  std::vector<mgl::Mesh*> MeshesList;

  // Scene Graph
  mgl::SceneNode* root = nullptr;
  mgl::SceneNode* tableNode = nullptr;
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
  void updatePieceTransforms(float dt);
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
    std::string mesh_dir = "./assets/models/";

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
}


///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    Shaders = new mgl::ShaderProgram();
    Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    if (MeshesList[0]->hasNormals()) {
        Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    }
    if (MeshesList[0]->hasTexcoords()) {
        Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    }
    if (MeshesList[0]->hasTangentsAndBitangents()) {
        Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);
    }

    Shaders->addUniform(mgl::MODEL_MATRIX);
    Shaders->addUniform("uColor");
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);

    Shaders->create();

    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
}


struct TransformationConfiguration {
    glm::vec3 pos;
    float rotX;
    float rotY;
    float rotZ;
};

const float SIDE = 0.89f; //length of a side of the pickagram in square form

TransformationConfiguration sealConfig[7] = {
    { glm::vec3(0.4232f, 0.8375f, 0.0f), 0.0f, 0.0f, 108.0f }, //triangle 1
    { glm::vec3(0.7628f, 0.8108f, 0.0f), 0.0f, 0.0f, 153.0f }, //triangle 2
    { glm::vec3(1.0905f, 0.8204f, 0.0f), 0.0f, 0.0f, 18.0f }, //triangle 4
    { glm::vec3(-0.0688f, 0.9116f, 0.0f), 0.0f, 0.0f, 198.0f }, //triangle 6
    { glm::vec3(0.0053f, 1.4036f, 0.0f), 0.0f, 0.0f, 18.0f }, // triangle 7
    { glm::vec3(-0.1402, 0.7714f, 0.0f), 0.0f, 0.0f, 288.0f }, //paralellogram 3a
    { glm::vec3(0.0741f, 1.1920f, 0.0f), 0.0f, 0.0f, 198.0f } //square 5
};

TransformationConfiguration squareConfig[7] = {
    { glm::vec3(-SIDE / 2 - 0.5f, 0.5f, 0.0f), 90.0f, 0.0f, -90.0f }, //triangle 1
    { glm::vec3(-0.5f, 0.5f, -SIDE / 2), 90.0f, 0.0f, 0.0f }, //triangle 2
    { glm::vec3(-0.5f, 0.5f, SIDE / 4), 90.0f, 0.0f, 0.0f }, //triangle 4
    { glm::vec3(SIDE / 2 - 0.5f, 0.5f, -SIDE / 4), 90.0f, 0.0f, 0.0f }, //triangle 6
    { glm::vec3(SIDE / 4 - 0.5f, 0.5f, SIDE / 4), 90.0f, 0.0f, 0.0f }, // triangle 7
    { glm::vec3(-SIDE / 8 - 0.5f, 0.5f, SIDE / 4 + SIDE / 8), 90.0f, 0.0f, 0.0f }, //paralellogram 3a
    { glm::vec3(SIDE / 4 - 0.5f, 0.5f, 0.0f), 90.0f, 0.0f, 0.0f } //square 5
};

void MyApp::createSceneGraph() {
    root = new mgl::SceneNode(nullptr, nullptr);

    // Mesa
    TangramPiece* tablePiece = new TangramPiece(MeshesList[0], glm::vec4(0.6f, 0.4f, 0.2f, 1.0f));
    tableNode = new mgl::SceneNode(tablePiece, Shaders);
    tableNode->transform = glm::mat4(1.0f);
    root->addChild(tableNode);

    // Peça 0 - Triangle 1
    TangramPiece* p0 = new TangramPiece(MeshesList[1], glm::vec4(0.9f, 0.1f, 0.1f, 1.0f));
    auto n0 = new mgl::SceneNode(p0, Shaders);
    n0->transform = getModel(squareConfig[0].pos, squareConfig[0].rotX, squareConfig[0].rotY, squareConfig[0].rotZ, 1.0f);
    tableNode->addChild(n0);
    nodes.push_back(n0);

    // Peça 1 - Triangle 2
    TangramPiece* p1 = new TangramPiece(MeshesList[2], glm::vec4(0.1f, 0.9f, 0.1f, 1.0f));
    auto n1 = new mgl::SceneNode(p1, Shaders);
    n1->transform = getModel(squareConfig[1].pos, squareConfig[1].rotX, squareConfig[1].rotY, squareConfig[1].rotZ, 1.0f);
    tableNode->addChild(n1);
    nodes.push_back(n1);

    // Peça 2 - Triangle 4
    TangramPiece* p2 = new TangramPiece(MeshesList[3], glm::vec4(0.1f, 0.1f, 0.9f, 1.0f));
    auto n2 = new mgl::SceneNode(p2, Shaders);
    n2->transform = getModel(squareConfig[2].pos, squareConfig[2].rotX, squareConfig[2].rotY, squareConfig[2].rotZ, 1.0f);
    tableNode->addChild(n2);
    nodes.push_back(n2);

    // Peça 3 - Triangle 6
    TangramPiece* p3 = new TangramPiece(MeshesList[4], glm::vec4(0.9f, 0.9f, 0.1f, 1.0f));
    auto n3 = new mgl::SceneNode(p3, Shaders);
    n3->transform = getModel(squareConfig[3].pos, squareConfig[3].rotX, squareConfig[3].rotY, squareConfig[3].rotZ, 1.0f);
    tableNode->addChild(n3);
    nodes.push_back(n3);

    // Peça 4 - Triangle 7
    TangramPiece* p4 = new TangramPiece(MeshesList[5], glm::vec4(0.9f, 0.1f, 0.9f, 1.0f));
    auto n4 = new mgl::SceneNode(p4, Shaders);
    n4->transform = getModel(squareConfig[4].pos, squareConfig[4].rotX, squareConfig[4].rotY, squareConfig[4].rotZ, 1.0f);
    tableNode->addChild(n4);
    nodes.push_back(n4);

    // Peça 5 - Paralelogram
    TangramPiece* p5 = new TangramPiece(MeshesList[6], glm::vec4(0.1f, 0.9f, 0.9f, 1.0f));
    auto n5 = new mgl::SceneNode(p5, Shaders);
    n5->transform = getModel(squareConfig[5].pos, squareConfig[5].rotX, squareConfig[5].rotY, squareConfig[5].rotZ, 1.0f);
    tableNode->addChild(n5);
    nodes.push_back(n5);

    // Peça 7 - Quadrado
    TangramPiece* p7 = new TangramPiece(MeshesList[7], glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    auto n7 = new mgl::SceneNode(p7, Shaders);
    n7->transform = getModel(squareConfig[6].pos, squareConfig[6].rotX, squareConfig[6].rotY, squareConfig[6].rotZ, 1.0f);
    tableNode->addChild(n7);
    nodes.push_back(n7);
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

    // --- SETUP CAM 2 ---
    glm::vec3 eye2(-5.0f, 2.0f, -5.0f);
    glm::vec3 up2(0.0f, 1.0f, 0.0f);

    cam2.radius = glm::length(eye2);
    cam2.rotation = glm::quatLookAt(glm::normalize(-eye2), up2);
    cam2.viewMatrix = glm::lookAt(eye2, target, up2);
    cam2.isOrtho = false;

    calculateProjection(cam1, 800, 600);
    calculateProjection(cam2, 800, 600);

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

void MyApp::updatePieceTransforms(float dt) {
    t += animDirection * animSpeed * dt;
    t = glm::clamp(t, 0.0f, 1.0f);

    for (int i = 0; i < nodes.size(); i++) {

        glm::vec3 pos = glm::mix(squareConfig[i].pos,
            sealConfig[i].pos, t);

        float rotX = glm::mix(squareConfig[i].rotX, sealConfig[i].rotX, t);
        float rotY = glm::mix(squareConfig[i].rotY, sealConfig[i].rotY, t);
        float rotZ = glm::mix(squareConfig[i].rotZ, sealConfig[i].rotZ, t);

        nodes[i]->transform = getModel(pos, rotX, rotY, rotZ, 1.0f);
    }
}


////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
  createMeshes();
  createShaderPrograms();
  createCamera();
  createSceneGraph();
}

void MyApp::windowSizeCallback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
    calculateProjection(cam1, width, height);
    calculateProjection(cam2, width, height);
    if (Camera && activeCam) {
        Camera->setProjectionMatrix(activeCam->projectionMatrix);
    }
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
    updatePieceTransforms(elapsed);
    drawScene(); 
}

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) animDirection = +1;
        if (action == GLFW_RELEASE) animDirection = 0;
    }

    if (key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) animDirection = -1;
        if (action == GLFW_RELEASE) animDirection = 0;
    }
    if (action != GLFW_PRESS) return;
    switch (key) {

    case GLFW_KEY_C:
        activeCam = (activeCam == &cam1) ? &cam2 : &cam1;

        Camera->setViewMatrix(activeCam->viewMatrix);
        Camera->setProjectionMatrix(activeCam->projectionMatrix);

        std::cout << "Camera: " << (activeCam == &cam1 ? "1" : "2") << std::endl;
        break;

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

    else if (leftMousePressed) {
        glm::vec3 camRight = activeCam->rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 camFront = activeCam->rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 translation = (camRight * (float)distanceX * panSpeed) +
            (camFront * (float)-distanceY * panSpeed);

        tableNode->transform = glm::translate(glm::mat4(1.0f), translation) * tableNode->transform;
    }
    lastCameraPosX = xpos;
    lastCameraPosY = ypos;
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(800, 600, "Create Pickagram 3D", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
