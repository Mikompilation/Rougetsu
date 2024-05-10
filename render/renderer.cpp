#include "renderer.h"
#include "Vu1Mem.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>

void MeshType0x82(SGDPROCUNITHEADER *pVUVN, SGDPROCUNITHEADER *pPUHead)
{
  std::vector<Vector3> v;
  
  SGDVUVNDESC *pVUVNDesc = (SGDVUVNDESC*)&pVUVN->VUVNDesc;
  SGDPROCUNITDATA *pVUVNData = (SGDPROCUNITDATA *) &pVUVN[1];
  
  for (int i = 0; i < pVUVNDesc->sNumVertex; i++)
  {
    auto vv = Vector3(pVUVNData->VUVNData_Preset.avt2[i].vVertex[0], 
                      pVUVNData->VUVNData_Preset.avt2[i].vVertex[1],
                      pVUVNData->VUVNData_Preset.avt2[i].vVertex[2]);
        
    auto m = (Matrix*)g_scratchpadLayout.Vu1Mem.Packed.Transform.matLocalWorld;
    
    auto mt = MatrixTranspose(*m);
        
    vv = Vector3Transform(vv, mt);
    v.push_back(vv);
  }
      
  DrawTriangleStrip3D(v.data() , pVUVNDesc->sNumVertex, DARKGRAY);  
}
