#include "renderer.h"
#include "Vu1Mem.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>

void HandleCamera(int width, int height)
{
  vertexOffset = 0;
  glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  unsigned int viewLoc = context->GetUniformLocation(shaderProgram, "view");
  context->UniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
  // Projection
  glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 1000.0f);
  unsigned int projectionLoc = context->GetUniformLocation(shaderProgram, "projection");
  context->UniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void MeshType0x82(SGDPROCUNITHEADER *pVUVN, SGDPROCUNITHEADER *pPUHead)
{
  SGDVUVNDESC *pVUVNDesc = (SGDVUVNDESC*)&pVUVN->VUVNDesc;
  SGDPROCUNITDATA *pVUVNData = (SGDPROCUNITDATA *) &pVUVN[1];
  SGDVUMESHPOINTNUM *pMeshInfo = (SGDVUMESHPOINTNUM *) &pPUHead[4];
  
  for (int i = 0; i < pPUHead->VUMeshDesc.ucNumMesh; i++)
  {    
    std::vector<GLfloat> v;
    
    for (int j = 0; j < pMeshInfo[i].uiPointNum; j++)
    {
      v.push_back(pVUVNData->VUVNData_Preset.avt2[vertexOffset].vVertex[0]);
      v.push_back(pVUVNData->VUVNData_Preset.avt2[vertexOffset].vVertex[1]);
      v.push_back(pVUVNData->VUVNData_Preset.avt2[vertexOffset].vVertex[2]);
      vertexOffset++;
    }
    
    if(pMeshInfo[i].uiPointNum == 0)
    {
      continue;
    }
  
    GLuint VAO, VBO;
  
    context->GenVertexArrays(1, &VAO);
    context->GenBuffers(1, &VBO);
  
    // Make the VAO the current Vertex Array Object by binding it
    context->BindVertexArray(VAO);
    
    // Bind the VBO specifying it's a GL_ARRAY_BUFFER
    context->BindBuffer(GL_ARRAY_BUFFER, VBO);
  
    // Introduce the vertices into the VBO
    context->BufferData(GL_ARRAY_BUFFER, v.size() * sizeof(GLfloat), v.data(), GL_STATIC_DRAW);

    // Configure the Vertex Attribute so that OpenGL knows how to read the VBO
    context->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  
    // Enable the Vertex Attribute so that OpenGL knows to use it
    context->EnableVertexAttribArray(0);

    // Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
    context->BindBuffer(GL_ARRAY_BUFFER, 0);
    context->BindVertexArray(0);
  
    // Tell OpenGL which Shader Program we want to use
    context->UseProgram(shaderProgram);
  
    // Bind the VAO so OpenGL knows to use it
    context->BindVertexArray(VAO);
  
    // Draw the triangle using the GL_TRIANGLE_STRIP primitive
    context->DrawArrays(GL_TRIANGLE_STRIP, 0, pMeshInfo[i].uiPointNum - 2);
  
    context->DeleteVertexArrays(1, &VAO);
    context->DeleteBuffers(1, &VBO);
  }
}
