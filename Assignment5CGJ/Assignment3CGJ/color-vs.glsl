#version 330 core

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
out vec4 exColor;

uniform mat4 ModelMatrix;

uniform Camera {
   mat4 ViewMatrix;
   mat4 ProjectionMatrix;
};

void main(void) {
  gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * inPosition;
  exColor = inColor;
}
