#include "Jout.hpp"
#include <imgui.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

static const double FPS = 1e-10;
static const double kPI = 3.14159265358979;

static void sqrz(double a, double b, double* x, double* y) {
    double theta;
    if      (a >  FPS) theta = atan(b/a);
    else if (a < -FPS) theta = atan(b/a) + kPI;
    else               theta = (b>=0) ? kPI/2 : -kPI/2;
    double r = pow(a*a+b*b, 0.25);
    *x = r * cos(theta/2);
    *y = r * sin(theta/2);
}

void JoutPlugin::setup(SDL_Renderer* r) { srand((unsigned)time(nullptr)); dirty_=true; }
void JoutPlugin::teardown() { if(tex_){SDL_DestroyTexture(tex_);tex_=nullptr;} }

void JoutPlugin::render(SDL_Renderer* r, int w, int h, int top) {
    int th = h - top;
    if (!tex_ || texW_!=w || texH_!=th) {
        if(tex_) SDL_DestroyTexture(tex_);
        tex_ = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, w, th);
        texW_=w; texH_=th;
    }
    void* px; int pitch;
    SDL_LockTexture(tex_,nullptr,&px,&pitch);
    Uint32* p=(Uint32*)px;
    for(int i=0;i<w*th;++i) p[i]=0x000000FF;

    double ratex = w  / (cex_-csx_);
    double ratey = th / (cey_-csy_);

    double a = 1.0+4.0*cx_, b = 4.0*cy_;
    double x, y;
    sqrz(a, b, &x, &y);

    // Choose starting branch closer to origin
    double x1=(1-x)/2, y1=-y/2, x2=(1+x)/2, y2=y/2;
    if(fabs(1+x1)>=fabs(1-x1)) { x=x2; y=y2; } else { x=x1; y=y1; }

    for(int i=0;i<count_;++i) {
        a=x+cx_; b=y+cy_;
        sqrz(a,b,&x,&y);
        // Plot both +/- square roots
        auto plot=[&](double px2,double py2){
            int sx=int((px2-csx_)*ratex);
            int sy=int(th-1-(py2-csy_)*ratey);
            if(sx>=0&&sx<w&&sy>=0&&sy<th)
                p[sy*(pitch/4)+sx]=0xFF44FFFF; // magenta
        };
        plot(x,y); plot(-x,-y);

        // Randomly flip sign (original behavior)
        double f1=(x+cx_)*(x+cx_)+(y+cy_)*(y+cy_);
        double f2=(-x+cx_)*(-x+cx_)+(-y+cy_)*(-y+cy_);
        if(f1>f2) { x=-x; y=-y; }
        if(rand()%1000<100) { x=-x; y=-y; }
    }
    SDL_UnlockTexture(tex_);
    dirty_=false;
}

void JoutPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if(ImGui::GetIO().WantCaptureMouse) return;
    sel_.handleEvent(e);
    if(sel_.isReady()) {
        SDL_Rect s=sel_.consume();
        int th=ctx.height-ctx.topOffset;
        double rx=(cex_-csx_)/ctx.width, ry=(cey_-csy_)/th;
        csx_+=s.x*rx; cex_=csx_+(s.w)*rx;
        double ny0=csy_+(th-s.y-s.h)*ry;
        cey_=ny0+(s.h)*ry; csy_=ny0;
        dirty_=true;
    }
}

void JoutPlugin::renderScene(const RenderContext& ctx) {
    if(dirty_) render(ctx.renderer,ctx.width,ctx.height,ctx.topOffset);
    if(tex_) {
        SDL_Rect dst={0,ctx.topOffset,ctx.width,ctx.height-ctx.topOffset};
        SDL_RenderCopy(ctx.renderer,tex_,nullptr,&dst);
    }
    sel_.draw(ctx.renderer);
}

void JoutPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({260,230},ImGuiCond_Once);
    ImGui::Begin("Julia Out (IIM)");
    ImGui::SeparatorText("Julia c parameter");
    if(ImGui::DragFloat("cx",(float*)&cx_,0.005f,-2.f,2.f)) dirty_=true;
    if(ImGui::DragFloat("cy",(float*)&cy_,0.005f,-2.f,2.f)) dirty_=true;
    ImGui::SeparatorText("View");
    if(ImGui::DragFloat("x min",(float*)&csx_,0.01f)) dirty_=true;
    if(ImGui::DragFloat("x max",(float*)&cex_,0.01f)) dirty_=true;
    if(ImGui::DragFloat("y min",(float*)&csy_,0.01f)) dirty_=true;
    if(ImGui::DragFloat("y max",(float*)&cey_,0.01f)) dirty_=true;
    if(ImGui::SliderInt("points",&count_,1000,200000)) dirty_=true;
    if(ImGui::Button("Reset")) {
        csx_=-2.5;csy_=-1.8;cex_=2.5;cey_=1.8;cx_=0.2;cy_=0.7;dirty_=true;
    }
    ImGui::SeparatorText("Info");
    ImGui::TextDisabled("Drag to zoom in");
    ImGui::TextDisabled("Inverse Iteration Method");
    ImGui::End();
}
