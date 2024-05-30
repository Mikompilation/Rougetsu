#include "gra3dSGD.h"
#include "../render/Vu1Mem.h"
#include "../render/renderer.h"
#include "../sce/libvu0.h"
#include "g3dCore.h"
#include "g3dDebug.h"
#include "g3dUtil.h"
#include "glm/gtc/type_ptr.hpp"
#include "gra3d.h"
#include "gra3dDma.h"
#include "gra3dSGDData.h"
#include "gra3dShadow.h"
#include <cstdio>
#include <vector>

static float (*s_pGlobalVertexBuffer)[4];
static float (*s_pGlobalNormalBuffer)[4];
static int s_iGlobalBufferSize = 64000;
SGDPROCUNITHEADER *save_tri2_pointer;
SGDPROCUNITHEADER *save_bw_pointer;
static SGDPROCUNITHEADER *s_ppuhVUVN;
static SGDCOORDINATE *s_pCoordBase;
static SGDFILEHEADER *s_pSGDTop;

CoordCache ccahe;
int edge_check;
SGDPROCUNITHEADER *previous_tri2_prim;

void gra3dsgdSetData(SGDFILEHEADER *pSGDTop)
{
  g3ddbg_WARNING_RETURN(pSGDTop, "pSGDTop is NULL");
  g3ddbg_WARNING_RETURN(pSGDTop->uiVersionId == SGD_VALID_VERSIONID,
                        "Invalid SGD File");
  s_pSGDTop = pSGDTop;
}

void gra3dsgdSetCoordinate(const SGDCOORDINATE *pCU, int iIndex)
{
  g3ddbg_ASSERT(pCU, "");

  if (iIndex == -1)
  {
    s_pCoordBase = (SGDCOORDINATE *) pCU;
    return;
  }

  s_pCoordBase[iIndex] = *pCU;
}

