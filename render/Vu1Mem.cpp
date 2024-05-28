#include "Vu1Mem.h"
#include "glm/geometric.hpp"
#include <cstdlib>

// Vertex Shader source code
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * transform * vec4(aPos, 1.0);\n"
    "}\0";

//Fragment Shader source code
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n\0";

int InitializeVu1Memory()
{
  if (!glfwInit())
  {
    return -1;
  }
  
  window = glfwCreateWindow(640, 480, "Rougetsu", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }
  
  /* Make the window's context current */
  glfwMakeContextCurrent(window);
  context = (GladGLContext*) calloc(1, sizeof(GladGLContext));
  int version = gladLoadGLContext(context, glfwGetProcAddress);
  
  return 0;
}

void InitializeShaders()
{
  GLuint vertexShader = context->CreateShader(GL_VERTEX_SHADER);
  context->ShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  
  // Compile the Vertex Shader into machine code
  context->CompileShader(vertexShader);
  
  // Create Fragment Shader Object and get its reference
  GLuint fragmentShader = context->CreateShader(GL_FRAGMENT_SHADER);
  
  // Attach Fragment Shader source to the Fragment Shader Object
  context->ShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  
  // Compile the Vertex Shader into machine code
  context->CompileShader(fragmentShader);
  
  // Create Shader Program Object and get its reference
  shaderProgram = context->CreateProgram();
  
  // Attach the Vertex and Fragment Shaders to the Shader Program
  context->AttachShader(shaderProgram, vertexShader);
  context->AttachShader(shaderProgram, fragmentShader);
  
  // Wrap-up/Link all the shaders together into the Shader Program
  context->LinkProgram(shaderProgram);

  // Delete the now useless Vertex and Fragment Shader objects
  context->DeleteShader(vertexShader);
  context->DeleteShader(fragmentShader);
}

void processInput(GLFWwindow* window) 
{
  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
  
  float cameraSpeed = 25.0f * deltaTime;
  
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}