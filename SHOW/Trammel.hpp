#pragma once
#include "IPlugin.hpp"
#include "ZoomSelector.hpp"
#include <string>

class TrammelPlugin : public IPlugin {
public:
    std::string name() const override { return "Trammel"; }
    void setup(SDL_Renderer* r) override {}
    void teardown() override {}
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    void draw(SDL_Renderer* r, int w, int h, int topOff=0);

    // From Demo.ini [Trammel] IPARAM=1440 2 48
    int    n_    = 1440;
    int    s_    = 2;
    int    k_    = 48;
    double scale_ = 3.0;   // zoom: 1.0 = default fit
    double offX_  = 0.0;   // pan center offset (normalized -1..1)
    double offY_  = 0.0;
    ZoomSelector sel_;
};