void _gra3dDrawSGD(SGDFILEHEADER *pSGDTop, SGDRENDERTYPE type,
                   SGDCOORDINATE *pCoord, int pnum)
{
  /// pnum seems to only have 3 values: -1, 0, 1
  /// INVALID_SGD_OBJECTID, ???, ???
  g3ddbg_WARNING_RETURN(pSGDTop, "pSGDTop is NULL");

  bool bPreset = sgdIsPresetData(pSGDTop);

  g3ddbg_WARNING_RETURN(pSGDTop->uiVersionId == SGD_VALID_VERSIONID,
                        "Invalid SGD File");

  g3ddbg_WARNING_RETURN(!((unsigned int) pCoord & 0x0f),
                        "SGDFILEHEADER::pCoord is illegal");

  gra3dsgdSetData(pSGDTop);

  if (pCoord)
  {
    gra3dsgdSetCoordinate(pCoord, -1);
  }
  else
  {
    gra3dsgdSetCoordinate(pSGDTop->pCoord, -1);
  }

  if (type == SRT_REALTIME)
  {
    sgdResetMaterialCache(pSGDTop);
    gra3dsgdSetupVu1();
  }
  else if (type != SRT_PRELIGHTING && type != SRT_CLEARPRELIGHTING
           && type == SRT_MAPSHADOW)
  {
    CoordCache CC = _GetCoordCache();
    CC.cache_on = -1;
    _SetCoordCache(CC);
  }

  g3ddbg_ASSERT(!(pnum != INVALID_SGD_OBJECTID && pnum < 0), "");

  g3ddbg_ASSERT(pnum <= gra3dsgdGetNumBlock() - 1,
                "pnum:%d, gra3dsgdGetNumBlock()-1:%d", pnum,
                gra3dsgdGetNumBlock() - 1);

  SGDPROCUNITHEADER **pk = pSGDTop->apProcUnitHead;

  g3ddbg_ASSERT(pk, "");

  if (bPreset)
  {
    if (type == SRT_REALTIME)
    {
      if (pnum < 0)
      {
        save_bw_pointer = nullptr;
        save_tri2_pointer = nullptr;

        SgSortPreProcessP(*pk);
        int iNumBlock = gra3dsgdGetNumBlock();

        for (int i = 1; i < iNumBlock; i++)
        {
          gra3dsgdDrawPresetDataObject(pk[i]);
        }
      }
      else if (pnum == 0)
      {
        save_bw_pointer = (SGDPROCUNITHEADER *) 0xffffffff;
        save_tri2_pointer = (SGDPROCUNITHEADER *) 0xffffffff;
        SgSortPreProcessP(*pk);
      }
      else
      {
        save_bw_pointer = nullptr;
        save_tri2_pointer = nullptr;

        gra3dsgdDrawPresetDataObject(pk[pnum]);
      }
      return;
    }
  }

  if (type == SRT_REALTIME)
  {
    /// Loops through LIGHT_POINT, 1 Object can only have 3 light points
    for (int i = 0; i < 3; i++)
    {
      int b = gra3dIsLightEnable(i + GRA3D_START_LIGHT_POINT);
      g3dLightEnable(i + GRA3D_START_LIGHT_POINT, b);

      if (b)
      {
        g3dSetLight(i + GRA3D_START_LIGHT_POINT,
                    gra3dGetLightRef(i + GRA3D_START_LIGHT_POINT));
      }
    }

    for (int i = 0; i < 3; i++)
    {
      int b = gra3dIsLightEnable(i + GRA3D_START_LIGHT_SPOT);
      g3dLightEnable(i + LID_POINT_3, b);

      if (b)
      {
        g3dSetLight(i + LID_POINT_3,
                    gra3dGetLightRef(i + GRA3D_START_LIGHT_SPOT));
      }
    }

    gra3dDmaLoadVu1MicroProgram(nullptr, 0);

    if (pnum < 0)
    {
      int i;
      
      /// Handles the textures
      SgSortPreProcess((u_int *) *pk);
      int iNumBlock = gra3dsgdGetNumBlock();

      for (i = 1; i < iNumBlock - 1; i++)
      {
        SgSortUnitPrim(pk[i]);
      }

      if (!pk[i])
      {
        return;
      }

      SgSortUnitPrimPost(pk[i]);
      return;
    }
    else if (pnum == 0)
    {
      SgSortPreProcess((u_int *) *pk);
      return;
    }
    else if (pnum == gra3dsgdGetNumBlock() - 1)
    {
      SgSortUnitPrimPost(pk[pnum]);
      return;
    }
    else
    {
      SgSortUnitPrim(pk[pnum]);
      return;
    }
  }
  else if (type == SRT_PRELIGHTING)
  {
    g3ddbg_ASSERT(bPreset, "");

    if (pnum < 0)
    {
      int iNumBlock = gra3dsgdGetNumBlock();

      for (int i = 1; i < iNumBlock; i++)
      {
        SgPreRenderPrim(pk[i]);
      }
    }
    else if (pnum != 0)
    {
      SgPreRenderPrim(pk[pnum]);
    }
  }
  else if (type == SRT_CLEARPRELIGHTING)
  {
    if (pnum < 0)
    {
      int iNumBlock = gra3dsgdGetNumBlock();

      for (int i = 1; i < iNumBlock; i++)
      {
        SgClearPreRenderPrim(pk[i]);
      }
    }
    else if (pnum != 0)
    {
      SgClearPreRenderPrim(pk[pnum]);
    }
  }
  else if (type == SRT_MAPSHADOW)
  {
    g3ddbg_ASSERT(gra3dshadowGetAssignGroup() < 0,
                  "グループパケットが必要！？意味不明");
    g3ddbg_ASSERT(pnum == -1, "see old source");

    int iNumBlock = gra3dsgdGetNumBlock();

    for (int i = 1; i < iNumBlock - 1; i++)
    {
      if (gra3dsgdGetCoordinate(i)->bInViewvolume)
      {
        AssignShadowPrim(pk[i]);
      }
    }
  }
  else
  {
    g3ddbg_ASSERT(false, "");
  }
}

void SgSortUnitPrim(SGDPROCUNITHEADER *pPUHead)
{
  while (pPUHead)
  {
    g3ddbg_ASSERT(!((int) pPUHead & 0xf), "memory illegal access occured");

    switch (pPUHead->iCategory)
    {
      case VUVN:
        _SetVUVNPRIM(pPUHead);
        break;
      case MESH:
        SetVUMeshData(pPUHead);
        break;
      case MATERIAL:
        SetMaterialDataVU(pPUHead);
        break;
      case BOUNDING_BOX:
        if (!CheckBoundingBox(pPUHead))
        {
          return;
        }
        _SetCoordData(pPUHead);

        break;
      case GS_IMAGE:
        GsImageProcess(pPUHead);
        break;
      case COORDINATE:
        break;
      default:
        break;
    }

    pPUHead = pPUHead->pNext;
  }
}

