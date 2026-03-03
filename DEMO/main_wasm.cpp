#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <memory>
#include <vector>
#include <string>
#include <cstdio>
#include <ctime>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// imfilebrowser needs filesystem — skip in WASM, use simple name input
#define WASM_BUILD

#include "Lorenz.hpp"
#include "Mandelbrot.hpp"
#include "Julia.hpp"
#include "Newton.hpp"
#include "Ecosys.hpp"
#include "Henon.hpp"
#include "May.hpp"
#include "Trammel.hpp"

// ── Globals ───────────────────────────────────────────────────────────────────

static SDL_Window*   gWindow   = nullptr;
static SDL_Renderer* gRenderer = nullptr;

static std::vector<std::unique_ptr<IPlugin>> gPlugins;
static int  gActive  = 0;
static bool gRunning = true;

static std::string gLastSavePath;
static float       gNotifyTimer = 0.f;
static bool        gSaveError   = false;
static bool        gShowSaveDlg = false;
static char        gSaveNameBuf[256] = {};

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string defaultFilename(const std::string& pluginName) {
    time_t t = time(nullptr);
    struct tm* tm = localtime(&t);
    char ts[32]; strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", tm);
    std::string safe = pluginName;
    for (char& c : safe) if (c==' ') c='_';
    return safe + "_" + ts + ".jpg";
}

// Callback for stb jpeg writer
static void stbWriteCb(void* ctx, void* data, int size) {
    auto* b = (std::vector<unsigned char>*)ctx;
    auto* p = (unsigned char*)data;
    b->insert(b->end(), p, p+size);
}

EM_JS(void, js_download_file, (const char* path), {
    var fname = UTF8ToString(path);
    try {
        var data = FS.readFile(fname);
        var blob = new Blob([data], {type:"image/jpeg"});
        var url  = URL.createObjectURL(blob);
        var a    = document.createElement("a");
        a.href     = url;
        a.download = fname.split("/").pop();
        a.style.display = "none";
        document.body.appendChild(a);
        a.click();
        setTimeout(function(){ document.body.removeChild(a); URL.revokeObjectURL(url); }, 1000);
        FS.unlink(fname);
    } catch(e) { console.error("Download failed:", e); }
});

// Write JPEG and trigger browser download
static void saveAndDownload(SDL_Renderer* r, const std::string& filename) {
    int w, h;
    SDL_GetRendererOutputSize(r, &w, &h);
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 24, SDL_PIXELFORMAT_RGB24);
    if (!surf) { gSaveError=true; gNotifyTimer=3.f; return; }
    SDL_RenderReadPixels(r, nullptr, SDL_PIXELFORMAT_RGB24, surf->pixels, surf->pitch);

    std::vector<unsigned char> buf;
    stbi_write_jpg_to_func(stbWriteCb, &buf, w, h, 3, surf->pixels, 90);
    SDL_FreeSurface(surf);

    if (buf.empty()) { gSaveError=true; gNotifyTimer=3.f; return; }

    // Write to emscripten virtual FS, then download via JS FS.readFile
    FILE* f = fopen(filename.c_str(), "wb");
    if (f) { fwrite(buf.data(), 1, buf.size(), f); fclose(f); }

    js_download_file(filename.c_str());

    gLastSavePath = filename;
    gSaveError    = false;
    gNotifyTimer  = 3.f;
}

static void openSaveDlg() {
    snprintf(gSaveNameBuf, sizeof(gSaveNameBuf), "%s",
             defaultFilename(gPlugins[gActive]->name()).c_str());
    gShowSaveDlg = true;
}

