#include "Trammel.hpp"
#include <imgui.h>
#include <cmath>
#include <SDL2/SDL.h>

static const double kPI2 = 6.28318530718;

void TrammelPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if (sel_.isReady()) {
        SDL_Rect s = sel_.consume();
        // Zoom: shrink scale by fraction of screen selected
        double fx = double(s.w) / ctx.width;
        double fy = double(s.h) / ctx.height;
        double frac = std::max(fx, fy);
        // Pan: selected center relative to screen center, in normalized coords
        double dx = (s.x + s.w*0.5 - ctx.width*0.5)  / (ctx.width*0.5);
        double dy = (s.y + s.h*0.5 - ctx.height*0.5) / (ctx.height*0.5);
        offX_ += dx * scale_;
        offY_ += dy * scale_;
        scale_ *= frac;
    }
}

void TrammelPlugin::draw(SDL_Renderer* r, int w, int h, int topOff) {
    double cx = w * 0.5;
    double cy = h * 0.5 + topOff * 0.5;
    double radius = std::min(w, h) * 0.42 / scale_;
    double R  = radius / 5.0;
    double dt = kPI2 / n_;

    // Apply pan offset in pixels
    double pcx = cx - offX_ * w * 0.5;
    double pcy = cy - offY_ * h * 0.5;

    double t = 0.0, ax = 0.0, ay = 0.0;
    double prevX = 0.0, prevY = 0.0;

    for (int i = 0; i <= n_; ++i) {
        double x = R * (std::cos(t) - std::cos(s_ * t));
        double y = R * (std::sin(t) + std::sin(s_ * t));
        for (int j = 0; j < k_; ++j) {
            double x1 =  x*std::cos(t) + y*std::sin(t);
            double y1 = -x*std::sin(t) + y*std::cos(t) + 2.0*R*std::sin(t);
            x = x1; y = y1;
        }
        ax = x; ay = y;

        float hue = float(i) / n_;
        float h6 = hue * 6.f;
        int   hi = int(h6) % 6;
        float f  = h6 - int(h6);
        Uint8 cr, cg, cb;
        switch (hi) {
            case 0: cr=255; cg=Uint8(255*f); cb=0; break;
            case 1: cr=Uint8(255*(1-f)); cg=255; cb=0; break;
            case 2: cr=0; cg=255; cb=Uint8(255*f); break;
            case 3: cr=0; cg=Uint8(255*(1-f)); cb=255; break;
            case 4: cr=Uint8(255*f); cg=0; cb=255; break;
            default:cr=255; cg=0; cb=Uint8(255*(1-f)); break;
        }
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);

        int px = int(pcx + ax * radius / R);
        int py = int(pcy - ay * radius / R);
        if (i > 0) {
            int ppx = int(pcx + prevX * radius / R);
            int ppy = int(pcy - prevY * radius / R);
            SDL_RenderDrawLine(r, ppx, ppy, px, py);
        }
        prevX = ax; prevY = ay;
        t += dt;
    }
}

void TrammelPlugin::renderScene(const RenderContext& ctx) {
    SDL_SetRenderDrawColor(ctx.renderer, 10, 10, 20, 255);
    SDL_RenderClear(ctx.renderer);
    draw(ctx.renderer, ctx.width, ctx.height, ctx.topOffset);
    sel_.draw(ctx.renderer);
}

void TrammelPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({255, 225}, ImGuiCond_Once);
    ImGui::Begin("Trammel");
    ImGui::SeparatorText("Parameters");
    ImGui::SliderInt("steps (n)", &n_, 10, 10800);
    ImGui::SliderInt("freq  (s)", &s_,  1, 90);
    ImGui::SliderInt("rotate(k)", &k_,  0, 300);
    if (ImGui::Button("Reset")) { n_=1440; s_=2; k_=48; scale_=3.0; offX_=0; offY_=0; }
    ImGui::SeparatorText("Presets");
    if (ImGui::Button("Flower"))  { n_=720;  s_=5;  k_=2;  scale_=3.0; offX_=offY_=0; }
    ImGui::SameLine();
    if (ImGui::Button("Star"))    { n_=360;  s_=7;  k_=0;  scale_=3.0; offX_=offY_=0; }
    ImGui::SameLine();
    if (ImGui::Button("Spiral"))  { n_=1800; s_=11; k_=3;  scale_=3.0; offX_=offY_=0; }
    ImGui::SeparatorText("Info");
    ImGui::TextDisabled("Drag to zoom in");
    ImGui::TextDisabled("x=r(cos t-cos st)");
    ImGui::TextDisabled("y=r(sin t+sin st)");
    ImGui::End();
}
