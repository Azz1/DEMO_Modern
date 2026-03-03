#pragma once
#include "IPlugin.hpp"
#include <string>
#include <vector>

struct LsysPreset {
        const char* name;
        const char* axiom;
        const char* ruleF;   // F -> ?
        const char* ruleX;   // X -> ? (optional)
        const char* ruleY;   // Y -> ? (optional)
        int  levels;
        float angle;  // degrees
        float startAngle;
};

class LsysPlugin : public IPlugin {
public:
    std::string name() const override { return "L-System"; }
    void setup(SDL_Renderer* r) override {}
    void teardown() override {}
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override {}

private:
    void   generate();
    void   drawTurtle(SDL_Renderer* r, int w, int h, int top);
    std::string expand(const std::string& s, int levels);

    int   preset_  = 0;
    int   levels_  = 4;
    float angle_   = 60.f;
    float startAngle_ = 0.f;
    char  axiomBuf_[64]  = "F--F--F";
    char  ruleFBuf_[128] = "F+F--F+F";
    char  ruleXBuf_[128] = "";
    char  ruleYBuf_[128] = "";
    bool  dirty_ = true;
    std::string turtle_;  // expanded string
};