static void SetVUMeshDataPost(SGDPROCUNITHEADER *pPUHead)
{
  SGDVUMESHDESC &rVUMeshDesc = (SGDVUMESHDESC &) pPUHead->VUMeshDesc;
  SGDVUMESHDATA *pVUMeshData = (SGDVUMESHDATA *) &pPUHead[1];

  printf("rVUMeshDesc.ucMeshType: %d\n", rVUMeshDesc.ucMeshType);
  
  switch (rVUMeshDesc.ucMeshType & 0x53)
  {
    case 0x0:
      g3dDmaAddPacket(pVUMeshData->qwVif1Code, rVUMeshDesc.iTagSize);
      SetVUVNDataPost(s_ppuhVUVN);
      gra3dCallMicroSubroutine2(nullptr);
      return;
      /// Textured only
    case 0x2:
      g3dDmaAddPacket(pVUMeshData->qwVif1Code, rVUMeshDesc.iTagSize);
      SetVUVNDataPost(s_ppuhVUVN);
      gra3dCallMicroSubroutine2(nullptr);
      return;
    case 0x42:
      g3dDmaAddPacket(pVUMeshData->qwVif1Code, rVUMeshDesc.iTagSize);
      gra3dCallMicroSubroutine2(nullptr);
      return;
    default:
      g3ddbg_ASSERT(false, "来てはいけないところに来ています。(mtype:%d)",
                    rVUMeshDesc.ucMeshType);
      return;
  }
}

void SgSortPreProcessP(SGDPROCUNITHEADER *pPUHead)
{
  ASSERT_RETURN(!pPUHead);

  while (pPUHead)
  {
    switch (pPUHead->iCategory)
    {
      case GS_IMAGE:
        GsImageProcess(pPUHead);
        break;
      case TRI2:
        if (save_tri2_pointer == (SGDPROCUNITHEADER *) 0xffffffff)
        {
          LoadTRI2Files(pPUHead);
          save_tri2_pointer = nullptr;
        }
        else
        {
          save_tri2_pointer = pPUHead;
        }

        break;
      case MonotoneTRI2:
        if (gra3dIsMonotoneDrawEnable())
        {
          if (save_bw_pointer == (SGDPROCUNITHEADER *) 0xffffffff)
          {
            LoadTRI2Files(pPUHead);
            save_bw_pointer = nullptr;
          }
          else
          {
            save_bw_pointer = pPUHead;
          }
        }
        break;
      case INVALID:
        g3ddbg_ASSERT(false, "");
        break;
      case StackTRI2:
        g3ddbg_ASSERT(false, "");
        break;
    }

    pPUHead = pPUHead->pNext;
    g3ddbg_ASSERT_WARNING((int) pPUHead != (int) 0xffffffff,
                          "sgd has been broken...");
  }
}

