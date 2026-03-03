#pragma once
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"
#include <string>

class MayPlugin : public IPlugin {
public:
    std::string name() const override { return "May Map"; }
    void setup(SDL_Renderer* r) override { dirty_ = true; }
    void teardown() override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    void render(SDL_Renderer* r, int w, int h);
    SDL_Texture* tex_ = nullptr;
    int texW_ = 0, texH_ = 0;

    // Initial view from Demo.ini [May] FPARAM=1.0 0.0 4.0 1.0
    int    warmup_ = 200;
    int    plot_   = 200;
    double rMin_ = 1.0, yMin_ = 0.0, rMax_ = 4.0, yMax_ = 1.0;
    bool   dirty_ = true;
    ZoomSelector sel_;
};
