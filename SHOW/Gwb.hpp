#pragma once
#include "IPlugin.hpp"
#include <string>

class GwbPlugin : public IPlugin {
public:
    std::string name() const override { return "GWB Solids"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    void buildScene(int sceneIdx);

    // 3D view params (same semantics as Geograph)
    float theta_  = 60.f;
    float rz_     = 30.f;
    float radiu_  = 400.f;
    float depF_   = 300.f;
    float ox_=0,oy_=0,oz_=0;

    int   scene_    = 0;
    int   drawMode_ = 1;  // 0=3D Line, 1=3D Solid
    bool  pan_    = false;
    int   panSX_  = 0, panSY_ = 0;
    float panBaseTheta_=60, panBaseRz_=30;

    bool  built_  = false;
};
