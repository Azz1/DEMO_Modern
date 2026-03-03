#include "Pfrac.hpp"
#include <imgui.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

void PfracPlugin::setup(SDL_Renderer*) {
    srand((unsigned)time(nullptr));

    // Presets from Demo.ini DATA (format: a b c d e f prob per transform)
    presets_ = {
        {"Barnsley Fern", {
            {0,0,0,0.16f,0,0,1},
            {0.85f,0.04f,-0.04f,0.85f,0,1.6f,85},
            {0.2f,-0.26f,0.23f,0.22f,0,1.6f,7},
            {-0.15f,0.28f,0.26f,0.24f,0,0.44f,7}
        }, -4.f,0.f,4.f,10.f},

        {"Tree", {
            {0.5f,0,0,0.5f,0,0,5},
            {0.42f,-0.42f,0.42f,0.42f,0,0.2f,40},
            {0.42f,0.42f,-0.42f,0.42f,0,0.2f,40},
            {0.1f,0,0,0.1f,0,0.2f,15}
        }, -0.333f,0.f,0.333f,0.5f},

        {"Sierpinski", {
            {0.5f,0,0,0.5f,0,0,33},
            {0.5f,0,0,0.5f,0.25f,0.433f,33},
            {0.5f,0,0,0.5f,0.5f,0,34}
        }, 0.f,0.f,1.f,1.f},

        {"Levy Curve", {
            {0.5f,-0.5f,0.5f,0.5f,0,0,50},
            {0.5f,0.5f,-0.5f,0.5f,0.5f,0.5f,50}
        }, -1.f,-0.5f,2.f,2.f},

        {"Flambo", {
            {0.25f,0,0,0.5f,0,0,15},
            {0.5f,0,0,0.5f,-0.25f,0.5f,31},
            {-0.25f,0,0,-0.25f,0.25f,1.f,8},
            {0.5f,0,0,0.5f,0,0.75f,31},
            {0.5f,0,0,-0.25f,0.5f,1.25f,15}
        }, -1.f,0.f,1.f,2.f},
    };
    dirty_=true;
}

void PfracPlugin::teardown() { if(tex_){SDL_DestroyTexture(tex_);tex_=nullptr;} }

void PfracPlugin::render(SDL_Renderer* r, int w, int h, int top) {
    int th=h-top;
    if(!tex_||texW_!=w||texH_!=th){
        if(tex_) SDL_DestroyTexture(tex_);
        tex_=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_STREAMING,w,th);
        texW_=w; texH_=th;
    }
    void* px; int pitch;
    SDL_LockTexture(tex_,nullptr,&px,&pitch);
    Uint32* p=(Uint32*)px;
    for(int i=0;i<w*th;++i) p[i]=0x050510FF;

    auto& pr=presets_[preset_];
    double ratex=w/(pr.x1-pr.x0), ratey=th/(pr.y1-pr.y0);

    double x=0,y=0;
    int cumProb[6]={};
    int n=(int)pr.tr.size();
    cumProb[0]=pr.tr[0].prob;
    for(int i=1;i<n;++i) cumProb[i]=cumProb[i-1]+pr.tr[i].prob;

    for(int i=0;i<count_;++i){
        int v=rand()%100;
        int ti=0;
        for(;ti<n-1&&v>=cumProb[ti];++ti);
        auto& t=pr.tr[ti];
        double nx=t.a*x+t.b*y+t.e;
        double ny=t.c*x+t.d*y+t.f;
        x=nx; y=ny;

        int sx=int((x-pr.x0)*ratex);
        int sy=int(th-1-(y-pr.y0)*ratey);
        if(sx>=0&&sx<w&&sy>=0&&sy<th){
            // Color by transform index
            Uint32 cols[]={0x00FF44FF,0xFF8800FF,0xFF44AAFF,0x44AAFFFF,0xFFFF44FF};
            p[sy*(pitch/4)+sx]=cols[ti%5];
        }
    }
    SDL_UnlockTexture(tex_);
    dirty_=false;
}

void PfracPlugin::renderScene(const RenderContext& ctx) {
    if(dirty_) render(ctx.renderer,ctx.width,ctx.height,ctx.topOffset);
    if(tex_){
        SDL_Rect dst={0,ctx.topOffset,ctx.width,ctx.height-ctx.topOffset};
        SDL_RenderCopy(ctx.renderer,tex_,nullptr,&dst);
    }
}

void PfracPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({255,160},ImGuiCond_Once);
    ImGui::Begin("IFS Fractal");
    ImGui::SeparatorText("Preset");
    const char* names[]={"Barnsley Fern","Tree","Sierpinski","Levy Curve","Flambo"};
    if(ImGui::Combo("##p",&preset_,names,5)) dirty_=true;
    ImGui::SeparatorText("Points");
    if(ImGui::SliderInt("count",&count_,1000,500000)) dirty_=true;
    if(ImGui::Button("Regenerate")) dirty_=true;
    ImGui::SeparatorText("Info");
    ImGui::TextDisabled("IFS random iteration");
    ImGui::TextDisabled("Colors = transform index");
    ImGui::End();
}
