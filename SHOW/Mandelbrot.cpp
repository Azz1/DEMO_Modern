#include "Mandelbrot.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

static SDL_Color mapColor(int iter, int maxIter, int scheme) {
    if (iter == maxIter) return {0, 0, 0, 255};
    float t = float(iter) / float(maxIter);
    switch (scheme) {
    case 0: {
        float r = 9*(1-t)*t*t*t, g = 15*(1-t)*(1-t)*t*t, b = 8.5f*(1-t)*(1-t)*(1-t)*t;
        return {(Uint8)(r*255),(Uint8)(g*255),(Uint8)(b*255),255};
    }
    case 1: {
        float r = std::min(1.f,t*3.f), g = std::max(0.f,t*3.f-1.f), b = std::max(0.f,t*3.f-2.f);
        return {(Uint8)(r*255),(Uint8)(g*255),(Uint8)(b*255),255};
    }
    default: { Uint8 v=(Uint8)(t*255); return {v,v,v,255}; }
    }
}

static int mandelbrot(double cr, double ci, int maxIter) {
    double zr=0, zi=0;
    for (int i=0; i<maxIter; ++i) {
        double zr2=zr*zr, zi2=zi*zi;
        if (zr2+zi2>4.0) return i;
        zi=2*zr*zi+ci; zr=zr2-zi2+cr;
    }
    return maxIter;
}

void MandelbrotPlugin::setup(SDL_Renderer* r) { renderer_=r; }

void MandelbrotPlugin::recompute(int w, int h) {
    if (!tex_ || texW_!=w || texH_!=h) {
        if (tex_) SDL_DestroyTexture(tex_);
        tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, h);
        texW_=w; texH_=h;
    }
    void* pixels; int pitch;
    SDL_LockTexture(tex_, nullptr, &pixels, &pitch);
    Uint32* px=(Uint32*)pixels;
    double aspect=double(w)/double(h);
    double scaleX=zoom_*aspect/w, scaleY=zoom_/h;
    for (int y=0; y<h; ++y) {
        double ci=cy_+(y-h*0.5)*scaleY;
        for (int x=0; x<w; ++x) {
            double cr=cx_+(x-w*0.5)*scaleX;
            int iter=mandelbrot(cr,ci,maxIter_);
            SDL_Color c=mapColor(iter,maxIter_,colorScheme_);
            px[y*(pitch/4)+x]=((Uint32)c.r<<24)|((Uint32)c.g<<16)|((Uint32)c.b<<8)|0xFF;
        }
    }
    SDL_UnlockTexture(tex_);
    dirty_=false;
}

void MandelbrotPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({270,250}, ImGuiCond_Once);
    ImGui::Begin("Mandelbrot Set");

    ImGui::SeparatorText("View");
    double mnX=-2.5, mxX=1.0;
    double mnY=-1.5, mxY=1.5;
    double mnZ=0.0001, mxZ=4.0;
    dirty_ |= ImGui::SliderScalar("center X", ImGuiDataType_Double, &cx_, &mnX, &mxX);
    dirty_ |= ImGui::SliderScalar("center Y", ImGuiDataType_Double, &cy_, &mnY, &mxY);
    dirty_ |= ImGui::SliderScalar("zoom",     ImGuiDataType_Double, &zoom_, &mnZ, &mxZ,
                                  "%.6f", ImGuiSliderFlags_Logarithmic);
    dirty_ |= ImGui::SliderInt("max iter", &maxIter_, 32, 1024);

    ImGui::SeparatorText("Color");
    const char* schemes[]={"Classic","Fire","Grayscale"};
    dirty_ |= ImGui::Combo("scheme", &colorScheme_, schemes, 3);

    ImGui::SeparatorText("Presets");
    if (ImGui::Button("Full"))       { cx_=-0.5;   cy_=0;     zoom_=3.0;   dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("Seahorse"))   { cx_=-0.743; cy_=0.127; zoom_=0.02;  dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("Elephant"))   { cx_=0.3;    cy_=0.0;   zoom_=0.1;   dirty_=true; }

    ImGui::Separator();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}

void MandelbrotPlugin::renderScene(const RenderContext& ctx) {
    // draw selector overlay after texture
    if (dirty_ || !tex_ || texW_!=ctx.width || texH_!=ctx.height)
        recompute(ctx.width, ctx.height);
    if (tex_) SDL_RenderCopy(ctx.renderer, tex_, nullptr, nullptr);
    sel_.draw(ctx.renderer);
}

void MandelbrotPlugin::teardown() {
    if (tex_) { SDL_DestroyTexture(tex_); tex_=nullptr; }
}

void MandelbrotPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if (sel_.isReady()) {
        SDL_Rect r = sel_.consume();
        applyZoom(r, ctx.width, ctx.height, cx_, cy_, zoom_);
        dirty_ = true;
    }
}
