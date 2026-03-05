#include "Diagram.hpp"
#include <imgui.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

constexpr Uint8 DiagramPlugin::kColorMap[8][3];

// ── Built-in presets ──────────────────────────────────────────────────────────
static const char* kPresets[] = {
    // 0: Sine + Cosine
    "# Sine and Cosine\n"
    "2\n"
    "0.0 0.0 1.0\n0.5 0.48 0.88\n1.0 0.84 0.54\n1.5 1.00 0.07\n"
    "2.0 0.91 -0.42\n2.5 0.60 -0.80\n3.0 0.14 -1.00\n3.5 -0.35 -0.94\n"
    "4.0 -0.76 -0.65\n4.5 -0.98 -0.21\n5.0 -0.96 0.28\n5.5 -0.71 0.71\n6.0 -0.28 0.96\n",

    // 1: Lissajous-like
    "# Polynomial curves\n"
    "2\n"
    "0.0 0.0 0.0\n0.5 0.12 0.03\n1.0 0.50 0.25\n1.5 1.12 0.84\n"
    "2.0 2.00 2.00\n2.5 3.12 3.91\n3.0 4.50 5.06\n3.5 6.12 5.69\n"
    "4.0 8.00 4.00\n4.5 10.12 -0.09\n5.0 12.50 -8.00\n",

    // 2: Three curves
    "# Three data series\n"
    "3\n"
    "1 10 20 5\n2 25 30 12\n3 15 45 8\n4 40 35 15\n"
    "5 60 50 22\n6 45 60 30\n7 70 55 25\n8 80 70 35\n",
};
static const char* kPresetNames[] = {"Sine/Cosine", "Polynomials", "3 Series"};

// ── Parse text → curves ───────────────────────────────────────────────────────
void DiagramPlugin::parseInput() {
    curves_.clear();
    const char* p = inputBuf_;
    // Skip comment lines
    while (*p) {
        while (*p=='\n'||*p=='\r'||*p==' ') ++p;
        if (*p=='#') { while (*p&&*p!='\n') ++p; continue; }
        break;
    }
    int nc=0; sscanf(p,"%d",&nc);
    while (*p&&*p!='\n') ++p;
    if (nc<=0||nc>8) return;

    curves_.resize(nc);
    char line[512];
    while (*p) {
        while (*p=='\n'||*p=='\r') ++p;
        if (!*p) break;
        // read one line
        int li=0;
        while (*p&&*p!='\n'&&*p!='\r'&&li<510) line[li++]=*p++;
        line[li]=0;
        if (!li||line[0]=='#') continue;
        // parse: x y0 y1 y2 ...
        float vals[9]={};
        int cnt=sscanf(line,"%f %f %f %f %f %f %f %f %f",
            vals,vals+1,vals+2,vals+3,vals+4,vals+5,vals+6,vals+7,vals+8);
        if (cnt<2) continue;
        for (int i=0;i<nc&&i<cnt-1;++i) {
            if (vals[i+1]!=(float)-999)
                curves_[i].push_back({vals[0], vals[i+1]});
        }
    }
    // Autofit view
    float minx=1e9,miny=1e9,maxx=-1e9,maxy=-1e9;
    for (auto& c:curves_) for (auto& pt:c) {
        if(pt[0]<minx)minx=pt[0]; if(pt[0]>maxx)maxx=pt[0];
        if(pt[1]<miny)miny=pt[1]; if(pt[1]>maxy)maxy=pt[1];
    }
    if (maxx>minx&&maxy>miny) {
        float mx=(maxx-minx)*0.1f, my=(maxy-miny)*0.1f;
        csx_=minx-mx; cex_=maxx+mx; csy_=miny-my; cey_=maxy+my;
    }
    inputDirty_=false;
}

void DiagramPlugin::buildBuiltin() {
    strncpy(inputBuf_,kPresets[preset_],sizeof(inputBuf_)-1);
    parseInput();
}

void DiagramPlugin::setup(SDL_Renderer*) { srand((unsigned)time(nullptr)); buildBuiltin(); }

