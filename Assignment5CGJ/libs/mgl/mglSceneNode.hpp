#ifndef MGL_SCENENODE_HPP
#define MGL_SCENENODE_HPP

#include <vector>
#include <glm/mat4x4.hpp>

#include "mglShader.hpp"
#include "mglScenegraph.hpp"
#include "TangramPiece.hpp"

namespace mgl {

    class SceneNode : public IDrawable {
    public:
        SceneNode();
        SceneNode(TangramPiece* piece, ShaderProgram* shader);

        void setPiece(TangramPiece* piece);
        void setShader(ShaderProgram* shader);

        void addChild(SceneNode* child);
        void removeChild(SceneNode* child);

        void draw(void) override;
        void setTransform(const glm::mat4& t);
        const glm::mat4& getTransform() const;

    public:
        glm::mat4 transform;


    private:
        void draw(const glm::mat4& parentModel, ShaderProgram* inheritedShader);

        TangramPiece* piece_;
        ShaderProgram* shader_;
        std::vector<SceneNode*> children_;
    };

} // namespace mgl

#endif /* MGL_SCENENODE_HPP */
