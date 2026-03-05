// eyes.cpp — SDL2 port of DEMO/EYES
// Uses SDL's own X11 display handle for XQueryPointer → works in VNC

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <X11/Xlib.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

static Display* gDisplay = nullptr;
static Window   gRoot    = 0;

static void queryGlobalMouse(int* mx, int* my) {
    if (!gDisplay) { SDL_GetGlobalMouseState(mx, my); return; }
    Window rw, cw; int rx,ry,wx,wy; unsigned int mask;
    XQueryPointer(gDisplay, gRoot, &rw, &cw, &rx, &ry, &wx, &wy, &mask);
    *mx = rx; *my = ry;
}

// ── Drawing helpers ───────────────────────────────────────────────────────────
static void fillEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry,
                        Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    for (int y = -ry; y <= ry; ++y) {
        double f = 1.0 - (double)(y*y)/(double)(ry*ry);
        if (f < 0) continue;
        int hw = (int)(rx * sqrt(f));
        SDL_RenderDrawLine(r, cx-hw, cy+y, cx+hw, cy+y);
    }
}
static void outlineEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry,
                           int thick, Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    int steps = (rx+ry)*6;
    for (int t = 0; t < thick; ++t)
        for (int i = 0; i <= steps; ++i) {
            double a = 2.0*M_PI*i/steps;
            SDL_RenderDrawPoint(r, cx+(int)((rx+t)*cos(a)), cy+(int)((ry+t)*sin(a)));
        }
}

// ── Pupil clamped to inner ellipse ───────────────────────────────────────────
static void getPupil(int dx, int dy, int a, int b, int* xe, int* ye) {
    if (!dx && !dy) { *xe=*ye=0; return; }
    double A=a*0.65, B=b*0.65;
    double alpha = atan2((double)dy,(double)dx);
    double px=A*cos(alpha), py=B*sin(alpha);
    double dist=sqrt((double)dx*dx+(double)dy*dy);
    double maxd=sqrt(px*px+py*py);
    *xe=(dist<maxd)?dx:(int)px;
    *ye=(dist<maxd)?dy:(int)py;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    int W=320, H=160;
    bool borderless=true;
    SDL_Window* win = SDL_CreateWindow("Eyes",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H,
        SDL_WINDOW_SHOWN|SDL_WINDOW_ALWAYS_ON_TOP|SDL_WINDOW_BORDERLESS);
    SDL_Renderer* ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);

    // Grab the X11 display from SDL — this is the VNC display
    SDL_SysWMinfo wm; SDL_VERSION(&wm.version);
    if (SDL_GetWindowWMInfo(win, &wm) && wm.subsystem == SDL_SYSWM_X11) {
        gDisplay = wm.info.x11.display;
        gRoot    = DefaultRootWindow(gDisplay);
        fprintf(stderr,"[eyes] Using X11 display from SDL: %p\n",(void*)gDisplay);
    } else {
        fprintf(stderr,"[eyes] SDL_GetWindowWMInfo failed, fallback to SDL_GetGlobalMouseState\n");
    }

    Uint32 lastClick=0;
    bool dragging=false, dragSet=false;
    int dragOX=0,dragOY=0;
    bool running=true;

    while (running) {
        SDL_GetWindowSize(win,&W,&H);
        int xL=W/4, xR=3*W/4, yE=H/2;
        int a=W/4-12, b=H/2-(borderless?12:22);
        if(a<5)a=5; if(b<5)b=5;
        int aP=std::max(a/5,5), bP=std::max(b/5,5);

        SDL_Event e;
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN&&e.key.keysym.sym==SDLK_ESCAPE) running=false;

            if(e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT){
                Uint32 now=SDL_GetTicks();
                if(now-lastClick<400){
                    borderless=!borderless;
                    SDL_SetWindowBordered(win,borderless?SDL_FALSE:SDL_TRUE);
                    SDL_SetWindowAlwaysOnTop(win,borderless?SDL_TRUE:SDL_FALSE);
                    lastClick=0; dragging=false;
                } else {
                    lastClick=now;
                    if(borderless){ dragging=true; dragSet=false; }
                }
            }
            if(e.type==SDL_MOUSEBUTTONUP&&e.button.button==SDL_BUTTON_LEFT)
                dragging=false;
            if(e.type==SDL_MOUSEMOTION&&dragging){
                if(!dragSet){ dragOX=e.motion.x; dragOY=e.motion.y; dragSet=true; }
                int wx,wy; SDL_GetWindowPosition(win,&wx,&wy);
                SDL_SetWindowPosition(win, wx+e.motion.x-dragOX,
                                          wy+e.motion.y-dragOY);
            }
            if(e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_RIGHT&&borderless)
                running=false;
        }

        int mx,my; queryGlobalMouse(&mx,&my);
        int wx,wy; SDL_GetWindowPosition(win,&wx,&wy);

        SDL_SetRenderDrawColor(ren,28,28,38,255);
        SDL_RenderClear(ren);

        for(int eye=0;eye<2;++eye){
            int cx=(eye?xR:xL);
            fillEllipse(ren,cx,yE,a,b,238,238,230);
            outlineEllipse(ren,cx,yE,a,b,2,25,25,35);
            int dx=mx-(wx+cx), dy=my-(wy+yE), xe,ye;
            getPupil(dx,dy,a,b,&xe,&ye);
            fillEllipse(ren,cx+xe,yE+ye,aP+3,bP+3,45,90,160);
            fillEllipse(ren,cx+xe,yE+ye,aP,bP,8,8,8);
            fillEllipse(ren,cx+xe+aP/3,yE+ye-bP/3,aP/4+1,bP/4+1,210,220,255);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
