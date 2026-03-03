#include "Julia.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

static SDL_Color juliaColor(int iter, int maxIter, int scheme) {
    if (iter == maxIter) return {0,0,0,255};
    float t = float(iter) / float(maxIter);
    switch (scheme) {
    case 0: { // Purple-cyan
        float r = t*t;
        float g = std::max(0.f, 2*t - 1.f);
        float b = sinf(t * 3.14159f);
        return {(Uint8)(r*200),(Uint8)(g*255),(Uint8)(b*255),255};
    }
    case 1: { // Rainbow (HSV-ish)
        float h = t * 6.f;
        int   hi = (int)h;
        float f  = h - hi;
        float q  = 1-f;
        switch (hi % 6) {
        case 0: return {255,(Uint8)(f*255),0,255};
        case 1: return {(Uint8)(q*255),255,0,255};
        case 2: return {0,255,(Uint8)(f*255),255};
        case 3: return {0,(Uint8)(q*255),255,255};
        case 4: return {(Uint8)(f*255),0,255,255};
        default:return {255,0,(Uint8)(q*255),255};
        }
    }
    default: { // Gold
        float r = std::min(1.f, t*2.f);
        float g = std::max(0.f, t*2.f - 0.5f);
        float b = 0;
        return {(Uint8)(r*255),(Uint8)(g*255),(Uint8)(b*255),255};
    }
    }
}

static int julia(double zr, double zi, double cr, double ci, int maxIter) {
    for (int i = 0; i < maxIter; ++i) {
        double zr2 = zr*zr, zi2 = zi*zi;
        if (zr2 + zi2 > 4.0) return i;
        double nzr = zr2 - zi2 + cr;
        zi = 2*zr*zi + ci;
        zr = nzr;
    }
    return maxIter;
}

void JuliaPlugin::setup(SDL_Renderer* r) { renderer_ = r; }

void JuliaPlugin::recompute(int w, int h) {
    if (!tex_ || texW_!=w || texH_!=h) {
        if (tex_) SDL_DestroyTexture(tex_);
        tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, h);
        texW_=w; texH_=h;
    }
    void* pixels; int pitch;
    SDL_LockTexture(tex_, nullptr, &pixels, &pitch);
    Uint32* px = (Uint32*)pixels;
    double aspect = double(w)/double(h);
    double scaleX = zoom_*aspect/w, scaleY = zoom_/h;
    for (int y=0; y<h; ++y) {
        double zi = cy_ + (y - h*0.5)*scaleY;
        for (int x=0; x<w; ++x) {
            double zr = cx_ + (x - w*0.5)*scaleX;
            int iter = julia(zr, zi, cx_, cy_, maxIter_);
            // Use fixed c param, not view center
            // Re-do with separate view center (0,0) for display
            zr = viewCx_ + (x - w*0.5)*scaleX;
            zi = viewCy_ + (y - h*0.5)*scaleY;
            iter = julia(zr, zi, cx_, cy_, maxIter_);
            SDL_Color c = juliaColor(iter, maxIter_, colorScheme_);
            px[y*(pitch/4)+x] = ((Uint32)c.r<<24)|((Uint32)c.g<<16)|((Uint32)c.b<<8)|0xFF;
        }
    }
    SDL_UnlockTexture(tex_);
    dirty_ = false;
}

void JuliaPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({280,300}, ImGuiCond_Once);
    ImGui::Begin("Julia Set");

    ImGui::SeparatorText("Julia Parameter c = a + bi");
    double mnC=-2.0, mxC=2.0;
    dirty_ |= ImGui::SliderScalar("a (real)", ImGuiDataType_Double, &cx_, &mnC, &mxC, "%.5f");
    dirty_ |= ImGui::SliderScalar("b (imag)", ImGuiDataType_Double, &cy_, &mnC, &mxC, "%.5f");

    ImGui::SeparatorText("View");
    double mnZ=0.1, mxZ=4.0;
    dirty_ |= ImGui::SliderScalar("zoom", ImGuiDataType_Double, &zoom_, &mnZ, &mxZ,
                                  "%.3f", ImGuiSliderFlags_Logarithmic);
    dirty_ |= ImGui::SliderInt("max iter", &maxIter_, 32, 512);

    ImGui::SeparatorText("Color");
    const char* schemes[]={"Purple-Cyan","Rainbow","Gold"};
    dirty_ |= ImGui::Combo("scheme", &colorScheme_, schemes, 3);

    ImGui::SeparatorText("Presets");
    if (ImGui::Button("Spiral"))     { cx_=-0.7269; cy_=0.1889; dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("Dendrite"))   { cx_=0.0;    cy_=1.0;     dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("Rabbit"))     { cx_=-0.123; cy_=0.745;   dirty_=true; }
    if (ImGui::Button("San Marco"))  { cx_=-0.75;  cy_=0.1;     dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("Siegel disk")){ cx_=-0.391; cy_=-0.587;  dirty_=true; }
    if (ImGui::Button("Reset view")) { viewCx_=0; viewCy_=0; zoom_=3.0; dirty_=true; }

    ImGui::Separator();
    ImGui::Text("c = %.5f + %.5fi", cx_, cy_);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}

void JuliaPlugin::renderScene(const RenderContext& ctx) {
    if (dirty_ || !tex_ || texW_!=ctx.width || texH_!=ctx.height)
        recompute(ctx.width, ctx.height);
    if (tex_) SDL_RenderCopy(ctx.renderer, tex_, nullptr, nullptr);
    sel_.draw(ctx.renderer);
}

void JuliaPlugin::teardown() {
    if (tex_) { SDL_DestroyTexture(tex_); tex_=nullptr; }
}

void JuliaPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if (sel_.isReady()) {
        SDL_Rect r = sel_.consume();
        // Julia view is centered at (0,0), use separate view cx/cy for pan
        applyZoom(r, ctx.width, ctx.height, viewCx_, viewCy_, zoom_);
        dirty_ = true;
    }
}
