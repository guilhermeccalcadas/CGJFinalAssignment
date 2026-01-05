#include "SceneObject.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

SceneObject::SceneObject(mgl::Mesh* mesh, glm::vec4 color, GLuint textureID, int materialType)
    : mesh_(mesh), color_(color), textureID_(textureID), materialType_(materialType) {
}

void SceneObject::setColor(const glm::vec4& color) {
    color_ = color;
}

// [CORREÇÃO] Implementação do draw() sem argumentos
void SceneObject::draw() {
    if (!mesh_) return;

    // 1. Descobrir qual o shader que o SceneNode ativou antes de nos chamar
    GLint programID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programID);

    // 2. Enviar Cor
    GLint locColor = glGetUniformLocation(programID, "uColor");
    if (locColor != -1) glUniform3fv(locColor, 1, glm::value_ptr(glm::vec3(color_)));

    // 3. Enviar Tipo de Material
    GLint locMat = glGetUniformLocation(programID, "uMaterialType");
    if (locMat != -1) glUniform1i(locMat, materialType_);

    // 4. ATIVAR A TEXTURA
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID_);

    // Avisar o shader que a textura está na slot 0
    GLint locTex = glGetUniformLocation(programID, "uNoiseTexture");
    if (locTex != -1) glUniform1i(locTex, 0);

    // 5. Desenhar a malha
    mesh_->draw();
}