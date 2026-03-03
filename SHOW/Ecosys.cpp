#include "Ecosys.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <numeric>

// 8-directional offsets: N NE E SE S SW W NW
static const int DX[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };
static const int DY[8] = {-1,-1, 0, 1, 1, 1, 0,-1 };

// ── Helpers ───────────────────────────────────────────────────────────────────

static int wrapX(int x, int W) { return (x + W) % W; }
static int wrapY(int y, int H) { return (y + H) % H; }

// ── IPlugin ───────────────────────────────────────────────────────────────────

void EcosysPlugin::setup(SDL_Renderer* r) {
    renderer_ = r;
    reset();
}

void EcosysPlugin::reset() {
    food_.assign(worldW_ * worldH_, false);
    grazers_.clear();
    popHistory_.clear();
    foodHistory_.clear();
    cycle_ = 0;

    // Seed food
    std::uniform_int_distribution<int> rx(0, worldW_-1), ry(0, worldH_-1);
    for (int i=0; i<4000; ++i) {
        int x=rx(rng_), y=ry(rng_);
        food_[y*worldW_+x] = true;
    }

    // Seed grazers
    std::uniform_int_distribution<int> re(50,100);
    std::uniform_int_distribution<uint8_t> rg(1,10);
    for (int i=0; i<initPop_; ++i) {
        Grazer g;
        g.x=rx(rng_); g.y=ry(rng_);
        g.energy=re(rng_); g.age=0;
        for (int d=0; d<8; ++d) g.moveGenes[d]=rg(rng_);
        g.sense=sense_;
        grazers_.push_back(g);
    }
}

void EcosysPlugin::step() {
    std::uniform_int_distribution<int> rx(0,worldW_-1), ry(0,worldH_-1);
    std::uniform_int_distribution<int> rmut(-1,1);

    // Add new food
    for (int i=0; i<newFood_; ++i) {
        int x=rx(rng_), y=ry(rng_);
        food_[y*worldW_+x] = true;
    }

    std::vector<Grazer> offspring;
    std::vector<bool> dead(grazers_.size(), false);

    for (size_t i=0; i<grazers_.size(); ++i) {
        auto& g = grazers_[i];
        g.age++;
        g.energy--;

        // Die of age or starvation
        if (g.energy <= 0 || g.age > maxAge_) { dead[i]=true; continue; }

        // Sense food nearby — find best direction
        int bestDir = -1;
        double bestScore = -1;
        for (int d=0; d<8; ++d) {
            int nx = wrapX(g.x + DX[d], worldW_);
            int ny = wrapY(g.y + DY[d], worldH_);
            // Check sense radius
            int foodCount = 0;
            for (int dy2=-g.sense; dy2<=g.sense; ++dy2)
            for (int dx2=-g.sense; dx2<=g.sense; ++dx2) {
                int sx=wrapX(nx+dx2,worldW_), sy=wrapY(ny+dy2,worldH_);
                if (food_[sy*worldW_+sx]) ++foodCount;
            }
            double score = g.moveGenes[d] * (1 + foodCount * 2.0);
            if (score > bestScore) { bestScore=score; bestDir=d; }
        }
        if (bestDir < 0) bestDir = 0;

        // Move
        g.x = wrapX(g.x + DX[bestDir], worldW_);
        g.y = wrapY(g.y + DY[bestDir], worldH_);

        // Eat food
        int idx = g.y*worldW_+g.x;
        if (food_[idx]) { food_[idx]=false; g.energy=std::min(g.energy+foodValue_, maxEnergy_); }

        // Reproduce
        if (g.age >= reproAge_ && g.energy >= reproEng_) {
            g.energy -= reproEng_/2;
            Grazer child = g;
            child.age = 0;
            child.energy = reproEng_/2;
            child.x = wrapX(g.x + rx(rng_)%3-1, worldW_);
            child.y = wrapY(g.y + ry(rng_)%3-1, worldH_);
            // Mutate genes
            for (int d=0; d<8; ++d) {
                int v = child.moveGenes[d] + rmut(rng_);
                child.moveGenes[d] = (uint8_t)std::clamp(v, 1, 20);
            }
            offspring.push_back(child);
        }
    }

    // Remove dead
    std::vector<Grazer> alive;
    alive.reserve(grazers_.size());
    for (size_t i=0; i<grazers_.size(); ++i)
        if (!dead[i]) alive.push_back(grazers_[i]);
    grazers_ = std::move(alive);
    for (auto& c : offspring) grazers_.push_back(c);

    // Record history
    int foodCount = std::count(food_.begin(), food_.end(), true);
    popHistory_.push_back((int)grazers_.size());
    foodHistory_.push_back(foodCount / 50);  // scaled
    if ((int)popHistory_.size() > kHistLen) {
        popHistory_.erase(popHistory_.begin());
        foodHistory_.erase(foodHistory_.begin());
    }

    ++cycle_;
}

