#include "Eyes.hpp"
#include <imgui.h>
#include <cmath>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
// Register JS mouse tracking relative to canvas
EM_JS(void, js_eyes_setup, (), {
    window._eyesMX = 0; window._eyesMY = 0;
    document.addEventListener('mousemove', function(e) {
        var rect = Module.canvas.getBoundingClientRect();
        window._eyesMX = e.clientX - rect.left;
        window._eyesMY = e.clientY - rect.top;
    });
});
EM_JS(int, js_eyes_mx, (), { return window._eyesMX|0; });
EM_JS(int, js_eyes_my, (), { return window._eyesMY|0; });
#endif

void EyesPlugin::setup(SDL_Renderer*) {
#ifdef __EMSCRIPTEN__
    js_eyes_setup();
#endif
}

void EyesPlugin::fillEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry,
                              Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    for (int y = -ry; y <= ry; ++y) {
        double f = 1.0 - (double)(y*y)/(double)(ry*ry);
        if (f < 0) continue;
        int hw = (int)(rx * sqrt(f));
        SDL_RenderDrawLine(r, cx-hw, cy+y, cx+hw, cy+y);
    }
}

void EyesPlugin::outlineEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry,
                                 int thick, Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    int steps = (rx+ry)*6;
    for (int t = 0; t < thick; ++t)
        for (int i = 0; i <= steps; ++i) {
            double a = 2.0*M_PI*i/steps;
            SDL_RenderDrawPoint(r, cx+(int)((rx+t)*cos(a)), cy+(int)((ry+t)*sin(a)));
        }
}

void EyesPlugin::getPupil(int dx, int dy, int a, int b, int* xe, int* ye) {
    if (!dx && !dy) { *xe=*ye=0; return; }
    double A=a*0.65, B=b*0.65;
    double alpha = atan2((double)dy,(double)dx);
    double px=A*cos(alpha), py=B*sin(alpha);
    double dist = sqrt((double)dx*dx+(double)dy*dy);
    double maxd = sqrt(px*px+py*py);
    *xe = (dist<maxd) ? dx : (int)px;
    *ye = (dist<maxd) ? dy : (int)py;
}

void EyesPlugin::renderScene(const RenderContext& ctx) {
    int W=ctx.width, H=ctx.height, top=ctx.topOffset;
    int drawH = H - top;

    SDL_SetRenderDrawColor(ctx.renderer, bgR_, bgG_, bgB_, 255);
    SDL_RenderClear(ctx.renderer);

    // Get mouse position
    int mx, my;
#ifdef __EMSCRIPTEN__
    mx = js_eyes_mx();
    my = js_eyes_my();
#else
    SDL_GetMouseState(&mx, &my);
#endif

    // Eye layout: 1 or 2 eyes centered vertically in draw area
    int n = numEyes_;
    int eyeW = W / (n*2);          // half-width per eye slot
    int a = eyeW - eyeW/5;         // eyeball x-radius
    int b = drawH/2 - drawH/8;     // eyeball y-radius
    if(a<6)a=6; if(b<6)b=6;
    int aP=std::max(a/5,5), bP=std::max(b/5,5);
    int yE = top + drawH/2;

    for (int i = 0; i < n; ++i) {
        int cx = W*(2*i+1)/(2*n);

        // White eyeball
        fillEllipse(ctx.renderer, cx, yE, a, b, 238, 238, 230);
        outlineEllipse(ctx.renderer, cx, yE, a, b, 2, 25, 25, 35);

        // Pupil
        int dx=mx-cx, dy=my-yE, xe, ye;
        getPupil(dx, dy, a, b, &xe, &ye);
        fillEllipse(ctx.renderer, cx+xe, yE+ye, aP+3, bP+3, 45, 90, 160);
        fillEllipse(ctx.renderer, cx+xe, yE+ye, aP,   bP,   8,   8,   8);
        fillEllipse(ctx.renderer, cx+xe+aP/3, yE+ye-bP/3, aP/4+1, bP/4+1, 210, 220, 255);
    }
}

void EyesPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({220,120},ImGuiCond_Once);
    ImGui::Begin("Eyes");
    ImGui::SeparatorText("Settings");
    ImGui::SliderInt("Eyes", &numEyes_, 1, 4);
    ImGui::SeparatorText("Background");
    float bg[3]={bgR_/255.f,bgG_/255.f,bgB_/255.f};
    if(ImGui::ColorEdit3("##bg",bg)){
        bgR_=(int)(bg[0]*255); bgG_=(int)(bg[1]*255); bgB_=(int)(bg[2]*255);
    }
    ImGui::End();
}
