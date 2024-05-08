#include "libvu0.h"

void sceVu0CopyMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
  for (auto i = 0; i < 4; i++)
  {
    for (auto j = 0; j < 4; j++)
    {
      m0[i][j] = m1[i][j];
    }
  }
}

void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2)
{
  for (auto i = 0; i < 4; i++)
  {
    for (auto j = 0; j < 4; j++)
    {
      m0[i][j] = m1[i][0] * m2[0][j] + 
                 m1[i][1] * m2[1][j] +
                 m1[i][2] * m2[2][j] + 
                 m1[i][3] * m2[3][j];
    }
  }
}
