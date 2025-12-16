#include "mglSceneNode.hpp"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace mgl {

    SceneNode::SceneNode()
        : piece_(nullptr),
        shader_(nullptr),
        transform(glm::mat4(1.0f)) {
    }

    SceneNode::SceneNode(TangramPiece* piece, ShaderProgram* shader)
        : piece_(piece),
        shader_(shader),
        transform(glm::mat4(1.0f)) {
    }

    void SceneNode::setPiece(TangramPiece* piece) {
        piece_ = piece;
    }

    void SceneNode::setShader(ShaderProgram* shader) {
        shader_ = shader;
    }

    void SceneNode::setTransform(const glm::mat4& t) {
        transform = t;
    }

    const glm::mat4& SceneNode::getTransform() const {
        return transform;
    }

    void SceneNode::addChild(SceneNode* child) {
        if (child) children_.push_back(child);
    }

    void SceneNode::removeChild(SceneNode* child) {
        auto it = std::find(children_.begin(), children_.end(), child);
        if (it != children_.end())
            children_.erase(it);
    }

    void SceneNode::draw() {
        draw(glm::mat4(1.0f), nullptr);
    }

    void SceneNode::draw(const glm::mat4& parentModel, ShaderProgram* inheritedShader) {
        ShaderProgram* activeShader = shader_ ? shader_ : inheritedShader;
        bool boundHere = false;

        if (activeShader && activeShader != inheritedShader) {
            activeShader->bind();
            boundHere = true;
        }

        glm::mat4 globalMatrix = parentModel * transform;

        if (activeShader && activeShader->isUniform("ModelMatrix")) {
            GLint loc = activeShader->Uniforms["ModelMatrix"].index;
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(globalMatrix));
        }

        if (piece_) {
            piece_->draw(activeShader);
        }

        for (auto child : children_) {
            child->draw(globalMatrix, activeShader);
        }

        if (boundHere) {
            activeShader->unbind();
        }
    }

}
