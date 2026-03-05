/* gwb_3d.h — standalone 3D projection (no Windows) */
#ifndef _GWB_3D_H
#define _GWB_3D_H 1

#include "gwb_mat.h"

#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C extern
#endif

EXTERN_C void GetEyePosi(vector eye);
EXTERN_C int  GwbInit(int W, int H, double ox, double oy, double oz,
                      double Radiu, double Theta, double Rz, double dep);
EXTERN_C int  transEye(vector posi, vector eye);
EXTERN_C int  EyeToScreenXYZ(vector eye, double *x, double *y, double *z);
EXTERN_C int  trans(vector posi, vector result);
EXTERN_C int  transXYZ(vector posi, double *x, double *y, double *z);

#endif
