#include "Newton.hpp"
#include <imgui.h>
#include <cmath>
#include <complex>
#include <vector>

// Roots of z^n - 1 = 0 (nth roots of unity)
static std::vector<std::complex<double>> computeRoots(int n) {
    std::vector<std::complex<double>> roots(n);
    for (int k=0; k<n; ++k)
        roots[k] = std::polar(1.0, 2.0*M_PI*k/n);
    return roots;
}

// Color palette per root
static SDL_Color rootColor(int rootIdx, int n, int iter, int maxIter, bool shading) {
    static const SDL_Color palette[8] = {
        {255, 80,  80,  255},  // red
        {80,  180, 255, 255},  // blue
        {80,  255, 120, 255},  // green
        {255, 220, 60,  255},  // yellow
        {200, 80,  255, 255},  // purple
        {255, 150, 60,  255},  // orange
        {60,  230, 220, 255},  // cyan
        {255, 100, 180, 255},  // pink
    };
    SDL_Color c = palette[rootIdx % 8];
    if (shading) {
        float shade = 1.0f - float(iter) / float(maxIter) * 0.7f;
        c.r = (Uint8)(c.r * shade);
        c.g = (Uint8)(c.g * shade);
        c.b = (Uint8)(c.b * shade);
    }
    return c;
}

// Newton's method: z^n - 1, derivative: n*z^(n-1)
static std::pair<int,int> newton(std::complex<double> z, int n,
                                  const std::vector<std::complex<double>>& roots,
                                  int maxIter, double damping) {
    for (int i=0; i<maxIter; ++i) {
        std::complex<double> zn  = std::pow(z, n);
        std::complex<double> dzn = double(n) * std::pow(z, n-1);
        if (std::abs(dzn) < 1e-12) break;
        z -= damping * (zn - 1.0) / dzn;

        // Find nearest root
        int nearest = 0;
        double minDist = std::abs(z - roots[0]);
        for (int k=1; k<n; ++k) {
            double d = std::abs(z - roots[k]);
            if (d < minDist) { minDist=d; nearest=k; }
        }
        if (minDist < 1e-6) return {nearest, i};
    }
    return {-1, maxIter};
}

void NewtonPlugin::setup(SDL_Renderer* r) { renderer_=r; }

void NewtonPlugin::recompute(int w, int h) {
    if (!tex_ || texW_!=w || texH_!=h) {
        if (tex_) SDL_DestroyTexture(tex_);
        tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, h);
        texW_=w; texH_=h;
    }
    auto roots = computeRoots(polyDeg_);
    void* pixels; int pitch;
    SDL_LockTexture(tex_, nullptr, &pixels, &pitch);
    Uint32* px=(Uint32*)pixels;
    double aspect=double(w)/double(h);
    double scaleX=zoom_*aspect/w, scaleY=zoom_/h;

    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            double zr = cx_ + (x - w*0.5)*scaleX;
            double zi = cy_ + (y - h*0.5)*scaleY;
            auto [root, iter] = newton({zr,zi}, polyDeg_, roots, maxIter_, damping_);
            SDL_Color c;
            if (root < 0) c = {10,10,10,255};
            else          c = rootColor(root, polyDeg_, iter, maxIter_, showShading_);
            px[y*(pitch/4)+x]=((Uint32)c.r<<24)|((Uint32)c.g<<16)|((Uint32)c.b<<8)|0xFF;
        }
    }
    SDL_UnlockTexture(tex_);
    dirty_=false;
}

void NewtonPlugin::renderUI() {
    ImGui::SetNextWindowPos({10, 35}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({270,250}, ImGuiCond_Once);
    ImGui::Begin("Newton Fractal  z^n - 1 = 0");

    ImGui::SeparatorText("Polynomial");
    dirty_ |= ImGui::SliderInt("degree n", &polyDeg_, 2, 8);
    dirty_ |= ImGui::SliderFloat("damping", &damping_, 0.1f, 2.0f);
    dirty_ |= ImGui::Checkbox("convergence shading", &showShading_);

    ImGui::SeparatorText("View");
    double mnX=-2.0,mxX=2.0, mnY=-2.0,mxY=2.0;
    double mnZ=0.1,mxZ=5.0;
    dirty_ |= ImGui::SliderScalar("center X", ImGuiDataType_Double, &cx_, &mnX, &mxX);
    dirty_ |= ImGui::SliderScalar("center Y", ImGuiDataType_Double, &cy_, &mnY, &mxY);
    dirty_ |= ImGui::SliderScalar("zoom",     ImGuiDataType_Double, &zoom_, &mnZ, &mxZ,
                                  "%.3f", ImGuiSliderFlags_Logarithmic);
    dirty_ |= ImGui::SliderInt("max iter", &maxIter_, 16, 256);

    ImGui::SeparatorText("Presets");
    if (ImGui::Button("z^3"))  { polyDeg_=3; damping_=1.0f; cx_=0; cy_=0; zoom_=3; dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("z^4"))  { polyDeg_=4; damping_=1.0f; cx_=0; cy_=0; zoom_=3; dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("z^6"))  { polyDeg_=6; damping_=1.0f; cx_=0; cy_=0; zoom_=3; dirty_=true; }
    if (ImGui::Button("Relaxed (d=0.5)")) { damping_=0.5f; dirty_=true; }
    ImGui::SameLine();
    if (ImGui::Button("Over (d=1.5)"))    { damping_=1.5f; dirty_=true; }

    ImGui::Separator();
    ImGui::Text("Roots: %d  |  FPS: %.1f", polyDeg_, ImGui::GetIO().Framerate);
    ImGui::End();
}

void NewtonPlugin::renderScene(const RenderContext& ctx) {
    if (dirty_ || !tex_ || texW_!=ctx.width || texH_!=ctx.height)
        recompute(ctx.width, ctx.height);
    if (tex_) SDL_RenderCopy(ctx.renderer, tex_, nullptr, nullptr);
    sel_.draw(ctx.renderer);
}

void NewtonPlugin::teardown() {
    if (tex_) { SDL_DestroyTexture(tex_); tex_=nullptr; }
}

void NewtonPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if (sel_.isReady()) {
        SDL_Rect r = sel_.consume();
        applyZoom(r, ctx.width, ctx.height, cx_, cy_, zoom_);
        dirty_ = true;
    }
}
