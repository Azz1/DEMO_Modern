#include "May.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

void MayPlugin::teardown() {
    if (tex_) { SDL_DestroyTexture(tex_); tex_ = nullptr; }
}

void MayPlugin::render(SDL_Renderer* r, int w, int h) {
    if (!tex_ || texW_ != w || texH_ != h) {
        if (tex_) SDL_DestroyTexture(tex_);
        tex_ = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, h);
        texW_ = w; texH_ = h;
    }
    void* px; int pitch;
    SDL_LockTexture(tex_, nullptr, &px, &pitch);
    Uint32* p = (Uint32*)px;
    for (int i = 0; i < w*h; ++i) p[i] = 0x0A0A14FF;

    double ry = h / (yMax_ - yMin_);
    for (int col = 0; col < w; ++col) {
        double r2 = rMin_ + (rMax_-rMin_) * col / w;
        double v  = 0.2;
        for (int j=0; j<warmup_; ++j) { v = r2*v*(1.0-v); if(v>1||v<0) break; }
        for (int j=0; j<plot_; ++j) {
            v = r2*v*(1.0-v);
            if (v>1.0||v<0.0) break;
            if (v>=yMin_ && v<=yMax_) {
                int row = h-1-(int)((v-yMin_)*ry);
                if (row>=0 && row<h) p[row*(pitch/4)+col] = 0x00FF88FF;
            }
        }
    }
    SDL_UnlockTexture(tex_);
    dirty_ = false;
}

void MayPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if (sel_.isReady()) {
        SDL_Rect s = sel_.consume();
        double rx = (rMax_-rMin_) / ctx.width;
        double ry = (yMax_-yMin_) / ctx.height;
        double nr0 = rMin_ + s.x * rx;
        double ny0 = yMax_ - (s.y+s.h) * ry;   // y is flipped
        double nr1 = rMin_ + (s.x+s.w) * rx;
        double ny1 = yMax_ - s.y * ry;
        rMin_=nr0; yMin_=ny0; rMax_=nr1; yMax_=ny1;
        dirty_ = true;
    }
}

void MayPlugin::renderScene(const RenderContext& ctx) {
    if (dirty_) render(ctx.renderer, ctx.width, ctx.height);
    if (tex_) { SDL_Rect dst={0, ctx.topOffset, ctx.width, ctx.height - ctx.topOffset}; SDL_RenderCopy(ctx.renderer,tex_,nullptr,&dst); }
    sel_.draw(ctx.renderer);
}

void MayPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({265, 240}, ImGuiCond_Once);
    ImGui::Begin("May Map (Logistic)");
    ImGui::SeparatorText("Iterations");
    if (ImGui::SliderInt("warmup",&warmup_,50,6000)) dirty_=true;
    if (ImGui::SliderInt("plot",  &plot_,  50,1000)) dirty_=true;
    ImGui::SeparatorText("View");
    if (ImGui::DragFloat("r min",(float*)&rMin_,0.01f,0.f,4.f)) dirty_=true;
    if (ImGui::DragFloat("r max",(float*)&rMax_,0.01f,0.f,4.f)) dirty_=true;
    if (ImGui::DragFloat("y min",(float*)&yMin_,0.005f,0.f,1.f)) dirty_=true;
    if (ImGui::DragFloat("y max",(float*)&yMax_,0.005f,0.f,1.f)) dirty_=true;
    if (ImGui::Button("Reset")) { rMin_=1.0;yMin_=0.0;rMax_=4.0;yMax_=1.0;warmup_=200;plot_=200;dirty_=true; }
    ImGui::SeparatorText("Info");
    ImGui::TextDisabled("Drag to zoom in");
    ImGui::TextDisabled("x'=r·x·(1-x)");
    ImGui::End();
}
