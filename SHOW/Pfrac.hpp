#pragma once
#include "IPlugin.hpp"
#include <string>
#include <vector>

class PfracPlugin : public IPlugin {
public:
    std::string name() const override { return "IFS Fractal"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override {}

private:
    void render(SDL_Renderer* r, int w, int h, int top);
    SDL_Texture* tex_ = nullptr;
    int texW_=0, texH_=0;

    struct IFSTransform { float a,b,c,d,e,f; int prob; };
    struct Preset { const char* name; std::vector<IFSTransform> tr;
                    float x0,y0,x1,y1; };
    std::vector<Preset> presets_;

    int   preset_ = 0;
    int   count_  = 50000;
    bool  dirty_  = true;
};