void sgdRemap(SGDFILEHEADER *pSGDHead)
{
  SGDVECTORINFO *pVectorInfo;

  g3ddbg_WARNING_RETURN(pSGDHead, "");

  g3ddbg_WARNING_RETURN(pSGDHead->uiVersionId == SGD_VALID_VERSIONID, "");

  ASSERT_RETURN(pSGDHead->ucMapFlag)

  pSGDHead->ucMapFlag = 1;

  void *p0 = pSGDHead->pCoord;
  if (p0 < (SGDCOORDINATE *) 0x30000000 && ((int) p0 != 0))
  {
    /* inlined from g3dUtil.h */
    pSGDHead->pCoord = (SGDCOORDINATE *) GetPtrOffset((int) pSGDHead, (int) p0);
    /* end of inlined section */
  }

  void *p1 = pSGDHead->pMaterial;
  if ((u_int) p1 < 0x30000000 && p1)
  {
    pSGDHead->pMaterial =
        (SGDMATERIAL *) GetPtrOffset((int) pSGDHead, (int) p1);
    g3ddbg_ASSERT(!((u_int) pSGDHead->pMaterial & 0xf),
                  "データのアラインがおかしいよ。")
  }

  SGDCOORDINATE *pCoord = pSGDHead->pCoord;
  if (pCoord)
  {
    for (unsigned int i = 0; i < pSGDHead->uiNumBlock - 1; i++)
    {
      SGDCOORDINATE &rCoord = pCoord[i];
      int j = (int) rCoord.pParent;
      g3ddbg_ASSERT((int) j <= (int) pSGDHead->uiNumBlock - 2,
                    "こーでねーとの親子関係がおかしい風\nj : %d, "
                    "pSGDHead->uiNumBlock : %d",
                    j, pSGDHead->uiNumBlock)

          if (j < 0)
      {
        rCoord.pParent = (SGDCOORDINATE *) nullptr;
      }
      else if (rCoord.pParent < pCoord)
      {
        rCoord.pParent = &pCoord[j];
      }
      g3ddbg_ASSERT(!((int) rCoord.pParent & 0xf),
                    "データのアラインがおかしいよ。")
    }
  }

  if (pSGDHead->pVectorInfo)
  {
    pVectorInfo =
        (SGDVECTORINFO *) ((int) pSGDHead->pVectorInfo + (int) pSGDHead);
    pSGDHead->pVectorInfo = pVectorInfo;

    g3ddbg_ASSERT(pVectorInfo, "pVectorInfo is NULL")

        for (unsigned int i = 0; i < pVectorInfo->uiNumAddress; i++)
    {
      SGDVECTORADDRESS &rVA = pVectorInfo->aAddress[i];
      if (rVA.uiSize != 0 && rVA.pvVertex)
      {
        /* inlined from g3dUtil.h */
        rVA.pvVertex =
            (sceVu0FVECTOR *) GetPtrOffset((int) pSGDHead, (int) rVA.pvVertex);
        /* end of inlined section */
      }

      if (1 < rVA.uiSize && rVA.pvNormal)
      {
        /* inlined from g3dUtil.h */
        rVA.pvNormal =
            (sceVu0FVECTOR *) GetPtrOffset((int) pSGDHead, (int) rVA.pvNormal);
        /* end of inlined section */
      }

      if (2 < rVA.uiSize && rVA.pVertexList)
      {
        /* inlined from g3dUtil.h */
        rVA.pVertexList =
            (_VERTEXLIST *) GetPtrOffset((int) pSGDHead, (int) rVA.pVertexList);
        /* end of inlined section */
      }
    }

    if (pVectorInfo->aAddress[SVA_UNIQUE].pVertexList
        && pVectorInfo->uiNumAddress == 4)
    {
      if (!_GetGlobalBufferSize())
      {
        pVectorInfo->aAddress[SVA_UNIQUE].pVertexList = nullptr;
        pVectorInfo->aAddress[SVA_COMMON].pVertexList = nullptr;
        pVectorInfo->aAddress[SVA_WEIGHTED].pVertexList = nullptr;
      }
      else
      {
        SGDVECTORADDRESS &rVA = pVectorInfo->aAddress[SVA_WEIGHTED];
        if (!rVA.pvVertex && !rVA.pvNormal)
        {
          rVA.pVertexList = nullptr;
        }
      }

      _VERTEXLIST *p_Var5 = pVectorInfo->aAddress[SVA_WEIGHTED].pVertexList;
      pVectorInfo->aAddress[SVA_UNIQUE].pVertexList = nullptr;
      if (pVectorInfo->aAddress[SVA_WEIGHTED].pVertexList)
      {
        MappingVertexList(pVectorInfo->aAddress[SVA_WEIGHTED].pVertexList,
                          pVectorInfo);
        MappingVertexList((_VERTEXLIST *) (&p_Var5->aList[p_Var5->iNumList]),
                          pVectorInfo);
      }
    }
  }

  SGDPROCUNITHEADER **apProcUnitHead = pSGDHead->apProcUnitHead;

  for (unsigned int i = 0; i < pSGDHead->uiNumBlock; i++)
  {
    SGDPROCUNITHEADER *&rpPH = apProcUnitHead[i];
    g3ddbg_ASSERT(!((int) rpPH & 0xf), "sgd is illegal") if ((int) rpPH)
    {
      /* inlined from g3dUtil.h */
      rpPH = (SGDPROCUNITHEADER *) GetPtrOffset((int) pSGDHead,
                                                (int) apProcUnitHead[i]);
      /* end of inlined section */
    }
  }

  SGDPROCUNITHEADER *b =
      !pSGDHead->pVectorInfo
          ? (SGDPROCUNITHEADER *) nullptr
          : (SGDPROCUNITHEADER *) pSGDHead->pVectorInfo->aAddress[SVA_UNIQUE]
                .pvVertex;

  if ((b || pSGDHead->apProcUnitHead[0]) && b == pSGDHead->apProcUnitHead[0])
  {
    g3dbgMessage("Illegal SGD Data\n")
  }

  for (unsigned int i = 0; i < pSGDHead->uiNumBlock; i++)
  {
    u_int *vuvnprim = (u_int *) nullptr;
    SGDPROCUNITHEADER *pPUHead = apProcUnitHead[i];

    while (pPUHead)
    {
      g3ddbg_ASSERT(!((int) pPUHead & 0xf), "sgd is illegal")
          
      
      if (!pPUHead->pNext)
      {
        break;
      }

      pPUHead->pNext =
          (SGDPROCUNITHEADER *) ((int) pPUHead->pNext + (int) pPUHead);

      g3ddbg_ASSERT(!((int) pPUHead->pNext & 0xf),
                    "sgd is illegal") switch (pPUHead->iCategory)
      {
        case VUVN:
          vuvnprim = (u_int *) pPUHead;
          break;
        case MESH:
          MappingMeshData(pPUHead, (u_int *) vuvnprim, pSGDHead);
          break;
        case MATERIAL:
          pPUHead->VUMaterialDesc.pMat =
              &pSGDHead->pMaterial[pPUHead->VUMaterialDesc.iMaterialIndex];
          break;
        case COORDINATE:
          MappingCoordinateData((u_int *) pPUHead, (HeaderSection *) pSGDHead);
          break;
        case TRI2:
          RebuildTRI2Files(pPUHead);
          break;
        case END:
        g3ddbg_ASSERT(false, "") case BOUNDING_BOX:
          break;
        case GS_IMAGE:
          break;
        case MonotoneTRI2:
          break;
        case INVALID:
        default:
          g3ddbg_ASSERT(false, "pPUHead->iCategory : %d", pPUHead->iCategory)
      }

      pPUHead = pPUHead->pNext;
    }
  }
}

