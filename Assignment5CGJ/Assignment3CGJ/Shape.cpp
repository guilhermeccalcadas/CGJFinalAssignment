#include "Shape.h"
#include <mgl.hpp>

Shape::Shape(const std::vector<glm::vec4>& points,
    const std::vector<GLubyte> index, glm::vec4 color): indices(index), color(color) {

    vertices.reserve(points.size());
    for (auto& p : points) {
        Vertex v;
        v.XYZW[0] = p.x;
        v.XYZW[1] = p.y;
        v.XYZW[2] = p.z;
        v.XYZW[3] = p.w;

        v.RGBA[0] = color.r;
        v.RGBA[1] = color.g;
        v.RGBA[2] = color.b;
        v.RGBA[3] = color.a;

        vertices.push_back(v);
    }

    //createBuffers();
}

Shape::~Shape() {
    glBindVertexArray(VaoId);
    glDeleteVertexArrays(1, &VaoId);
    glDeleteBuffers(2, VboId);
    glDisableVertexAttribArray(POSITION);
    glDisableVertexAttribArray(COLOR);
    glBindVertexArray(0);
}

void Shape::createBuffers() {
    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);
    {
        glGenBuffers(2, VboId);

        glBindBuffer(GL_ARRAY_BUFFER, VboId[0]);
        {
            //Change back to sizeof(vertices), vertices if with subclasses we pass it as a list
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(POSITION);
            glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<GLvoid*>(0));

            glEnableVertexAttribArray(COLOR);
            glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<GLvoid*>(sizeof(vertices[0].XYZW)));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VboId[1]);
        {
            //Change back to sizeof(indices), indices if with subclasses we pass it as a list
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLubyte), indices.data(), GL_STATIC_DRAW);
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glDeleteBuffers(2, VboId); //VboId buffers necessary to alter the color
}

void Shape::changeColor(glm::vec4 newColor) {
    for (auto& v : vertices) {
        v.RGBA[0] = newColor.r;
        v.RGBA[1] = newColor.g;
        v.RGBA[2] = newColor.b;
        v.RGBA[3] = newColor.a;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VboId[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
}

glm::mat4 Shape::getModelMatrix(glm::vec3 pos, float rot, float scal) {
    glm::mat4 M = glm::translate(glm::mat4(1.0f), pos)
        * glm::rotate(glm::mat4(1.0f), glm::radians(rot), glm::vec3(0, 0, 1))
        * glm::scale(glm::mat4(1.0f), glm::vec3(scal, scal, 1.0f));
    return M;
}

void Shape::draw(GLint matrixUniform, GLint colorUniform, glm::vec3 pos, float rot, float scal) {
    glm::mat4 M = getModelMatrix(pos, rot, scal);

    glUniformMatrix4fv(matrixUniform, 1, GL_FALSE, glm::value_ptr(M));
    //glUniform4fv(colorUniform, 1, glm::value_ptr(color));

    glBindVertexArray(VaoId);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, 0);
    glBindVertexArray(0);
}

void Shape::createMesh(std::string mesh_file) {
    //Handles creating a mesh, but i dont know if its a single mesh per shape
    std::string mesh_fullname = mesh_dir + mesh_file;
    Mesh = new mgl::Mesh();
    Mesh->joinIdenticalVertices();
    Mesh->create(mesh_fullname);
}


