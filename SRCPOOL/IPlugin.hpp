#pragma once
#include <string>
#include <SDL2/SDL.h>

struct RenderContext {
    SDL_Renderer* renderer;
    int width;
    int height;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string name() const = 0;
    virtual void setup(SDL_Renderer* renderer) = 0;
    virtual void renderUI() = 0;
    virtual void renderScene(const RenderContext& ctx) = 0;
    virtual void handleEvent(const SDL_Event& e, const RenderContext& ctx) {}
    virtual void teardown() = 0;
};
