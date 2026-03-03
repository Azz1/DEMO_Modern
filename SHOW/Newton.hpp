#pragma once
#include <SDL2/SDL.h>
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"

class NewtonPlugin : public IPlugin {
public:
    std::string name() const override { return "Newton Fractal"; }
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

    double cx_ = 0.0, cy_ = 0.0;
    double zoom_ = 3.0;
    int    maxIter_ = 64;
    int    polyDeg_ = 3;   // degree: z^n - 1 = 0, roots = nth roots of unity
    float  damping_ = 1.0f;
    bool   showShading_ = true;
    bool          dirty_ = true;
    ZoomSelector  sel_;
};