static void drawSaveDlg(SDL_Renderer* r, int sw, int sh) {
    if (!gShowSaveDlg) return;
    ImGui::SetNextWindowSize({420, 95}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({float(sw)/2-210, float(sh)/2-47}, ImGuiCond_Always);
    ImGui::Begin("💾 Save Screenshot", nullptr,
                 ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|
                 ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("File name:");
    ImGui::SetNextItemWidth(320);
    bool enter = ImGui::InputText("##n", gSaveNameBuf, sizeof(gSaveNameBuf),
                                  ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Save") || enter) {
        gShowSaveDlg = false;
        std::string name = gSaveNameBuf;
        if (name.size()<4 || name.substr(name.size()-4)!=".jpg") name+=".jpg";
        saveAndDownload(r, name);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) gShowSaveDlg = false;
    ImGui::End();
}

static void drawNotification(int sw, int sh) {
    if (gNotifyTimer<=0.f) return;
    ImGui::SetNextWindowPos({float(sw)/2-200, float(sh)-56}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({400, 40}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin("##notif", nullptr,
                 ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
                 ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
    if (gSaveError)
        ImGui::TextColored({1,.3f,.3f,1}, "✗ Save error");
    else
        ImGui::TextColored({.3f,1,.5f,1}, "✓ Downloaded: %s", gLastSavePath.c_str());
    ImGui::End();
}

// ── Main loop (called by emscripten) ─────────────────────────────────────────

static void mainLoop() {
    static Uint32 prevTick = SDL_GetTicks();
    Uint32 now = SDL_GetTicks();
    float delta = (now-prevTick)/1000.f;
    prevTick = now;
    if (gNotifyTimer>0.f) gNotifyTimer -= delta;

    SDL_Event e;
    int w, h;
    SDL_GetRendererOutputSize(gRenderer, &w, &h);
    float menuH = ImGui::GetFrameHeight();
    RenderContext ctx{gRenderer, w, h, (int)menuH};

    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type==SDL_QUIT) { emscripten_cancel_main_loop(); return; }
        if (e.type==SDL_KEYDOWN && !gShowSaveDlg) {
            if (e.key.keysym.sym>=SDLK_1 && e.key.keysym.sym<=SDLK_8)
                gActive = e.key.keysym.sym-SDLK_1;
            if (e.key.keysym.sym==SDLK_F12) openSaveDlg();
        }
        if (!gShowSaveDlg)
            gPlugins[gActive]->handleEvent(e, ctx);
    }

    SDL_SetRenderDrawColor(gRenderer,12,12,20,255);
    SDL_RenderClear(gRenderer);
    gPlugins[gActive]->renderScene(ctx);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::BeginMainMenuBar()) {
        for (int i=0;i<(int)gPlugins.size();++i) {
            bool sel=(i==gActive);
            if (ImGui::MenuItem(gPlugins[i]->name().c_str(),nullptr,sel)) gActive=i;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("📷 Save JPG","F12")) openSaveDlg();
        ImGui::Separator();
        ImGui::TextDisabled("(keys 1-8)  |  drag to zoom");
        ImGui::EndMainMenuBar();
    }

    drawSaveDlg(gRenderer, w, h);
    drawNotification(w, h);
    gPlugins[gActive]->renderUI();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), gRenderer);
    SDL_RenderPresent(gRenderer);
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    // Read canvas size set by HTML/CSS (90vw x 90vh)
    int W = 1280, H = 800;
    emscripten_get_canvas_element_size("#canvas", &W, &H);
    if (W < 100) W = 1280;
    if (H < 100) H = 800;

    gWindow = SDL_CreateWindow("Demo1",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               W, H, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!gWindow) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        return 1;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
    if (!gRenderer) {
        printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        return 1;
    }
    printf("Renderer: %s\n", SDL_GetRendererInfo(gRenderer, nullptr), "ok");
    SDL_RendererInfo info; SDL_GetRendererInfo(gRenderer, &info);
    printf("Using renderer: %s\n", info.name);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().FontGlobalScale = 1.2f;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(gWindow, gRenderer);
    ImGui_ImplSDLRenderer2_Init(gRenderer);

    gPlugins.push_back(std::make_unique<LorenzPlugin>());
    gPlugins.push_back(std::make_unique<MandelbrotPlugin>());
    gPlugins.push_back(std::make_unique<JuliaPlugin>());
    gPlugins.push_back(std::make_unique<NewtonPlugin>());
    gPlugins.push_back(std::make_unique<EcosysPlugin>());
    gPlugins.push_back(std::make_unique<HenonPlugin>());
    gPlugins.push_back(std::make_unique<MayPlugin>());
    gPlugins.push_back(std::make_unique<TrammelPlugin>());
    for (auto& p : gPlugins) p->setup(gRenderer);

    emscripten_set_main_loop(mainLoop, 0, 1);
    return 0;
}
