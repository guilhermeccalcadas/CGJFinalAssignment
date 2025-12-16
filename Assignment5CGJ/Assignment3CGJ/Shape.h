#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <mgl.hpp>



class Shape {

typedef struct {
    GLfloat XYZW[4];
    GLfloat RGBA[4];
} Vertex;

public:
    glm::vec4 color;
    //float rotation;
    //glm::vec2 position;
    //float scale;

    //Buffers
    GLuint VaoId, VboId[2];

    //Substitute later when we have sub classes?
    std::vector<Vertex> vertices;
    std::vector<GLubyte> indices;
    mgl::Mesh* Mesh = nullptr;

    virtual ~Shape();

    void changeColor(glm::vec4 newColor);

    void draw(GLint matrixUniform, GLint colorUniform, glm::vec3 pos, float rot, float scal);
    glm::mat4 getModelMatrix(glm::vec3 pos, float rot, float scal);

    void createMesh(std::string mesh_file);

protected:
    const GLuint POSITION = 0, COLOR = 1;
    const std::string mesh_dir = "./assets/models/";

    Shape(const std::vector<glm::vec4>& points, const std::vector<GLubyte> index, glm::vec4 color);

    void createBuffers();
};
