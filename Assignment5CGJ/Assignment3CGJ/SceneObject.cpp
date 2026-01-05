#include "SceneObject.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

SceneObject::SceneObject(mgl::Mesh* mesh, glm::vec4 color, GLuint textureID, int materialType)
    : mesh_(mesh), color_(color), textureID_(textureID), materialType_(materialType) {
}

void SceneObject::setColor(const glm::vec4& color) {
    color_ = color;
}


void SceneObject::draw() {
    if (!mesh_) return;


    GLint programID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programID);


    GLint locColor = glGetUniformLocation(programID, "uColor");
    if (locColor != -1) glUniform4fv(locColor, 1, glm::value_ptr(color_));

    GLint locMat = glGetUniformLocation(programID, "uMaterialType");
    if (locMat != -1) glUniform1i(locMat, materialType_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID_);

    GLint locTex = glGetUniformLocation(programID, "uNoiseTexture");
    if (locTex != -1) glUniform1i(locTex, 0);

    // 5. Desenhar a malha
    mesh_->draw();
}