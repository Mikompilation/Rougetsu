#include "renderer.h"
#include "../graph3d/gra3dSGD.h"
#include "../utils.h"
#include "Vu1Mem.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void HandleCamera(int width, int height)
{
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
  SGDPROCUNITDATA *pVUVNData = (SGDPROCUNITDATA *) &pVUVN[1];
  SGDVUMESHPOINTNUM *pMeshInfo = (SGDVUMESHPOINTNUM *) &pPUHead[4];
  int vertexOffset = 0;
  
  for (int i = 0; i < pPUHead->VUMeshDesc.ucNumMesh; i++)
  {    
    if (pMeshInfo[i].uiPointNum == 0)
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
    context->BufferData(GL_ARRAY_BUFFER, pMeshInfo[i].uiPointNum * sizeof(SGDMESHVERTEXDATA_TYPE2), &pVUVNData->VUVNData_Preset.avt2[vertexOffset], GL_STATIC_DRAW);

    // Configure the Vertex Attribute so that OpenGL knows how to read the VBO
    context->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SGDMESHVERTEXDATA_TYPE2), nullptr);
  
    // Enable the Vertex Attribute so that OpenGL knows to use it
    context->EnableVertexAttribArray(0);

    // Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
    context->BindBuffer(GL_ARRAY_BUFFER, 0);
    context->BindVertexArray(0);
  
    // Bind the VAO so OpenGL knows to use it
    context->BindVertexArray(VAO);
  
    // Draw the triangle using the GL_TRIANGLE_STRIP primitive
    auto render_type = wireframe ? GL_LINE_STRIP : GL_TRIANGLE_STRIP;
    context->DrawArrays(render_type, 0, pMeshInfo[i].uiPointNum);
  
    context->DeleteVertexArrays(1, &VAO);
    context->DeleteBuffers(1, &VBO);
    
    vertexOffset += pMeshInfo[i].uiPointNum;
  }
}

void MeshType_iMT_2F(SGDPROCUNITHEADER *pVUVN, SGDPROCUNITHEADER *pPUHead)
{
  SGDPROCUNITDATA *pVUVNData = (SGDPROCUNITDATA *) &pVUVN[1];
  auto pProcData = (SGDPROCUNITDATA *) &pPUHead[1];
  SGDVUMESHPOINTNUM *pMeshInfo = (SGDVUMESHPOINTNUM *) &pPUHead[4];
  
  SGDVUMESHSTDATA *sgdMeshData = RelOffsetToPtr<SGDVUMESHSTDATA>(
      pVUVNData, (pVUVNData->VUMeshData_Preset.sOffsetToST - 1) * 4);
  
  auto pVMCD =
      (_SGDVUMESHCOLORDATA *) (&pPUHead->pNext
                               + pProcData->VUMeshData_Preset.sOffsetToPrim);
  
  int vertexOffset = 0;
  
  for (int i = 0; i < pPUHead->VUMeshDesc.ucNumMesh; i++)
  {
    pVMCD = (_SGDVUMESHCOLORDATA *) GetNextUnpackAddr((uint *) pVMCD);
    
    GLfloat* vertices =
        (GLfloat *) (pVUVNData->VUVNData_Preset.aui
                     + (vertexOffset + pVUVN->VUVNDesc.sNumNormal) * 3 + 10);
    
    GLuint VAO, VBO;
  
    context->GenVertexArrays(1, &VAO);
    context->GenBuffers(1, &VBO);
  
    // Make the VAO the current Vertex Array Object by binding it
    context->BindVertexArray(VAO);
    
    // Bind the VBO specifying it's a GL_ARRAY_BUFFER
    context->BindBuffer(GL_ARRAY_BUFFER, VBO);
  
    // Introduce the vertices into the VBO
    context->BufferData(GL_ARRAY_BUFFER,pVMCD->VifUnpack.NUM * sizeof(VECTOR3), vertices, GL_STATIC_DRAW);
    
    // Configure the Vertex Attribute so that OpenGL knows how to read the VBO
    context->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  
    // Enable the Vertex Attribute so that OpenGL knows to use it
    context->EnableVertexAttribArray(0);

    // Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
    context->BindBuffer(GL_ARRAY_BUFFER, 0);
    context->BindVertexArray(0);
  
    // Bind the VAO so OpenGL knows to use it
    context->BindVertexArray(VAO);
  
    // Draw the triangle using the GL_TRIANGLE_STRIP primitive
    context->DrawArrays(GL_LINE_STRIP, 0, pVMCD->VifUnpack.NUM);
  
    context->DeleteVertexArrays(1, &VAO);
    context->DeleteBuffers(1, &VBO);
    
    vertexOffset += pVMCD->VifUnpack.NUM;
    pVMCD = (_SGDVUMESHCOLORDATA *) &pVMCD->avColor[pVMCD->VifUnpack.NUM];
  }
}
