#ifndef ROUGETSU_VU1MEM_H
#define ROUGETSU_VU1MEM_H

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "../graph3d/g3d_types.h"
#include "../sce/sce_types.h"
#include "glm/geometric.hpp"
#include "glm/vec3.hpp"

inline GLFWwindow* window;
inline GRA3DSCRATCHPADLAYOUT g_scratchpadLayout;
inline GladGLContext* context;
inline GLuint shaderProgram;

inline bool wireframe = false;

inline glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
inline glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
inline glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
inline glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
inline glm::vec3 cameraUpDirection = glm::normalize(glm::cross(cameraRight, cameraFront));

inline float deltaTime = 0.0f;
inline float lastFrame = 0.0f;

inline bool firstMouse = true;
inline float yaw = -90.0f; // Yaw is initialized to -90.0 degrees to look initially along the negative Z-axis
inline float pitch = 0.0f;
inline float lastX = 640 / 2.0;
inline float lastY = 480 / 2.0;
inline float fov = 90.0f;

int InitializeVu1Memory();
void InitializeShaders();
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

#endif//ROUGETSU_VU1MEM_H
