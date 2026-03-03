#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"

class MandelbrotPlugin : public IPlugin {
public:
    std::string name() const override { return "Mandelbrot Set"; }
    void setup(SDL_Renderer* renderer) override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;
    void teardown() override;

private:
    void recompute(int w, int h);

    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture*  tex_      = nullptr;
    int texW_ = 0, texH_ = 0;

    // View
    double cx_  = -0.5,  cy_  = 0.0;
    double zoom_ = 3.0;   // total width in complex plane
    int    maxIter_ = 256;
    bool          dirty_ = true;
    ZoomSelector  sel_;

    // Color scheme
    int colorScheme_ = 0; // 0=classic, 1=fire, 2=grayscale
};
