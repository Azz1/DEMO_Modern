#include "Lorenz.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

void LorenzPlugin::setup(SDL_Renderer*) { recompute(); }

void LorenzPlugin::recompute() {
    pts_.clear();
    pts_.reserve(steps_);
    float x = 0.1f, y = 0.0f, z = 0.0f;
    for (int i = 0; i < steps_; ++i) {
        float dx = sigma_ * (y - x);
        float dy = x * (rho_ - z) - y;
        float dz = x * y - beta_ * z;
        x += dx * dt_; y += dy * dt_; z += dz * dt_;
        pts_.push_back({x, y, z, float(i) / float(steps_ - 1)});
    }
    dirty_ = false;
}

void LorenzPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({270, 260}, ImGuiCond_Once);
    ImGui::Begin("Lorenz Attractor");
    ImGui::SeparatorText("Parameters");
    dirty_ |= ImGui::SliderFloat("sigma", &sigma_, 1.f, 30.f);
    dirty_ |= ImGui::SliderFloat("rho",   &rho_,   1.f, 60.f);
    dirty_ |= ImGui::SliderFloat("beta",  &beta_,  0.1f, 5.f);
    dirty_ |= ImGui::SliderInt ("steps",  &steps_, 1000, 100000);
    ImGui::SeparatorText("View");
    ImGui::SliderFloat("rot X", &rotX_, -180.f, 180.f);
    ImGui::SliderFloat("rot Y", &rotY_, -180.f, 180.f);
    ImGui::SliderFloat("zoom",  &zoom_,   1.f,  20.f);
    if (dirty_) recompute();
    ImGui::Separator();
    ImGui::Text("Points: %d | FPS: %.1f", (int)pts_.size(), ImGui::GetIO().Framerate);
    ImGui::End();
}

void LorenzPlugin::renderScene(const RenderContext& ctx) {
    float cx = ctx.width  * 0.5f;
    float cy = ctx.height * 0.5f;
    float ry = rotY_ * 3.14159f / 180.f;
    float rx = rotX_ * 3.14159f / 180.f;
    float cosY = cosf(ry), sinY = sinf(ry);
    float cosX = cosf(rx), sinX = sinf(rx);

    for (size_t i = 1; i < pts_.size(); ++i) {
        auto project = [&](const Point3& p) -> SDL_FPoint {
            // Rotate Y then X
            float x1 =  p.x * cosY + p.z * sinY;
            float y1 =  p.x * sinX * (-sinY) + p.y * cosX + p.z * sinX * cosY;
            return { cx + x1 * zoom_, cy - y1 * zoom_ };
        };

        SDL_FPoint a = project(pts_[i-1]);
        SDL_FPoint b = project(pts_[i]);

        // Color: gradient from blue to red
        float t = pts_[i].t;
        Uint8 r = (Uint8)(50  + t * 200);
        Uint8 g = (Uint8)(100 - t * 50);
        Uint8 bl= (Uint8)(255 - t * 200);
        SDL_SetRenderDrawColor(ctx.renderer, r, g, bl, 180);
        SDL_RenderDrawLineF(ctx.renderer, a.x, a.y, b.x, b.y);
    }
}

void LorenzPlugin::handleEvent(const SDL_Event& e, const RenderContext&) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT) {
        drag_=true; dragSX_=e.button.x; dragSY_=e.button.y;
        dragBaseX_=rotX_; dragBaseY_=rotY_;
    }
    if (e.type==SDL_MOUSEBUTTONUP&&e.button.button==SDL_BUTTON_LEFT) drag_=false;
    if (e.type==SDL_MOUSEMOTION&&drag_) {
        rotX_ = dragBaseX_ + (e.motion.y - dragSY_) * 0.4f;
        rotY_ = dragBaseY_ + (e.motion.x - dragSX_) * 0.4f;
    }
    if (e.type==SDL_MOUSEWHEEL)
        zoom_ = std::max(1.f, zoom_ + e.wheel.y * 0.5f);
}

void LorenzPlugin::teardown() {}