int _GetGlobalBufferSize()
{
  return s_iGlobalBufferSize;
}

void _SetVUVNPRIM(const SGDPROCUNITHEADER *ppuhVUVN)
{
  s_ppuhVUVN = (SGDPROCUNITHEADER*) ppuhVUVN;
}

void gra3dsgdDrawPresetDataObject(SGDPROCUNITHEADER *pPUHead)
{
  while (pPUHead)
  {
    switch (pPUHead->iCategory)
    {
      case VUVN:
        _SetVUVNPRIM(pPUHead);
        break;
      case MESH:
        SetVUMeshDataP(pPUHead);
        break;
      case MATERIAL:
        SetMaterialDataVU(pPUHead);
        break;
      case BOUNDING_BOX:
        if (!CheckBoundingBox(pPUHead))
        {
          return;
        }
        if (!BoundingBoxCalcP(pPUHead))
        {
          return;
        }

        _SetCoordData(pPUHead);

        if (save_tri2_pointer)
        {
          LoadTRI2Files(save_tri2_pointer);
          save_tri2_pointer = nullptr;
        }

        if (save_bw_pointer)
        {
          LoadTRI2Files(save_bw_pointer);
          save_bw_pointer = nullptr;
        }

        break;
      case GS_IMAGE:
        GsImageProcess(pPUHead);
        break;
      case 0x37:
        break;
      case 0x38:
        break;
      case 0x39:
        break;
      case 0x40:
        break;
      case 0x41:
        break;
        // default:                                        break;
    }

    pPUHead = pPUHead->pNext;
  }
}

static void _CalcWeightedVectorBuffer(_ONELIST *aList, int iNumList,
                                      float (*pvDest)[4],
                                      const float (*pvSrc)[4],
                                      void (*pFunc)(float *, const float *, float (*)[4], float (*)[4]))
{
  float matWork[4][4];
  float matWork2[4][4];
  static int s_iWriteSize;

  g3ddbg_ASSERT(pFunc, "");

  for (int i = 0; i < iNumList; i++)
  {
    _ONELIST &rList = aList[i];

    if (i == 0)
    {
      s_iWriteSize = 0;
    }

    s_iWriteSize += rList.usNumVector;

    g3ddbg_ASSERT(s_iWriteSize < s_iGlobalBufferSize, "");

    _CalcWeightedLocalWorldMatrix(
        matWork, gra3dsgdGetCoordinate(rList.sCoordId0)->matLocalWorld);

    /// Copies the matWork matrix for point 1 to float registers vf4, vf5, vf6, vf7
    //__asm__ __volatile__(
    //    "\n\
	//    lqc2    vf4,0x0(%0)\n\
	//    lqc2    vf5,0x10(%0)\n\
    //    lqc2    vf6,0x20(%0)\n\
	//    lqc2    vf7,0x30(%0)\n\
    //    "
    //    :
    //    : "r"(matWork));

    _CalcWeightedLocalWorldMatrix(
        matWork2, gra3dsgdGetCoordinate(rList.sCoordId1)->matLocalWorld);
    
    /// Copies the matWork matrix for point 2 to float registers vf8, vf9, vf10, vf11
    //__asm__ __volatile__(
    //    "\n\
	//    lqc2    vf8,0x0(%0)\n\
	//    lqc2    vf9,0x10(%0)\n\
    //    lqc2    vf10,0x20(%0)\n\
	//    lqc2    vf11,0x30(%0)\n\
    //    "
    //    :
    //    : "r"(matWork));

    for (int j = 0; j < rList.usNumVector; j++)
    {
      /// The float registers will be used within the function
      pFunc(*pvDest, *pvSrc,
            matWork,
            matWork2);
      pvSrc += 2;
      pvDest++;
    }
  }
}

