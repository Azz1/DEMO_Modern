#pragma once
#include "IPlugin.hpp"
#include <string>
#include <vector>

struct GNode { char lbl[32]; float weight, x, y; };
struct GEdge { char lbl[32]; int n0, n1; float stress; };

class GraphViewPlugin : public IPlugin {
public:
    std::string name() const override { return "Graph View"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override {}
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    void loadBuiltin();
    bool loadFile(const char* path);
    void parseData();
    void drawArrow(SDL_Renderer* r, int x1, int y1, int x2, int y2, Uint8 R, Uint8 G, Uint8 B);

    std::vector<GNode> nodes_;
    std::vector<GEdge> edges_;
    bool directed_ = true;
    bool showLabels_ = true;
    bool showWeights_ = true;
    float nodeR_ = 18.f;
    float panX_ = 0, panY_ = 0, zoom_ = 1.f;
    bool panning_ = false;
    int  panStartX_ = 0, panStartY_ = 0;
    float panStartOX_ = 0, panStartOY_ = 0;
    char dataBuf_[2048] = "";
    bool dataDirty_ = false;
};
