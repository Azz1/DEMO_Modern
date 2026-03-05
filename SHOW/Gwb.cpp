#include "Gwb.hpp"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <cstring>
#include <algorithm>

/* Pull in all GWB C code */
extern "C" {
#include "GWB/gwb.h"
}

// ── Scene definitions ─────────────────────────────────────────────────────────
static const char* kSceneNames[] = {
    "Cylinder + Ball + Torus",
    "Block",
    "Torus",
    "Ball",
    "Cylinder",
};
static const int kNumScenes = (int)(sizeof(kSceneNames)/sizeof(kSceneNames[0]));

void GwbPlugin::buildScene(int idx) {
    RemoveAll();
    Solid *s;
    switch(idx) {
    case 0:  // Original demo scene
        s = Gcyl(1, 20, 50, 20);
        stranslate(s, 0.0, 0.0, 0.0);
        s = GBall(2, 30, 12, 12);
        stranslate(s, 0.0, 0.0, 120.0);
        s = Gtorus(3, 80, 10, 20, 8);
        srotate(s, 0.0, 75.0, 0.0);
        stranslate(s, 0.0, 0.0, -90.0);
        ox_=0; oy_=0; oz_=0;
        radiu_=400; depF_=300;
        break;
    case 1:
        s = Gblock(1, 80, 60, 40);
        stranslate(s, -40.0, -30.0, -20.0);
        ox_=0; oy_=0; oz_=0;
        radiu_=300; depF_=250;
        break;
    case 2:
        s = Gtorus(1, 80, 20, 24, 10);
        ox_=0; oy_=0; oz_=0;
        radiu_=350; depF_=280;
        break;
    case 3:
        s = GBall(1, 60, 16, 16);
        ox_=0; oy_=0; oz_=0;
        radiu_=300; depF_=250;
        break;
    case 4:
        s = Gcyl(1, 40, 80, 24);
        ox_=0; oy_=0; oz_=0;
        radiu_=300; depF_=250;
        break;
    }
    built_ = true;
}

void GwbPlugin::setup(SDL_Renderer*) {
    SetFirsts(nullptr);
    buildScene(scene_);
}

void GwbPlugin::teardown() {
    RemoveAll();
    built_ = false;
}

void GwbPlugin::renderScene(const RenderContext& ctx) {
    int W=ctx.width, H=ctx.height, top=ctx.topOffset;
    int dH = H-top;

    SDL_SetRenderDrawColor(ctx.renderer, 8, 8, 18, 255);
    SDL_RenderClear(ctx.renderer);

    if (!built_) return;

    GwbInit(W, dH, ox_, oy_, oz_, radiu_, theta_, rz_, depF_);
    gGwbSolidMode = drawMode_;
    GwbOutLine(W, dH, top, ctx.renderer);
}

void GwbPlugin::handleEvent(const SDL_Event& e, const RenderContext&) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT) {
        pan_=true; panSX_=e.button.x; panSY_=e.button.y;
        panBaseTheta_=theta_; panBaseRz_=rz_;
    }
    if (e.type==SDL_MOUSEBUTTONUP&&e.button.button==SDL_BUTTON_LEFT) pan_=false;
    if (e.type==SDL_MOUSEMOTION&&pan_) {
        float dx=e.motion.x-panSX_, dy=e.motion.y-panSY_;
        rz_    = panBaseRz_    - dx*0.4f;
        theta_ = std::max(5.f, std::min(175.f, panBaseTheta_+dy*0.3f));
    }
    if (e.type==SDL_MOUSEWHEEL) {
        radiu_ = std::max(50.f, radiu_ - e.wheel.y*20.f);
    }
}

void GwbPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({260,280},ImGuiCond_Once);
    ImGui::Begin("GWB Solids");

    ImGui::SeparatorText("Scene");
    if (ImGui::Combo("##scene", &scene_, kSceneNames, kNumScenes))
        buildScene(scene_);

    ImGui::SeparatorText("Mode");
    ImGui::RadioButton("3D Line",  &drawMode_, 0); ImGui::SameLine();
    ImGui::RadioButton("3D Solid", &drawMode_, 1);

    ImGui::SeparatorText("3D View");
    ImGui::SliderFloat("Elevation", &theta_, 5.f, 175.f);
    ImGui::SliderFloat("Azimuth",   &rz_,   -180.f, 180.f);
    ImGui::SliderFloat("Distance",  &radiu_, 50.f, 1000.f);
    ImGui::SliderFloat("Focal",     &depF_,  50.f,  800.f);
    if (ImGui::Button("Reset View")) { theta_=60; rz_=30; radiu_=400; depF_=300; }
    ImGui::TextDisabled("LMB drag = rotate  Scroll = zoom");

    ImGui::SeparatorText("Info");
    // Count solids/edges
    int ns=0,ne=0;
    Solid *s=firsts;
    while(s){ns++;Edge*e=s->sedges;while(e){ne++;e=e->nexte;}s=s->nexts;}
    ImGui::Text("Solids: %d   Edges: %d", ns, ne);

    ImGui::End();
}