void EcosysPlugin::drawWorld(SDL_Renderer* r, int screenW, int screenH) {
    // Recreate texture if needed
    if (!worldTex_ || texW_!=worldW_ || texH_!=worldH_) {
        if (worldTex_) SDL_DestroyTexture(worldTex_);
        worldTex_ = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_STREAMING, worldW_, worldH_);
        texW_=worldW_; texH_=worldH_;
    }

    void* pixels; int pitch;
    SDL_LockTexture(worldTex_, nullptr, &pixels, &pitch);
    Uint32* px=(Uint32*)pixels;

    // Background
    for (int i=0; i<worldW_*worldH_; ++i) px[i] = 0x0D1A0AFF; // dark green

    // Food (bright green)
    for (int y=0; y<worldH_; ++y)
    for (int x=0; x<worldW_; ++x)
        if (food_[y*worldW_+x] && x%2==0 && y%2==0) px[y*(pitch/4)+x] = 0x00FF44FF;

    SDL_UnlockTexture(worldTex_);

    // Scale world to screen (leave bottom 130px for graph)
    int dispH = screenH - 130;
    SDL_Rect dst = {0, 0, screenW, dispH};
    SDL_RenderCopy(r, worldTex_, nullptr, &dst);

    // Draw grazers — color encodes fitness (gene specialization)
    // low  fitness → blue/cyan  (generalist, less evolved)
    // mid  fitness → yellow     (moderate)
    // high fitness → red/orange (highly specialized / evolved)
    float scaleX = float(screenW) / worldW_;
    float scaleY = float(dispH)   / worldH_;
    int   dotW   = std::max(4, int(scaleX * 1.8f));
    int   dotH   = std::max(4, int(scaleY * 1.8f));

    for (auto& g : grazers_) {
        // Fitness: how much one direction dominates (max*8 / sum)
        int sum = 0, mx = 0;
        for (int d=0;d<8;d++) { sum += g.moveGenes[d]; mx = std::max(mx,(int)g.moveGenes[d]); }
        float fit = sum>0 ? float(mx*8)/sum : 0.f;
        fit = std::clamp((fit-1.0f)/6.0f, 0.f, 1.f);  // remap 1..7 -> 0..1

        // Color ramp: blue -> cyan -> yellow -> red
        Uint8 cr, cg, cb;
        if (fit < 0.33f) {
            float t = fit/0.33f;
            cr=Uint8(200+55*t); cg=Uint8(200-t*20); cb=0;   // yellow
        } else if (fit < 0.66f) {
            float t = (fit-0.33f)/0.33f;
            cr=Uint8(255*(1-t)); cg=Uint8(180*t); cb=Uint8(200+55*t);  // -> blue/cyan
        } else {
            float t = (fit-0.66f)/0.34f;
            cr=255; cg=Uint8(200*(1-t)); cb=0;               // red
        }

        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_Rect dot = {
            int(g.x*scaleX) - dotW/2,
            int(g.y*scaleY) - dotH/2,
            dotW, dotH
        };
        SDL_RenderFillRect(r, &dot);
    }
}