void _CalcWeightedVertexBuffer(float *dp, const float *v, float (*matWorkP0)[4], float (*matWorkP1)[4])
{
  return;
  float x = v[0];
  float y = v[1];
  float z = v[2];
  float w = v[3];

  float x0 = matWorkP0[0][0] * x + matWorkP0[0][1] * y + matWorkP0[0][2] * z + matWorkP0[0][3] * w;
  float y0 = matWorkP0[1][0] * x + matWorkP0[1][1] * y + matWorkP0[1][2] * z + matWorkP0[1][3] * w;
  float z0 = matWorkP0[2][0] * x + matWorkP0[2][1] * y + matWorkP0[2][2] * z + matWorkP0[2][3] * w;
  float w0 = matWorkP0[3][0] * x + matWorkP0[3][1] * y + matWorkP0[3][2] * z + matWorkP0[3][3] * w;

  float x1 = matWorkP1[0][0] * x + matWorkP1[0][1] * y + matWorkP1[0][2] * z + matWorkP1[0][3] * w;
  float y1 = matWorkP1[1][0] * x + matWorkP1[1][1] * y + matWorkP1[1][2] * z + matWorkP1[1][3] * w;
  float z1 = matWorkP1[2][0] * x + matWorkP1[2][1] * y + matWorkP1[2][2] * z + matWorkP1[2][3] * w;
  float w1 = matWorkP1[3][0] * x + matWorkP1[3][1] * y + matWorkP1[3][2] * z + matWorkP1[3][3] * w;

  *dp++ = x0;
  *dp++ = y0;
  *dp++ = z0;
  *dp++ = w0;

  *dp++ = x1;
  *dp++ = y1;
  *dp++ = z1;
  *dp++ = w1;
}

static void CalcVertexBuffer(SGDPROCUNITHEADER *pPUHead)
{
  SGDCOORDINATEDESC *pCoord = &pPUHead->CoordDesc;
  SGDVECTORINFO *pVI = (SGDVECTORINFO *) _GetLPHEAD();
  ASSERT_RETURN(pCoord->iCoordId1 == 0);

  _VERTEXLIST *pVL = pVI->aAddress[SVA_WEIGHTED].pVertexList;

  ASSERT_RETURN(!pVL);

  _CalcWeightedVectorBuffer(pVL->aList, pVL->iNumList, s_pGlobalVertexBuffer,
                            pVI->aAddress[SVA_WEIGHTED].pvVertex,
                            _CalcWeightedVertexBuffer);

  pVL = (_VERTEXLIST *) &pVL->aList[pVL->iNumList];
  _CalcWeightedVectorBuffer(pVL->aList, pVL->iNumList, s_pGlobalNormalBuffer,
                            pVI->aAddress[SVA_WEIGHTED].pvNormal,
                            _CalcWeightedNormalBuffer);
}

SGDFILEHEADER *_GetSGDTop()
{
  return s_pSGDTop;
}

PHEAD *_GetLPHEAD()
{
  g3ddbg_ASSERT(_GetSGDTop(), "sgd_top_addr is null");
  return (PHEAD *) s_pSGDTop->pVectorInfo;
}

sceVu0FVECTOR *_GetGlobalVertexBuffer()
{
  return reinterpret_cast<sceVu0FVECTOR *>(s_pGlobalVertexBuffer);
}

sceVu0FVECTOR *_GetGlobalNormalBuffer()
{
  return reinterpret_cast<sceVu0FVECTOR *>(s_pGlobalNormalBuffer);
}

SGDCOORDINATE *gra3dsgdGetCoordinate(int iIndex)
{
  g3ddbg_ASSERT(iIndex < gra3dsgdGetNumBlock(), "");
  return &s_pCoordBase[iIndex];
}

CoordCache &_GetCoordCache()
{
  return ccahe;
}

