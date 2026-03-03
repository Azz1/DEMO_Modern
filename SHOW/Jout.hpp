#pragma once
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"
#include <string>

class JoutPlugin : public IPlugin {
public:
    std::string name() const override { return "Julia Out"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    void render(SDL_Renderer* r, int w, int h, int top);
    SDL_Texture* tex_ = nullptr;
    int texW_=0, texH_=0;

    // From Demo.ini [Jout] FPARAM=-2.5 -1.8 2.5 1.8 cx=0.2 cy=0.7
    double csx_=-2.5, csy_=-1.8, cex_=2.5, cey_=1.8;
    double cx_=0.2, cy_=0.7;
    int    count_=30000;
    bool   dirty_=true;
    ZoomSelector sel_;
};
