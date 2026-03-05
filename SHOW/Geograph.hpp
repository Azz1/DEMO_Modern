#pragma once
#include "IPlugin.hpp"
#include "Geograph_presets.hpp"
#include <string>
#include <vector>
#include <array>

class GeographPlugin : public IPlugin {
public:
    std::string name() const override { return "Geograph"; }
    void setup(SDL_Renderer* r) override;
    void teardown() override;
    void renderUI() override;
    void renderScene(const RenderContext& ctx) override;
    void handleEvent(const SDL_Event& e, const RenderContext& ctx) override;

private:
    // ── Data ──
    int   m_=0, n_=0;          // rows, cols
    float stepx_=1, stepy_=1;
    float sx_=0, sy_=0;        // origin
    float minZ_=0, maxZ_=1;
    std::vector<std::vector<float>> grid_;   // grid_[row][col]

    void loadBuiltin();
    void loadPreset(int idx);
    bool loadFile(const char* path);
    void parseData();
    void serializeToText();
    void buildContour(int levels);
    void getExt();

    // ── Contour ──
    using fPoint = std::array<float,2>;
    using fCurve = std::vector<fPoint>;
    using fLevel = std::vector<fCurve>;
    std::vector<fLevel> contours_;  // one fLevel per height step
    int   builtContourLevels_ = 0;

    void contourh(float h, fLevel& lev);
    void getI(float h, std::vector<std::vector<short>>& I);
    void getP(float h,int ix,int iy,int v,float&x,float&y);
    int  extP(float h,int&ix,int&iy,int&v,float&x,float&y,
              std::vector<std::vector<short>>&I);
    void getCurve(float h,int idrec,int ix,int iy,int v,
                  fLevel&lev, std::vector<std::vector<short>>&I);

    // ── Color ──
    void contColour(int op, float h, Uint8&r, Uint8&g, Uint8&b);

    // ── 3D projection ──
    struct Proj3D {
        double a,b,c,d,e,f,u,v,D;
        int Vsx,Vsy;
        bool valid=false;
    } proj_;

    void initProj(int W, int H);
    bool trans3D(float wx,float wy,float wz,int&sx,int&sy);

    // ── Render helpers ──
    void render2DMap    (SDL_Renderer*,int W,int H,int top);
    void render2DContour(SDL_Renderer*,int W,int H,int top);
    void render3DWire   (SDL_Renderer*,int W,int H,int top);
    void render3DHidden    (SDL_Renderer*,int W,int H,int top);
    void render3DCont      (SDL_Renderer*,int W,int H,int top);
    void render3DContHidden(SDL_Renderer*,int W,int H,int top);

    // Visibility helpers (port of original pvisible/getposi/faceeq)
    static void faceEq(float ix,float iy,float iz,
                       float tx,float ty,float tz, float feq[4]);
    static int  relpf (float x,float y,float z,const float feq[4]);
    bool getPosi(int wh,int idx,float ix,float iy,float tanv,
                 const float feq[4],int& pos) const;
    bool isVisible(float gx,float gy,float gz) const;

    // Eye position in grid-index coords (set by initProj)
    float eyeGX_=0,eyeGY_=0,eyeGZ_=0;
    void drawColorBar   (SDL_Renderer*,int W,int H,int top);

    SDL_Texture* tex_=nullptr;
    int texW_=0,texH_=0;
    bool dirty_=true;

    // ── UI state ──
    int   mode_   = 0;    // 0=2D map, 1=2D contour, 2=3D wire, 3=3D+cont
    int   levels_ = 10;
    int   colorOp_= 0;    // 0=color, 1=gray
    float theta_  = 45.f; // elevation angle (degrees)
    float rz_     = 30.f; // azimuth (degrees)
    float radiu_  = 800.f;
    float depF_   = 400.f;
    char  fileBuf_[256]="";
    char  dataBuf_[65536]="";
    bool  dataDirty_=false;
    int   preset_=-1;   // -1 = built-in wave, 0..N = .pan preset
    bool  pan_=false; int panSX_=0,panSY_=0;
    float panBaseTheta_=45,panBaseRz_=30;
};
