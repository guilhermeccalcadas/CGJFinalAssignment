#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <mgl.hpp>
#include "Shape.h"

class Triangle : public Shape {
public:
	Triangle(const glm::vec4 points[3], const GLubyte index[3], glm::vec4 color);
};