// ── Coordinate transform ──────────────────────────────────────────────────────
DiagramPlugin::fPoint DiagramPlugin::toScreen(float x, float y, int W, int H, int top) const {
    float rx=(W)/(cex_-csx_), ry=(H-top)/(cey_-csy_);
    return {(x-csx_)*rx, (H-top)-((y-csy_)*ry)+(float)top};
}

// ── Grid / axes ───────────────────────────────────────────────────────────────
void DiagramPlugin::drawGrid(SDL_Renderer* r, int W, int H, int top) {
    if (!grid_) return;
    float rx=W/(cex_-csx_), ry=(H-top)/(cey_-csy_);
    float stepx=(cex_-csx_)/10.f, stepy=(cey_-csy_)/10.f;
    SDL_SetRenderDrawColor(r,40,40,60,255);
    for (float x=csx_; x<=cex_+0.001f; x+=stepx) {
        int sx=(int)((x-csx_)*rx);
        SDL_RenderDrawLine(r,sx,top,sx,H);
    }
    for (float y=csy_; y<=cey_+0.001f; y+=stepy) {
        int sy=(int)(H-(y-csy_)*ry);
        SDL_RenderDrawLine(r,0,sy,W,sy);
    }
    // Axes
    SDL_SetRenderDrawColor(r,120,120,160,255);
    int ax0=(int)((0-csx_)*rx), ay0=(int)(H-(0-csy_)*ry);
    if (ax0>=0&&ax0<W) SDL_RenderDrawLine(r,ax0,top,ax0,H);
    if (ay0>=top&&ay0<H) SDL_RenderDrawLine(r,0,ay0,W,ay0);
}

// ── Dline ─────────────────────────────────────────────────────────────────────
void DiagramPlugin::drawDline(SDL_Renderer* r, const fCurve& p, int W, int H, int top,
                               Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r,R,G,B,255);
    for (int i=0;i<(int)p.size()-1;++i) {
        auto a=toScreen(p[i][0],p[i][1],W,H,top);
        auto b=toScreen(p[i+1][0],p[i+1][1],W,H,top);
        SDL_RenderDrawLine(r,(int)a[0],(int)a[1],(int)b[0],(int)b[1]);
    }
}

// ── Spline ────────────────────────────────────────────────────────────────────
void DiagramPlugin::computeSplineDerivs(const fCurve& p,
                                         std::vector<fPoint>& dp, std::vector<float>& L) {
    int n=(int)p.size();
    L.resize(n); dp.resize(n);
    for (int i=0;i<n-1;++i)
        L[i]=sqrt((p[i+1][0]-p[i][0])*(p[i+1][0]-p[i][0])
                 +(p[i+1][1]-p[i][1])*(p[i+1][1]-p[i][1]));

    std::vector<float> U(n),a(n); std::vector<fPoint> b(n),C(n);
    C[0]={3.f*(p[1][0]-p[0][0])/L[0], 3.f*(p[1][1]-p[0][1])/L[0]};
    C[n-1]={3.f*(p[n-1][0]-p[n-2][0])/L[n-2], 3.f*(p[n-1][1]-p[n-2][1])/L[n-2]};
    U[0]=1.f; U[n-1]=0.f;
    for (int i=1;i<n-1;++i) {
        U[i]=L[i-1]/(L[i]+L[i-1]);
        C[i]={3.f*((1-U[i])*(p[i][0]-p[i-1][0])/L[i-1]+U[i]*(p[i+1][0]-p[i][0])/L[i]),
              3.f*((1-U[i])*(p[i][1]-p[i-1][1])/L[i-1]+U[i]*(p[i+1][1]-p[i][1])/L[i])};
    }
    a[0]=-U[0]/2.f; b[0]={C[0][0]/2.f,C[0][1]/2.f};
    for (int i=1;i<n;++i) {
        float mid=2.f+(1-U[i])*a[i-1];
        a[i]=-U[i]/mid;
        b[i]={(C[i][0]-(1-U[i])*b[i-1][0])/mid,
              (C[i][1]-(1-U[i])*b[i-1][1])/mid};
    }
    dp[n-1]=b[n-1];
    for (int i=n-2;i>=0;--i) dp[i]={a[i]*dp[i+1][0]+b[i][0], a[i]*dp[i+1][1]+b[i][1]};
}

