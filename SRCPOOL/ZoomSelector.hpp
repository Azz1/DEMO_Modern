#pragma once
#include <SDL2/SDL.h>

// Rubber-band rectangle zoom selector.
// Call handleEvent() from your plugin's handleEvent().
// Call draw() after renderScene() to overlay the selection rect.
// On release, isReady() returns true — call consume() to get the rect and reset.
struct ZoomSelector {
    bool   active   = false;
    bool   ready    = false;
    int    x0=0, y0=0, x1=0, y1=0;   // screen coords

    void handleEvent(const SDL_Event& e) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            x0 = x1 = e.button.x;
            y0 = y1 = e.button.y;
            active = true; ready = false;
        }
        if (e.type == SDL_MOUSEMOTION && active) {
            x1 = e.motion.x;
            y1 = e.motion.y;
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && active) {
            x1 = e.button.x;
            y1 = e.button.y;
            active = false;
            // Only commit if drag was large enough
            if (abs(x1-x0) > 5 && abs(y1-y0) > 5) ready = true;
        }
    }

    void draw(SDL_Renderer* r) const {
        if (!active) return;
        SDL_Rect rect = {
            std::min(x0,x1), std::min(y0,y1),
            abs(x1-x0),      abs(y1-y0)
        };
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 40);
        SDL_RenderFillRect(r, &rect);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
        SDL_RenderDrawRect(r, &rect);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

    // Returns selected screen rect and resets state
    SDL_Rect consume() {
        SDL_Rect r = { std::min(x0,x1), std::min(y0,y1), abs(x1-x0), abs(y1-y0) };
        ready = false;
        return r;
    }

    bool isReady() const { return ready; }
};

// Convert a screen rect selection to new (cx, cy, zoom) for a 2D fractal view.
inline void applyZoom(SDL_Rect sel, int screenW, int screenH,
                      double& cx, double& cy, double& zoom) {
    double aspect = double(screenW) / double(screenH);
    double scaleX = zoom * aspect / screenW;
    double scaleY = zoom / screenH;

    // Center of selection in complex coords
    double sx = sel.x + sel.w * 0.5;
    double sy = sel.y + sel.h * 0.5;
    double newCx = cx + (sx - screenW*0.5) * scaleX;
    double newCy = cy + (sy - screenH*0.5) * scaleY;

    // New zoom = fraction of old zoom
    double fracX = double(sel.w) / screenW;
    double fracY = double(sel.h) / screenH;
    double frac  = std::max(fracX, fracY);
    double newZoom = zoom * frac;

    cx   = newCx;
    cy   = newCy;
    zoom = std::max(newZoom, 1e-10);
}
