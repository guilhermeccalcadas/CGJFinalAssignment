#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "mglShader.hpp"
#include "mglMesh.hpp"

namespace mgl {

    class SceneNode {
    public:
        SceneNode();
        SceneNode(IDrawable* piece, ShaderProgram* shader = nullptr);

        void setPiece(IDrawable* piece);
        void setShader(ShaderProgram* shader);
        void setTransform(const glm::mat4& t);
        const glm::mat4& getTransform() const;

        void addChild(SceneNode* child);
        void removeChild(SceneNode* child);

        void draw();
        void draw(const glm::mat4& parentModel, ShaderProgram* inheritedShader);
        void setPickID(int id) { pickID_ = id; }
        glm::mat4 transform;

    private:
        int pickID_ = 0;
        IDrawable* piece_;
        ShaderProgram* shader_;
        std::vector<SceneNode*> children_;
    };
}