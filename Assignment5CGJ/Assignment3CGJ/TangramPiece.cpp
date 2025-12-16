#include "TangramPiece.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

TangramPiece::TangramPiece(mgl::Mesh* mesh, const glm::vec4& color)
    : mesh_(mesh), color_(color) {
}

void TangramPiece::setColor(const glm::vec4& color) {
    color_ = color;
}

const glm::vec4& TangramPiece::getColor() const {
    return color_;
}

mgl::Mesh* TangramPiece::getMesh() const {
    return mesh_;
}

void TangramPiece::draw(mgl::ShaderProgram* shader) {
    if (!shader || !mesh_) return;

    if (shader->isUniform("uColor")) {
        GLint loc = shader->Uniforms["uColor"].index;
        glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(color_)));
    }

    mesh_->draw();
}