void _SetCoordCache(const CoordCache &rCC)
{
  ccahe = rCC;
}

int gra3dsgdGetNumBlock()
{
  g3ddbg_ASSERT(_GetSGDTop(), "sgd_top_addr is null");
  return s_pSGDTop->uiNumBlock;
}

void sgdCalcBoneCoordinate(SGDCOORDINATE *pCoord, int iNumBlock)
{
  for (int i = 0; i < iNumBlock; i++)
  {
    pCoord[i].bCalc = false;
  }
  
  for (int i = 0; i < iNumBlock; i++)
  {
    sgdCalcCoordinateMatrix(&pCoord[i]);
  }
}

void sgdCalcCoordinateMatrix(SGDCOORDINATE *pCoord)
{
  if (pCoord && pCoord != (SGDCOORDINATE*)-1 && !pCoord->bCalc) 
  {    
    if (!pCoord->pParent || pCoord->pParent == (SGDCOORDINATE *)-1) 
    {
      sceVu0CopyMatrix(pCoord->matLocalWorld, pCoord->matCoord); 
    }
    else 
    {
      sgdCalcCoordinateMatrix(pCoord->pParent);
      sceVu0MulMatrix(pCoord->matLocalWorld, pCoord->pParent->matLocalWorld, pCoord->matCoord);
    }
    pCoord->bCalc = true;
  }
}

void SetVUMeshDataP(SGDPROCUNITHEADER* pPUHead) 
{
  SGDVUMESHDESC& rVUMeshDesc = pPUHead->VUMeshDesc;
  SGDVUMESHDATA_PRESET& rVUMeshData = (SGDVUMESHDATA_PRESET&)pPUHead[1];
  
  SGDVUVNDESC& rVUVNDesc = _GetVUVNPRIM()->VUVNDesc;
  SGDVUVNDATA_PRESET& rVUVNData = (SGDVUVNDATA_PRESET&)_GetVUVNPRIM()[1];
    
  u_int dsize;
    
  switch (rVUMeshDesc.ucMeshType) 
  {
    case iMT_0:
      gra3dDmaLoadVu1MicroProgram((unsigned int*)nullptr, 0);

      g3dDmaAddPacket(rVUMeshData.alData, rVUMeshDesc.iTagSize);
            
      dsize = _GetVUVNPRIM()->VUVNDesc.ucSize;
            
      g3dDmaAddPacket(_GetVUVNPRIM() + 4, dsize);

      if (_GetEdgeCheck()) 
      {
        gra3dCallMicroSubroutine1((unsigned int*)nullptr);
      } 
      else 
      {
        gra3dCallMicroSubroutine2((unsigned int*)nullptr);
      }
      break;
    case iMT_2:
    case iMT_2F:
      MeshType_iMT_2F(_GetVUVNPRIM(), pPUHead);
      gra3dDmaLoadVu1MicroProgram(nullptr, 0);
      g3dDmaAddPacket(&rVUVNData, rVUVNDesc.ucSize);
      g3dDmaAddPacket(rVUMeshData.alData, rVUMeshDesc.iTagSize);

      if (_GetEdgeCheck()) 
      {
        gra3dCallMicroSubroutine1((unsigned int*)nullptr);
      } 
      else 
      {
        gra3dCallMicroSubroutine2((unsigned int*)nullptr);
      }
      break;
    case 0x52:
    case 0x72:
      if (!_GetEdgeCheck()) 
      {
        gra3dCallMicroSubroutine2((unsigned int*)nullptr);
      }
      break;
    default:
      return;
  }
}

SGDPROCUNITHEADER* _GetVUVNPRIM()
{
  return s_ppuhVUVN;
}

int _GetEdgeCheck()
{
  return edge_check;
}

unsigned int *GetNextUnpackAddr(unsigned int *prim)
{
  while ((*prim & 0x60000000) != 0x60000000)
  {
    prim = prim + 1;
  }
  
  return prim;
}

/// Partially IMPLEMENTED
void SgSortUnitPrimPost(SGDPROCUNITHEADER *pPUHead)
{
  while (pPUHead) 
  {
    switch(pPUHead->iCategory) 
    {
      case VUVN:          _SetVUVNPRIM(pPUHead);     break;
      case MESH:          SetVUMeshDataPost(pPUHead);           break;
      case MATERIAL:      SetMaterialDataVU(pPUHead);           break;
      case COORDINATE:    CalcVertexBuffer(pPUHead);            break;
      case GS_IMAGE:      GsImageProcess(pPUHead);              break;
      default:            break;
    }
        
    pPUHead = pPUHead->pNext;
  }
}

