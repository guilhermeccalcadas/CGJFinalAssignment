#pragma once

#include "mglMesh.hpp"
#include "mglShader.hpp"

#include <glm/glm.hpp>

class TangramPiece {
public:
    TangramPiece(mgl::Mesh* mesh, const glm::vec4& color);

    void setColor(const glm::vec4& color);
    const glm::vec4& getColor() const;

    mgl::Mesh* getMesh() const;

    // Desenha a peça usando o shader ativo fornecido pelo SceneNode
    void draw(mgl::ShaderProgram* shader);

private:
    mgl::Mesh* mesh_;
    glm::vec4 color_;
};
