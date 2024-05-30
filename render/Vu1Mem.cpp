#include "Vu1Mem.h"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cstdlib>

// Vertex Shader source code
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
    "}\0";

//Fragment Shader source code
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n\0";

void GLAPIENTRY
MessageCallback( GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
          type, severity, message );
}

int InitializeVu1Memory()
{
  if (!glfwInit())
  {
    return -1;
  }
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  
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
  
  context->Enable              ( GL_DEBUG_OUTPUT );
  context->DebugMessageCallback( MessageCallback, 0 );
  
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
  
  // Tell OpenGL which Shader Program we want to use
  context->UseProgram(shaderProgram);
  
  //context->Enable(GL_DEPTH_TEST);
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
  
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
  {
    wireframe = !wireframe;
  }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f; // Change this value to your liking
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // Make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
}