void DiagramPlugin::drawScurve(SDL_Renderer* r, const fCurve& p, int W, int H, int top,
                                Uint8 R, Uint8 G, Uint8 B) {
    int n=(int)p.size(); if (n<2) return;
    std::vector<fPoint> dp; std::vector<float> L;
    computeSplineDerivs(p,dp,L);
    SDL_SetRenderDrawColor(r,R,G,B,255);
    for (int i=0;i<n-1;++i) {
        float l1=L[i], l2=l1*l1, l3=l2*l1;
        float x0=p[i][0], y0=p[i][1];
        int steps=std::max(20,(int)(L[i]*2));
        float dt=l1/steps;
        for (int c=0;c<=steps;++c) {
            float t=c*dt;
            float x=(2*p[i][0]-2*p[i+1][0]+dp[i][0]*l1+dp[i+1][0]*l1)*t*t*t/l3
                   +(-3*p[i][0]+3*p[i+1][0]-2*dp[i][0]*l1-dp[i+1][0]*l1)*t*t/l2
                   +dp[i][0]*t+p[i][0];
            float y=(2*p[i][1]-2*p[i+1][1]+dp[i][1]*l1+dp[i+1][1]*l1)*t*t*t/l3
                   +(-3*p[i][1]+3*p[i+1][1]-2*dp[i][1]*l1-dp[i+1][1]*l1)*t*t/l2
                   +dp[i][1]*t+p[i][1];
            auto sa=toScreen(x0,y0,W,H,top), sb=toScreen(x,y,W,H,top);
            SDL_RenderDrawLine(r,(int)sa[0],(int)sa[1],(int)sb[0],(int)sb[1]);
            x0=x; y0=y;
        }
    }
}

// ── Fcurve (IFS on control points) ───────────────────────────────────────────
void DiagramPlugin::drawFcurve(SDL_Renderer* r, const fCurve& p, int W, int H, int top,
                                Uint8 R, Uint8 G, Uint8 B) {
    int n=(int)p.size(); if (n<2) return;
    // Transform the curve to screen first (use pre-transformed coords)
    // Compute IFS coefficients
    std::vector<float> la(n),lc(n),le(n),lff(n);
    // Use screen-space coords
    std::vector<fPoint> sp(n);
    for (int i=0;i<n;++i) sp[i]=toScreen(p[i][0],p[i][1],W,H,top);

    float b=(float)(sp[n-1][0]-sp[0][0]);
    if (fabs(b)<1.f) b=1.f;
    float fd=fracD_;
    for (int i=1;i<n;++i) {
        la[i]=(sp[i][0]-sp[i-1][0])/b;
        le[i]=(sp[n-1][0]*sp[i-1][0]-sp[0][0]*sp[i][0])/b;
        lc[i]=(sp[i][1]-sp[i-1][1]-fd*(sp[n-1][1]-sp[0][1]))/b;
        lff[i]=(sp[n-1][0]*sp[i-1][1]-sp[0][0]*sp[i][1]
               -fd*(sp[n-1][0]*sp[0][1]-sp[0][0]*sp[n-1][1]))/b;
    }
    float x=sp[0][0], y=sp[0][1];
    SDL_SetRenderDrawColor(r,R,G,B,255);
    for (int i=0;i<20000;++i) {
        int k=rand()%(n-1)+1;
        float nx=la[k]*x+le[k];
        float ny=lc[k]*x+fd*y+lff[k];
        x=nx; y=ny;
        int px=(int)x, py=(int)y;
        if (px>=0&&px<W&&py>=top&&py<H)
            SDL_RenderDrawPoint(r,px,py);
    }
}

