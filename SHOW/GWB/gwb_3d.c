/* gwb_3d.c — 3D projection (adapted from SRCPOOL/3d.c, no Windows) */
#include "gwb_3d.h"
#include <math.h>

static double  gv, gu;
static double  ga, gb, gc, gd, ge, gf_z;
static int     gvalid;
static int     gVsx, gVsy;
static double  gD;

void GetEyePosi(vector eye) {
    eye[0] = ga+gd; eye[1] = gb+ge; eye[2] = gc+gf_z;
}

int GwbInit(int W, int H, double ox, double oy, double oz,
            double Radiu, double Theta, double Rz, double dep) {
    double tmp;
    Theta /= 360.0; Theta -= floor(Theta); Theta *= 360.0;
    tmp = fabs(Theta)/180.0;
    if (tmp - floor(tmp) < 0.01) {
        if (Theta > 0.0 || Theta > 180.0) Theta += 0.01; else Theta -= 0.01;
    }
    if (Theta > 180.0) { Theta = 360.0-Theta; Rz += 180.0; }

    gd = ox; ge = oy; gf_z = oz;
    ga = Radiu * cos(PI*Rz/180.0) * sin(PI*Theta/180.0);
    gb = Radiu * sin(PI*Rz/180.0) * sin(PI*Theta/180.0);
    gc = Radiu * cos(PI*Theta/180.0);
    gv = sqrt(ga*ga + gb*gb);
    gu = sqrt(ga*ga + gb*gb + gc*gc);
    gD = fabs(dep);
    gVsx = W/2; gVsy = H/2;
    gvalid = (gu > 100.0*FPS && gv > 100.0*FPS && gD > 100.0*FPS);
    return gvalid;
}

int transEye(vector posi, vector eye) {
    double xw, yw, zw, ab;
    if (gvalid) {
        xw = posi[0]-gd; yw = posi[1]-ge; zw = posi[2]-gf_z;
        ab = ga*xw + gb*yw;
        eye[0] = (-gb*xw + ga*yw)/gv;
        eye[1] = (-ab*gc + zw*gv*gv)/(gu*gv);
        eye[2] = (-ab - gc*zw + gu*gu)/gu;
    }
    return gvalid;
}

int EyeToScreenXYZ(vector eye, double *x, double *y, double *z) {
    double w, xs, ys;
    if (gvalid && eye[2] >= FPS) {
        w = eye[2]/gD;
        xs = eye[0]/w; ys = eye[1]/w;
        *x = xs+gVsx; *y = ys+gVsy; *z = eye[2];
        if (fabs(xs) <= 2048.0 && fabs(ys) <= 2048.0) return 1;
    }
    *z = -1.0; return 0;
}

int transXYZ(vector posi, double *x, double *y, double *z) {
    vector eye;
    if (transEye(posi, eye)) return EyeToScreenXYZ(eye, x, y, z);
    return 0;
}

int trans(vector posi, vector result) {
    return transXYZ(posi, &result[0], &result[1], &result[2]);
}
