#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <random>
#include "IPlugin.hpp"

// ── Grazer ──────────────────────────────────────────────────────────────────
struct Grazer {
    int x, y;
    int energy;
    int age;
    uint8_t moveGenes[8];   // weighted directional bias (N NE E SE S SW W NW)
    uint8_t sense;          // food sense radius
};

// ── EcosysPlugin ─────────────────────────────────────────────────────────────
class EcosysPlugin : public IPlugin {
public:
    std::string name() const override { return "Ecosystem"; }
    void setup(SDL_Renderer* r) override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override {}
    void teardown() override;

private:
    void reset();
    void step();
    void drawWorld(SDL_Renderer* r, int screenW, int screenH);
    void drawPopGraph(SDL_Renderer* r, int x, int y, int w, int h);

    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture*  worldTex_ = nullptr;
    int texW_ = 0, texH_ = 0;

    // World
    int worldW_ = 400, worldH_ = 300;
    std::vector<bool>   food_;
    std::vector<Grazer> grazers_;

    // Parameters
    int  initPop_   = 50;
    int  newFood_   = 60;
    int  maxEnergy_ = 200;
    int  maxAge_    = 300;
    int  reproAge_  = 80;
    int  reproEng_  = 100;
    int  foodValue_ = 30;
    int  sense_     = 3;

    // Sim state
    int  cycle_     = 0;
    bool running_   = false;
    int  stepsPerFrame_ = 1;

    // Population history
    std::vector<int> popHistory_;
    std::vector<int> foodHistory_;
    static constexpr int kHistLen = 400;

    std::mt19937 rng_{std::random_device{}()};
};
