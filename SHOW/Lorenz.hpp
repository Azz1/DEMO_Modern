#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include "IPlugin.hpp"

class LorenzPlugin : public IPlugin {
public:
    std::string name() const override { return "Lorenz Attractor"; }
    void setup(SDL_Renderer* renderer) override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;
    void teardown() override;

private:
    void recompute();

    float sigma_ = 10.0f, rho_ = 28.0f, beta_ = 8.0f/3.0f;
    float dt_    = 0.005f;
    int   steps_ = 30000;

    float rotX_ = 20.0f, rotY_ = 0.0f, zoom_ = 8.0f;

    bool  drag_ = false;
    int   dragSX_ = 0, dragSY_ = 0;
    float dragBaseX_ = 20.f, dragBaseY_ = 0.f;

    struct Point3 { float x, y, z, t; };
    std::vector<Point3> pts_;
    bool dirty_ = true;
};