/* /home/zero_rom/zero2np/src/graphics/graph3d/gra3dSGD.c */
void SetVUMeshData(SGDPROCUNITHEADER *pPUHead)
{
  SGDVUMESHDESC &rVUMeshDesc = pPUHead->VUMeshDesc;
  SGDVUMESHDATA *pVUMeshData = (SGDVUMESHDATA*) &pPUHead[1];
  SGDVUVNDESC *pVUVNDesc = (SGDVUVNDESC*)&s_ppuhVUVN->VUVNDesc;
  SGDVUVNDATA *pVUVNData = (SGDVUVNDATA*) &s_ppuhVUVN[1];
  
  switch (rVUMeshDesc.ucMeshType & 0xD3)
  {
    case 0:
      g3dDmaAddPacket(pVUMeshData, rVUMeshDesc.iTagSize);
      SetVUVNData(s_ppuhVUVN);
      // 0x12f8
      gra3dCallMicroSubroutine2(nullptr);
      break;
    case 2:
      g3dDmaAddPacket(pVUMeshData, rVUMeshDesc.iTagSize);
      SetVUVNData(s_ppuhVUVN);
      // 0x598
      gra3dCallMicroSubroutine2(nullptr);
      break;
    case 0x80:
      g3dDmaAddPacket(pVUMeshData, rVUMeshDesc.iTagSize);
      g3dDmaAddPacket(pVUVNData, pVUVNDesc->ucSize);
      // 0x12F8
      gra3dCallMicroSubroutine2(nullptr);
      break;
    case 0x82:
      MeshType0x82(s_ppuhVUVN, pPUHead);
      g3dDmaAddPacket(pVUMeshData, rVUMeshDesc.iTagSize);
      g3dDmaAddPacket(pVUVNData, pVUVNDesc->ucSize);
      // 0x598
      gra3dCallMicroSubroutine2(nullptr);
      break;
    case 0x42:
      g3dDmaAddPacket(pVUMeshData, rVUMeshDesc.iTagSize);
      // 0x1968
      gra3dCallMicroSubroutine2(nullptr);
      break;
    default: g3ddbg_ASSERT(false, "来てはいけないところに来ています。(mtype:%d)", rVUMeshDesc.ucMeshType);
  }
}

void _SetCoordData(SGDPROCUNITHEADER *pPUHead) 
{
  g3ddbg_ASSERT(s_pCoordBase, "");
  sgdCalcBoneCoordinate(s_pCoordBase, gra3dsgdGetNumBlock() - 1);
  SGDCOORDINATE *cp0 = gra3dsgdGetCoordinate(pPUHead->CoordDesc.iCoordId0);
  memcpy(&g_scratchpadLayout.Vu1Mem.Packed.Transform.matLocalWorld, &cp0->matCoord, sizeof(float[4][4]));
  
  glm::mat4 trans = glm::make_mat4((float*)g_scratchpadLayout.Vu1Mem.Packed.Transform.matLocalWorld);
  
  unsigned int transformLoc = context->GetUniformLocation(shaderProgram, "model");
  context->UniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
}

/// NOT IMPLEMENTED
void sgdResetMaterialCache(SGDFILEHEADER *pSGDData)
{
}

void gra3dsgdSetupVu1()
{
}

void SgSortPreProcess(u_int *_prim)
{
}

void SgPreRenderPrim(SGDPROCUNITHEADER *pPUHead)
{
}

void SgClearPreRenderPrim(SGDPROCUNITHEADER *pPUHead)
{
}

void SetVUVNDataPost(SGDPROCUNITHEADER *pPUHead)
{
}

void LoadTRI2Files(SGDPROCUNITHEADER *pPUHead)
{
}

void GsImageProcess(const SGDPROCUNITHEADER *pPUHead)
{
}

int CheckBoundingBox(SGDPROCUNITHEADER *pPUHead)
{
  return 1;
}

int BoundingBoxCalcP(SGDPROCUNITHEADER *_prim)
{
  return 1;
}

void _CalcWeightedLocalWorldMatrix(float (*matRet)[4],
                                   float (*matLocalWorld)[4])
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      matRet[i][j] = matLocalWorld[i][j];
    }
  }
}

void _CalcWeightedNormalBuffer(float *dp, const float *v, float (*matWorkP0)[4], float (*matWorkP1)[4])
{
}

void SetVUVNData(SGDPROCUNITHEADER *pPUHead)
{
}
