#ifndef ROUGETSU_VU1MEM_H
#define ROUGETSU_VU1MEM_H

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "../graph3d/g3d_types.h"
#include "../sce/sce_types.h"
#include "glm/vec3.hpp"

inline GLFWwindow* window;
inline GRA3DSCRATCHPADLAYOUT g_scratchpadLayout;
inline GladGLContext* context;
inline GLuint shaderProgram;
inline glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
inline glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
inline glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

inline float deltaTime = 0.0f;
inline float lastFrame = 0.0f;

int InitializeVu1Memory();
void InitializeShaders();
void processInput(GLFWwindow* window);

#endif//ROUGETSU_VU1MEM_H
