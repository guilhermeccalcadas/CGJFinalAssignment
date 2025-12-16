////////////////////////////////////////////////////////////////////////////////
//
// OpenGL Error Handling
//
// Copyright (c)2022-25 by Carlos Martinho
//
////////////////////////////////////////////////////////////////////////////////

#include "./mglError.hpp"

#include <GL/glew.h>

#include <iostream>

////////////////////////////////////////////////////// DEBUG OUTPUT (OPENGL 4.3)

const std::string errorSource(GLenum source) {
  switch (source) {
  case GL_DEBUG_SOURCE_API:
    return "API";
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    return "window system";
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    return "shader compiler";
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    return "third party";
  case GL_DEBUG_SOURCE_APPLICATION:
    return "application";
  case GL_DEBUG_SOURCE_OTHER:
    return "other";
  default:
    exit(EXIT_FAILURE);
  }
}

const std::string errorType(GLenum type) {
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    return "error";
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    return "deprecated behavior";
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    return "undefined behavior";
  case GL_DEBUG_TYPE_PORTABILITY:
    return "portability issue";
  case GL_DEBUG_TYPE_PERFORMANCE:
    return "performance issue";
  case GL_DEBUG_TYPE_MARKER:
    return "stream annotation";
  case GL_DEBUG_TYPE_PUSH_GROUP:
    return "push group";
  case GL_DEBUG_TYPE_POP_GROUP:
    return "pop group";
  case GL_DEBUG_TYPE_OTHER_ARB:
    return "other";
  default:
    exit(EXIT_FAILURE);
  }
}

const std::string errorSeverity(GLenum severity) {
  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    return "high";
  case GL_DEBUG_SEVERITY_MEDIUM:
    return "medium";
  case GL_DEBUG_SEVERITY_LOW:
    return "low";
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    return "notification";
  default:
    exit(EXIT_FAILURE);
  }
}

void error(GLenum source, GLenum type, GLuint id, GLenum severity,
           GLsizei length, const GLchar *message, const void *userParam) {

  if (severity == GL_DEBUG_SEVERITY_LOW ||
      severity == GL_DEBUG_SEVERITY_MEDIUM) {
    std::cerr << "GL WARNING:" << std::endl;
  } else {
    std::cerr << "GL ERROR:" << std::endl;
  }
  std::cerr << "  source:     " << errorSource(source) << std::endl;
  std::cerr << "  type:       " << errorType(type) << std::endl;
  std::cerr << "  severity:   " << errorSeverity(severity) << std::endl;
  std::cerr << "  debug call: " << std::endl
            << message << std::endl
            << std::endl;
  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    exit(EXIT_FAILURE);
  }
}

void setupDebugOutput() {
  int context_flags = 0;
  glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
  if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    std::cout << "Debug context created." << std::endl;
  }
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(error, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                        GL_TRUE);
  // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
  //                       GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
  //                       GL_FALSE);
  // params: source, type, severity, count, ids, enabled
}

////////////////////////////////////////////////////////////////////////////////
