#include "Henon.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

void HenonPlugin::teardown() {
    if (tex_) { SDL_DestroyTexture(tex_); tex_ = nullptr; }
}

void HenonPlugin::render(SDL_Renderer* r, int w, int h) {
    if (!tex_ || texW_ != w || texH_ != h) {
        if (tex_) SDL_DestroyTexture(tex_);
        tex_ = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, h);
        texW_ = w; texH_ = h;
    }
    void* px; int pitch;
    SDL_LockTexture(tex_, nullptr, &px, &pitch);
    Uint32* p = (Uint32*)px;
    for (int i = 0; i < w * h; ++i) p[i] = 0x000000FF;

    double rx = w / (cex_ - csx_);
    double ry = h / (cey_ - csy_);
    for (int pi = 0; pi < w; ++pi)
    for (int pj = 0; pj < h; ++pj) {
        double a = csx_ + pi / rx;
        double b = csy_ + pj / ry;
        double x0 = 1.0, y0 = 1.0;
        Uint32 col = 0x000000FF;
        for (int k = 0; k <= iters_; ++k) {
            double x = 1.0 - a * x0 * x0 + y0;
            double y = b * x0;
            if (std::fabs(x) >= 3.0 || std::fabs(y) >= 3.0) {
                int rv=(k*(k+2))%256, gv=(k*k)%256, bv=(k*(iters_-k))%256;
                col = (rv<<24)|(gv<<16)|(bv<<8)|0xFF;
                break;
            }
            x0 = x; y0 = y;
        }
        p[pj*(pitch/4)+pi] = col;
    }
    SDL_UnlockTexture(tex_);
    dirty_ = false;
}

void HenonPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if (sel_.isReady()) {
        SDL_Rect s = sel_.consume();
        double rx = (cex_-csx_) / ctx.width;
        double ry = (cey_-csy_) / ctx.height;
        double nx0 = csx_ + s.x * rx;
        double ny0 = csy_ + s.y * ry;
        double nx1 = csx_ + (s.x+s.w) * rx;
        double ny1 = csy_ + (s.y+s.h) * ry;
        csx_=nx0; csy_=ny0; cex_=nx1; cey_=ny1;
        dirty_ = true;
    }
}

void HenonPlugin::renderScene(const RenderContext& ctx) {
    if (dirty_) render(ctx.renderer, ctx.width, ctx.height);
    if (tex_) { SDL_Rect dst={0, ctx.topOffset, ctx.width, ctx.height - ctx.topOffset}; SDL_RenderCopy(ctx.renderer,tex_,nullptr,&dst); }
    sel_.draw(ctx.renderer);
}

void HenonPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({265, 235}, ImGuiCond_Once);
    ImGui::Begin("Henon Map");
    ImGui::SeparatorText("Iterations");
    if (ImGui::SliderInt("iters", &iters_, 10, 500)) dirty_=true;
    ImGui::SeparatorText("View");
    if (ImGui::DragFloat("x min",(float*)&csx_,0.01f)) dirty_=true;
    if (ImGui::DragFloat("x max",(float*)&cex_,0.01f)) dirty_=true;
    if (ImGui::DragFloat("y min",(float*)&csy_,0.01f)) dirty_=true;
    if (ImGui::DragFloat("y max",(float*)&cey_,0.01f)) dirty_=true;
    if (ImGui::Button("Reset")) { csx_=-2.0;csy_=-2.4;cex_=3.8;cey_=2.4;iters_=30;dirty_=true; }
    ImGui::SeparatorText("Info");
    ImGui::TextDisabled("Drag to zoom in");
    ImGui::TextDisabled("x'=1-ax²+y  y'=bx");
    ImGui::End();
}
