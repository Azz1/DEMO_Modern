#pragma once
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"
#include <string>

class HenonPlugin : public IPlugin {
public:
    std::string name() const override { return "Henon Map"; }
    void setup(SDL_Renderer* r) override { dirty_ = true; }
    void teardown() override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    void render(SDL_Renderer* r, int w, int h);
    SDL_Texture* tex_ = nullptr;
    int texW_ = 0, texH_ = 0;

    // Initial view from Demo.ini [Henon] FPARAM=-2.0 -2.4 3.8 2.4
    int    iters_ = 30;
    double csx_ = -2.0, csy_ = -2.4, cex_ = 3.8, cey_ = 2.4;
    bool   dirty_ = true;
    ZoomSelector sel_;
};