void EcosysPlugin::drawPopGraph(SDL_Renderer* r, int gx, int gy, int gw, int gh) {
    // Background
    SDL_Rect bg = {gx, gy, gw, gh};
    SDL_SetRenderDrawColor(r, 20, 20, 30, 200);
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 60, 60, 80, 255);
    SDL_RenderDrawRect(r, &bg);

    if (popHistory_.empty()) return;
    int maxPop = *std::max_element(popHistory_.begin(), popHistory_.end());
    int maxFood = *std::max_element(foodHistory_.begin(), foodHistory_.end());
    int maxVal = std::max({maxPop, maxFood, 1});

    int n = (int)popHistory_.size();
    float dx = float(gw) / kHistLen;

    // Food (green line)
    SDL_SetRenderDrawColor(r, 0, 200, 80, 255);
    for (int i=1; i<n; ++i) {
        int x0 = gx + int((i-1)*dx);
        int y0 = gy + gh - gh * foodHistory_[i-1] / maxVal;
        int x1 = gx + int(i*dx);
        int y1 = gy + gh - gh * foodHistory_[i] / maxVal;
        SDL_RenderDrawLine(r, x0, y0, x1, y1);
    }
    // Population (yellow line)
    SDL_SetRenderDrawColor(r, 255, 200, 50, 255);
    for (int i=1; i<n; ++i) {
        int x0 = gx + int((i-1)*dx);
        int y0 = gy + gh - gh * popHistory_[i-1] / maxVal;
        int x1 = gx + int(i*dx);
        int y1 = gy + gh - gh * popHistory_[i] / maxVal;
        SDL_RenderDrawLine(r, x0, y0, x1, y1);
    }
}

void EcosysPlugin::renderScene(const RenderContext& ctx) {
    if (running_)
        for (int i=0; i<stepsPerFrame_; ++i) {
            step();
            if (grazers_.empty()) { running_=false; break; }
        }

    drawWorld(ctx.renderer, ctx.width, ctx.height);
    drawPopGraph(ctx.renderer, 0, ctx.height-125, ctx.width, 120);
}

void EcosysPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({280, 320}, ImGuiCond_Once);
    ImGui::Begin("Ecosystem Simulator");

    ImGui::SeparatorText("Controls");
    if (ImGui::Button(running_ ? "⏸ Pause" : "▶ Run")) running_ = !running_;
    ImGui::SameLine();
    if (ImGui::Button("↺ Reset")) { running_=false; reset(); }
    ImGui::SliderInt("speed", &stepsPerFrame_, 1, 20);

    ImGui::SeparatorText("Parameters");
    ImGui::SliderInt("init pop",    &initPop_,   10, 200);
    ImGui::SliderInt("new food/cy", &newFood_,    0, 200);
    ImGui::SliderInt("food value",  &foodValue_,  5, 80);
    ImGui::SliderInt("max energy",  &maxEnergy_, 50, 500);
    ImGui::SliderInt("max age",     &maxAge_,    50, 1000);
    ImGui::SliderInt("repro age",   &reproAge_,  10, 300);
    ImGui::SliderInt("repro eng",   &reproEng_,  20, 300);
    ImGui::SliderInt("sense",       &sense_,      1, 10);

    ImGui::SeparatorText("Stats");
    ImGui::TextColored({1.0f,0.8f,0.2f,1.0f}, "🟡 Grazers: %d", (int)grazers_.size());
    int foodCnt = std::count(food_.begin(), food_.end(), true);
    ImGui::TextColored({0.0f,1.0f,0.3f,1.0f}, "🟢 Food:    %d", foodCnt);
    ImGui::Text("Cycle: %d", cycle_);

    ImGui::Separator();
    ImGui::TextDisabled("Graph: 🟡 population  🟢 food");
    ImGui::End();
}

void EcosysPlugin::teardown() {
    if (worldTex_) { SDL_DestroyTexture(worldTex_); worldTex_=nullptr; }
}
