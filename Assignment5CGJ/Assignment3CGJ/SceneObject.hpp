#pragma once

#include "mglMesh.hpp"
#include "mglShader.hpp"
#include <glm/glm.hpp>

class SceneObject : public mgl::IDrawable {
public:
    SceneObject(mgl::Mesh* mesh, glm::vec4 color, GLuint textureID, int materialType);

    void setColor(const glm::vec4& color);

    void draw() override;

private:
    mgl::Mesh* mesh_;
    glm::vec4 color_;
    GLuint textureID_;
    int materialType_;
};