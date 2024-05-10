#include "renderer.h"
#include "Vu1Mem.h"
#include <vector>

void MeshType0x82(SGDPROCUNITHEADER *pVUVN, SGDPROCUNITHEADER *pPUHead)
{
  std::vector<GLfloat> v;
  
  SGDVUVNDESC *pVUVNDesc = (SGDVUVNDESC*)&pVUVN->VUVNDesc;
  SGDPROCUNITDATA *pVUVNData = (SGDPROCUNITDATA *) &pVUVN[1];
  
  for (int i = 0; i < pVUVNDesc->sNumVertex; i++)
  {
    v.push_back(pVUVNData->VUVNData_Preset.avt2[i].vVertex[0]);
    v.push_back(pVUVNData->VUVNData_Preset.avt2[i].vVertex[1]);
    v.push_back(pVUVNData->VUVNData_Preset.avt2[i].vVertex[2]);
  }
  
  GLuint VAO, VBO;
  
  context->GenVertexArrays(1, &VAO);
  context->GenBuffers(1, &VBO);
  
  // Make the VAO the current Vertex Array Object by binding it
  context->BindVertexArray(VAO);

  // Bind the VBO specifying it's a GL_ARRAY_BUFFER
  context->BindBuffer(GL_ARRAY_BUFFER, VBO);
  
  // Introduce the vertices into the VBO
  context->BufferData(GL_ARRAY_BUFFER, pVUVNDesc->sNumVertex * sizeof(GLfloat) * 3, v.data(), GL_STATIC_DRAW);

  // Configure the Vertex Attribute so that OpenGL knows how to read the VBO
  context->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  
  // Enable the Vertex Attribute so that OpenGL knows to use it
  context->EnableVertexAttribArray(0);

  // Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
  context->BindBuffer(GL_ARRAY_BUFFER, 0);
  context->BindVertexArray(0);
  
  // Tell OpenGL which Shader Program we want to use
  context->UseProgram(shaderProgram);
  
  // Bind the VAO so OpenGL knows to use it
  context->BindVertexArray(VAO);
  
  // Draw the triangle using the GL_TRIANGLE_FAN primitive
  context->DrawArrays(GL_TRIANGLE_FAN, 0, pVUVNDesc->sNumVertex / 3);
  
  context->DeleteVertexArrays(1, &VAO);
  context->DeleteBuffers(1, &VBO);
}
