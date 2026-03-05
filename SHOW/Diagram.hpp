#pragma once
#include "IPlugin.hpp"
#include <string>
#include <vector>
#include <array>

class DiagramPlugin : public IPlugin {
public:
    std::string name() const override { return "Diagram"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override {}
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override {}

private:
    using fPoint = std::array<float,2>;
    using fCurve = std::vector<fPoint>;

    void drawGrid(SDL_Renderer* r, int W, int H, int top);
    void drawDline (SDL_Renderer* r, const fCurve& p, int W, int H, int top, Uint8 R, Uint8 G, Uint8 B);
    void drawScurve(SDL_Renderer* r, const fCurve& p, int W, int H, int top, Uint8 R, Uint8 G, Uint8 B);
    void drawFcurve(SDL_Renderer* r, const fCurve& p, int W, int H, int top, Uint8 R, Uint8 G, Uint8 B);
    void computeSplineDerivs(const fCurve& p, std::vector<fPoint>& dp, std::vector<float>& L);
    fPoint toScreen(float x, float y, int W, int H, int top) const;
    void buildBuiltin();
    void parseInput();

    // View
    float csx_=-1.f, csy_=-1.f, cex_=5.f, cey_=5.f;
    bool  grid_=true;
    int   curveMode_=1;  // 0=line 1=spline 2=fractal
    float fracD_=0.2f;
    bool  showPoints_=true;

    // Data
    std::vector<fCurve> curves_;
    static const int kColors = 8;
    static constexpr Uint8 kColorMap[8][3] = {
        {255,255,0},{255,80,80},{80,255,80},{255,80,255},
        {80,255,255},{155,100,200},{80,200,130},{255,255,255}
    };

    // Text editor
    char  inputBuf_[2048] = "";
    bool  inputDirty_=false;
    // Built-in presets
    int   preset_=0;
};
