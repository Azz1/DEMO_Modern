/* gwb_shgwb.c — SDL2 display with back-face culling for hidden line removal */
#include "gwb.h"
#include <SDL2/SDL.h>

/* Precompute face equations for all faces in a solid */
static void precomputeFaceEq(Solid *s) {
    Face *f = s->sfaces;
    while (f) {
        Loop *l = f->flout ? f->flout : f->floops;
        if (l) faceeq(l, f->feq);
        f = f->nextf;
    }
}

/*
 * Back-face culling:
 * face equation feq satisfies feq·p + feq[3] = 0 on the face plane.
 * If feq·eye + feq[3] > 0  → eye is on the "front" (outward normal) side.
 * We draw an edge if at least one of its two adjacent faces is front-facing.
 */
static int faceVisible(Face *f, const vector eye) {
    double d = f->feq[0]*eye[0] + f->feq[1]*eye[1]
             + f->feq[2]*eye[2] + f->feq[3];
    return d >= 0.0;
}

static int edgeVisible(Edge *e, const vector eye) {
    return faceVisible(e->he1->wloop->lface, eye)
        || faceVisible(e->he2->wloop->lface, eye);
}

static void sline(SDL_Renderer *r, int top, vector p1, vector p2,
                  Uint8 cr, Uint8 cg, Uint8 cb) {
    double x1,y1,z1,x2,y2,z2;
    if (transXYZ(p1,&x1,&y1,&z1) && transXYZ(p2,&x2,&y2,&z2)) {
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_RenderDrawLine(r, (int)x1, (int)y1+top, (int)x2, (int)y2+top);
    }
}

int gGwbSolidMode = 1;  /* 0 = wireframe, 1 = solid (back-face cull) */

void GwbOutLine(int W, int H, int top, void *renderer) {
    SDL_Renderer *r = (SDL_Renderer*)renderer;
    Solid  *s = firsts;
    Edge   *e;
    int     solidIdx = 0;
    vector  eye;
    (void)W; (void)H;

    static const Uint8 colors[][3] = {
        {255,220,  0},   /* yellow  */
        { 80,200,255},   /* cyan    */
        {255,120, 40},   /* orange  */
        {180,255,100},   /* green   */
        {255, 80,180},   /* pink    */
    };
    int ncolors = (int)(sizeof(colors)/sizeof(colors[0]));

    GetEyePosi(eye);   /* world position of the eye */

    while (s != NULL) {
        int ci = solidIdx % ncolors;
        Uint8 ucr=colors[ci][0], ucg=colors[ci][1], ucb=colors[ci][2];

        if (gGwbSolidMode) precomputeFaceEq(s);

        e = s->sedges;
        while (e != NULL) {
            int draw = gGwbSolidMode ? edgeVisible(e, eye) : 1;
            if (draw)
                sline(r, top,
                      e->he1->vex->vcoord,
                      e->he2->vex->vcoord,
                      ucr, ucg, ucb);
            e = e->nexte;
        }
        s = s->nexts;
        solidIdx++;
    }
}
