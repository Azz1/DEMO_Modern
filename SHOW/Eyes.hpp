#pragma once
#include "IPlugin.hpp"
#include <string>

class EyesPlugin : public IPlugin {
public:
    std::string name() const override { return "Eyes"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override {}
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override {}

private:
    void fillEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry,
                     Uint8 R, Uint8 G, Uint8 B);
    void outlineEllipse(SDL_Renderer* r, int cx, int cy, int rx, int ry,
                        int thick, Uint8 R, Uint8 G, Uint8 B);
    void getPupil(int dx, int dy, int a, int b, int* xe, int* ye);

    int bgR_=28, bgG_=28, bgB_=38;
    int numEyes_ = 2;   // 1 or 2
};