// ── renderScene ───────────────────────────────────────────────────────────────
void DiagramPlugin::renderScene(const RenderContext& ctx) {
    int W=ctx.width, H=ctx.height, top=ctx.topOffset;
    SDL_SetRenderDrawColor(ctx.renderer,10,10,18,255);
    SDL_RenderClear(ctx.renderer);
    drawGrid(ctx.renderer,W,H,top);

    for (int i=0;i<(int)curves_.size();++i) {
        auto& c=curves_[i];
        if (c.size()<2) continue;
        int ci=i%kColors;
        Uint8 R=kColorMap[ci][0], G=kColorMap[ci][1], B=kColorMap[ci][2];
        switch(curveMode_) {
            case 0: drawDline (ctx.renderer,c,W,H,top,R,G,B); break;
            case 1: drawScurve(ctx.renderer,c,W,H,top,R,G,B); break;
            case 2: drawFcurve(ctx.renderer,c,W,H,top,R,G,B); break;
        }
        // Points
        if (showPoints_) {
            SDL_SetRenderDrawColor(ctx.renderer,255,80,255,255);
            for (auto& pt : c) {
                auto s=toScreen(pt[0],pt[1],W,H,top);
                SDL_Rect rc={(int)s[0]-3,(int)s[1]-3,6,6};
                SDL_RenderFillRect(ctx.renderer,&rc);
            }
        }
    }

    // Axis labels via ImGui overlay
    ImDrawList* dl=ImGui::GetBackgroundDrawList();
    char buf[32];
    snprintf(buf,31,"%.2f",csx_); dl->AddText(ImVec2(4,H-16),IM_COL32(180,180,180,200),buf);
    snprintf(buf,31,"%.2f",cex_); dl->AddText(ImVec2(W-40,H-16),IM_COL32(180,180,180,200),buf);
    snprintf(buf,31,"%.2f",cey_); dl->AddText(ImVec2(4,top+2),IM_COL32(180,180,180,200),buf);
    snprintf(buf,31,"%.2f",csy_); dl->AddText(ImVec2(4,H-30),IM_COL32(180,180,180,200),buf);
}

// ── renderUI ──────────────────────────────────────────────────────────────────
void DiagramPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({310,420},ImGuiCond_Once);
    ImGui::Begin("Diagram");

    ImGui::SeparatorText("Preset");
    if (ImGui::Combo("##pre",&preset_,kPresetNames,3)) buildBuiltin();

    ImGui::SeparatorText("Curve Mode");
    ImGui::RadioButton("Line",  &curveMode_,0); ImGui::SameLine();
    ImGui::RadioButton("Spline",&curveMode_,1); ImGui::SameLine();
    ImGui::RadioButton("Fractal",&curveMode_,2);
    if (curveMode_==2)
        ImGui::SliderFloat("Frac d",&fracD_,-1.f,1.f);
    ImGui::Checkbox("Show points",&showPoints_); ImGui::SameLine();
    ImGui::Checkbox("Grid",&grid_);

    ImGui::SeparatorText("View");
    ImGui::DragFloat("xMin",&csx_,0.05f); ImGui::SameLine();
    ImGui::DragFloat("xMax",&cex_,0.05f);
    ImGui::DragFloat("yMin",&csy_,0.05f); ImGui::SameLine();
    ImGui::DragFloat("yMax",&cey_,0.05f);

    ImGui::SeparatorText("Data (x y0 y1 ...)");
    ImGui::TextDisabled("Line1: # comment  Line2: ncurves");
    if (ImGui::InputTextMultiline("##data",inputBuf_,sizeof(inputBuf_),
                                  ImVec2(-1,130))) inputDirty_=true;
    if (inputDirty_) {
        if (ImGui::Button("Apply")) parseInput();
        ImGui::SameLine();
    }
    if (ImGui::Button("Refit")) {
        if (inputDirty_) parseInput();
        else buildBuiltin();
    }
    ImGui::SeparatorText("Info");
    ImGui::Text("%d curves", (int)curves_.size());
    ImGui::End();
}
