#pragma once
#include <SDL2/SDL.h>
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"

class JuliaPlugin : public IPlugin {
public:
    std::string name() const override { return "Julia Set"; }
    void setup(SDL_Renderer* r) override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;
    void teardown() override;

private:
    void recompute(int w, int h);

    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture*  tex_      = nullptr;
    int texW_ = 0, texH_ = 0;

    double cx_ = -0.7,  cy_ = 0.27015;  // Julia parameter c
    double viewCx_ = 0.0, viewCy_ = 0.0;  // view pan
    double zoom_ = 3.0;
    int    maxIter_ = 256;
    int    colorScheme_ = 0;
    bool   animate_ = false;
    float  animAngle_ = 0.f;
    bool          dirty_ = true;
    ZoomSelector  sel_;
};
