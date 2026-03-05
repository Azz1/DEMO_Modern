#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <memory>
#include <vector>
#include <string>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "imfilebrowser.h"

#include "Lorenz.hpp"
#include "Mandelbrot.hpp"
#include "Julia.hpp"
#include "Newton.hpp"
#include "Ecosys.hpp"
#include "Henon.hpp"
#include "May.hpp"
#include "Trammel.hpp"
#include "Lsys.hpp"
#include "Jout.hpp"
#include "Pfrac.hpp"
#include "Eyes.hpp"
#include "GraphView.hpp"
#include "Diagram.hpp"
#include "Geograph.hpp"

namespace fs = std::filesystem;

// ── Globals ───────────────────────────────────────────────────────────────────

static std::string lastSaveDir;
static std::string lastSavePath;
static float       saveNotifyTimer = 0.f;
static bool        saveError       = false;

// File browser instance (save mode)
static ImGui::FileBrowser fileBrowser(
    ImGuiFileBrowserFlags_EnterNewFilename |
    ImGuiFileBrowserFlags_CreateNewDir
);
static bool fileBrowserPending = false;   // waiting for browser result
static std::string pendingFilename;       // default filename to suggest

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string defaultFilename(const std::string& pluginName) {
    time_t t = time(nullptr);
    struct tm* tm = localtime(&t);
    char ts[32]; strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", tm);
    std::string safe = pluginName;
    for (char& c : safe) if (c==' ') c='_';
    return safe + "_" + ts + ".jpg";
}

static void doSave(SDL_Renderer* renderer, const std::string& path) {
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 24, SDL_PIXELFORMAT_RGB24);
    if (!surf) { saveError=true; saveNotifyTimer=3.f; return; }
    SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGB24, surf->pixels, surf->pitch);
    int ok = stbi_write_jpg(path.c_str(), w, h, 3, surf->pixels, 90);
    SDL_FreeSurface(surf);
    lastSavePath    = path;
    saveError       = !ok;
    saveNotifyTimer = 3.f;
    size_t sl = path.rfind('/');
    if (sl != std::string::npos) lastSaveDir = path.substr(0, sl);
    printf("%s: %s\n", ok?"Saved":"Error", path.c_str());
}

static void openSaveDialog(const std::string& pluginName) {
    pendingFilename = defaultFilename(pluginName);
    std::string dir = lastSaveDir.empty() ? "." : lastSaveDir;

    fileBrowser.SetTitle("Save Screenshot");
    fileBrowser.SetTypeFilters({".jpg", ".jpeg"});
    fileBrowser.SetPwd(dir);
    fileBrowser.SetInputName(pendingFilename);
    fileBrowser.Open();
    fileBrowserPending = true;
}

static void drawNotification(int screenW, int screenH) {
    if (saveNotifyTimer <= 0.f) return;
    ImGui::SetNextWindowPos({float(screenW)/2-220, float(screenH)-60}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({440, 40}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin("##notif", nullptr,
                 ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|
                 ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
    size_t sl = lastSavePath.rfind('/');
    std::string fname = (sl==std::string::npos) ? lastSavePath : lastSavePath.substr(sl+1);
    if (saveError)
        ImGui::TextColored({1,.3f,.3f,1}, "✗ Error saving: %s", fname.c_str());
    else
        ImGui::TextColored({.3f,1,.5f,1}, "✓ Saved: %s", fname.c_str());
    ImGui::End();
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    if (SDL_Init(SDL_INIT_VIDEO)!=0) {
        fprintf(stderr,"SDL_Init: %s\n",SDL_GetError()); return 1;
    }
    SDL_Window* win = SDL_CreateWindow("Demo1 — Math Visualizer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800,
        SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN);
    if (!win) { fprintf(stderr,"%s\n",SDL_GetError()); return 1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(win,-1,SDL_RENDERER_SOFTWARE);
    if (!renderer) { fprintf(stderr,"%s\n",SDL_GetError()); return 1; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().FontGlobalScale = 1.2f;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(win, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    std::vector<std::unique_ptr<IPlugin>> plugins;
    plugins.push_back(std::make_unique<LorenzPlugin>());
    plugins.push_back(std::make_unique<MandelbrotPlugin>());
    plugins.push_back(std::make_unique<JuliaPlugin>());
    plugins.push_back(std::make_unique<NewtonPlugin>());
    plugins.push_back(std::make_unique<EcosysPlugin>());
    plugins.push_back(std::make_unique<HenonPlugin>());
    plugins.push_back(std::make_unique<MayPlugin>());
    plugins.push_back(std::make_unique<TrammelPlugin>());
    plugins.push_back(std::make_unique<LsysPlugin>());
    plugins.push_back(std::make_unique<JoutPlugin>());
    plugins.push_back(std::make_unique<PfracPlugin>());
    plugins.push_back(std::make_unique<EyesPlugin>());
    plugins.push_back(std::make_unique<GraphViewPlugin>());
    plugins.push_back(std::make_unique<DiagramPlugin>());
    plugins.push_back(std::make_unique<GeographPlugin>());
    for (auto& p : plugins) p->setup(renderer);

    // Init save dir to cwd
    { char buf[4096]; if (getcwd(buf,sizeof(buf))) lastSaveDir=buf; }

    int    active  = 0;
    bool   running = true;
    Uint32 prevTick = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float delta = (now-prevTick)/1000.f;
        prevTick = now;
        if (saveNotifyTimer>0.f) saveNotifyTimer -= delta;

        SDL_Event e;
        int w, h;
        SDL_GetRendererOutputSize(renderer,&w,&h);
        float menuH = ImGui::GetFrameHeight();
        RenderContext ctx{renderer, w, h, (int)menuH};

        bool browserOpen = fileBrowser.IsOpened();
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type==SDL_QUIT) running=false;
            if (e.type==SDL_KEYDOWN && !browserOpen) {
                if (e.key.keysym.sym==SDLK_F12)
                    openSaveDialog(plugins[active]->name());
            }
            if (!browserOpen)
                plugins[active]->handleEvent(e, ctx);
        }

        SDL_SetRenderDrawColor(renderer,12,12,20,255);
        SDL_RenderClear(renderer);
        plugins[active]->renderScene(ctx);

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // ── Menu bar ──
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Demo")) {
                int evalStart = (int)plugins.size()-3;
                for (int i=0;i<evalStart;++i) {
                    bool sel=(i==active);
                    if (ImGui::MenuItem(plugins[i]->name().c_str(),
                                        nullptr,sel))
                        active=i;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Evaluate")) {
                int evalStart2 = (int)plugins.size()-3;
                for (int i=evalStart2;i<(int)plugins.size();++i) {
                    bool sel=(i==active);
                    if (ImGui::MenuItem(plugins[i]->name().c_str(),
                                        nullptr,sel))
                        active=i;
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("📷 Save JPG","F12"))
                openSaveDialog(plugins[active]->name());
            ImGui::Separator();
            ImGui::TextDisabled(plugins[active]->name().c_str());
            ImGui::EndMainMenuBar();
        }

        // ── File browser ──
        fileBrowser.Display();
        if (fileBrowser.HasSelected()) {
            std::string path = fileBrowser.GetSelected().string();
            if (path.size()<4 || path.substr(path.size()-4)!=".jpg")
                path += ".jpg";
            doSave(renderer, path);
            fileBrowser.ClearSelected();
            fileBrowserPending = false;
        }

        drawNotification(w, h);
        plugins[active]->renderUI();
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    for (auto& p : plugins) p->teardown();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
