#ifndef ROUGETSU_RENDERER_H
#define ROUGETSU_RENDERER_H

#include "../graph3d/sgd_types.h"

typedef void (*mesh_rendering)(SGDPROCUNITHEADER*,SGDPROCUNITHEADER*);
void HandleCamera(int width, int height);
void MeshType0x82(SGDPROCUNITHEADER *pVUVN, SGDPROCUNITHEADER *pPUHead);
void MeshType_iMT_2F(SGDPROCUNITHEADER *pVUVN, SGDPROCUNITHEADER *pPUHead);

#endif//ROUGETSU_RENDERER_H
