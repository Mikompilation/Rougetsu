#ifndef ROUGETSU_VU1MEM_H
#define ROUGETSU_VU1MEM_H

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "../graph3d/g3d_types.h"
#include "../sce/sce_types.h"

inline GLFWwindow* window;
inline GRA3DSCRATCHPADLAYOUT g_scratchpadLayout;
inline GladGLContext* context;
inline GLuint shaderProgram;

int InitializeVu1Memory();
void InitializeShaders();

#endif//ROUGETSU_VU1MEM_H
