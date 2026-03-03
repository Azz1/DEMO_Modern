#include "Lsys.hpp"
#include <imgui.h>
#include <cmath>
#include <stack>
#include <SDL2/SDL.h>

static const float kPI = 3.14159265f;

LsysPreset LsysPlugin_presets[] = {
    {"Von Koch",   "F--F--F",        "F+F--F+F",         "",                       "",  4, 60.f,  0.f},
    {"Hilbert",    "A",              "B+F+B",             "+AF-BFB-FA+",            "B", 5, 90.f,  0.f},  // A/B rules
    {"Sierpinski", "F-G-G",         "F-G+F+G-F",         "GG",                     "G", 6,120.f,  0.f},  // F/G rules
    {"Tree",       "F",             "FF+[+F-F-F]-[-F+F+F]","",                     "",  4, 25.f, 90.f},
    {"Branch",     "X",             "FF",                "F+[[X]-X]-F[-FX]+X",     "",  6, 25.f, 65.f},
    {"Dragon",     "FX",            "F",                 "FX+YF+",                 "-FX-YF", 12, 90.f, 0.f},
};
static const int kNumPresets = 6;

std::string LsysPlugin::expand(const std::string& axiom, int levels) {
    std::string cur = axiom;
    for (int lv = 0; lv < levels; ++lv) {
        std::string next;
        next.reserve(cur.size() * 3);
        for (char c : cur) {
            if      (c=='F') next += ruleFBuf_;
            else if (c=='X') next += ruleXBuf_[0] ? ruleXBuf_ : "X";
            else if (c=='Y') next += ruleYBuf_[0] ? ruleYBuf_ : "Y";
            else if (c=='G') next += ruleYBuf_[0] ? ruleYBuf_ : "G"; // G uses Y slot
            else if (c=='A') next += ruleXBuf_[0] ? ruleXBuf_ : "A";
            else if (c=='B') next += ruleYBuf_[0] ? ruleYBuf_ : "B";
            else next += c;
        }
        cur = next;
        if (cur.size() > 2000000) break; // safety cap
    }
    return cur;
}

void LsysPlugin::generate() {
    turtle_ = expand(axiomBuf_, levels_);
    dirty_  = false;
}

void LsysPlugin::drawTurtle(SDL_Renderer* r, int w, int h, int topOff) {
    if (turtle_.empty()) return;

    // First pass: find bounds
    float x=0, y=0, th=startAngle_*kPI/180.f;
    float rad = angle_ * kPI / 180.f;
    float minX=0,maxX=0,minY=0,maxY=0;
    struct State { float x,y,th; };
    std::stack<State> stk;

    for (char c : turtle_) {
        if (c=='F'||c=='G') {
            float nx=x+cosf(th), ny=y-sinf(th);
            if(nx<minX)minX=nx; if(nx>maxX)maxX=nx;
            if(ny<minY)minY=ny; if(ny>maxY)maxY=ny;
            x=nx; y=ny;
        } else if(c=='+') th+=rad;
        else if(c=='-') th-=rad;
        else if(c=='[') stk.push({x,y,th});
        else if(c==']'&&!stk.empty()) { auto s=stk.top();stk.pop();x=s.x;y=s.y;th=s.th; }
    }

    float rangeX = maxX-minX, rangeY = maxY-minY;
    if(rangeX<0.001f) rangeX=1; if(rangeY<0.001f) rangeY=1;
    float usableH = h - topOff;
    float scale = fminf(w*0.9f/rangeX, usableH*0.9f/rangeY);
    float ox = w*0.05f - minX*scale;
    float oy = topOff + usableH*0.05f - minY*scale;

    // Second pass: draw
    x=0; y=0; th=startAngle_*kPI/180.f;
    while(!stk.empty()) stk.pop();
    float px=ox, py=oy;
    bool penDown=false;

    int hue=0;
    for (char c : turtle_) {
        if(c=='F'||c=='G') {
            float nx=x+cosf(th), ny=y-sinf(th);
            float sx=nx*scale+ox, sy=ny*scale+oy;
            // Color by hue
            float h6 = (hue%360)/60.f;
            int hi=int(h6)%6; float f=h6-int(h6);
            Uint8 cr,cg,cb;
            switch(hi){
              case 0:cr=0;cg=Uint8(200*f);cb=255;break;
              case 1:cr=0;cg=200;cb=Uint8(255*(1-f));break;
              case 2:cr=Uint8(180*f);cg=200;cb=0;break;
              case 3:cr=180;cg=Uint8(200*(1-f));cb=0;break;
              case 4:cr=255;cg=0;cb=Uint8(200*f);break;
              default:cr=255;cg=0;cb=Uint8(200*(1-f));break;
            }
            SDL_SetRenderDrawColor(r,cr,cg,cb,255);
            SDL_RenderDrawLine(r,int(px),int(py),int(sx),int(sy));
            hue+=2; px=sx; py=sy; x=nx; y=ny;
        } else if(c=='f') {
            float nx=x+cosf(th),ny=y-sinf(th);
            px=nx*scale+ox; py=ny*scale+oy; x=nx; y=ny;
        } else if(c=='+') th+=rad;
        else if(c=='-') th-=rad;
        else if(c=='[') { stk.push({x,y,th}); px=x*scale+ox; py=y*scale+oy; }
        else if(c==']'&&!stk.empty()) {
            auto s=stk.top();stk.pop();x=s.x;y=s.y;th=s.th;
            px=x*scale+ox; py=y*scale+oy;
        }
    }
}

void LsysPlugin::renderScene(const RenderContext& ctx) {
    SDL_SetRenderDrawColor(ctx.renderer,8,8,18,255);
    SDL_RenderClear(ctx.renderer);
    if(dirty_) generate();
    drawTurtle(ctx.renderer, ctx.width, ctx.height, ctx.topOffset);
}

void LsysPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({300,310},ImGuiCond_Once);
    ImGui::Begin("L-System");

    ImGui::SeparatorText("Preset");
    const char* names[kNumPresets];
    for(int i=0;i<kNumPresets;++i) names[i]=LsysPlugin_presets[i].name;
    if(ImGui::Combo("##pre",&preset_,names,kNumPresets)){
        auto& p2=LsysPlugin_presets[preset_];
        strncpy(axiomBuf_,p2.axiom,sizeof(axiomBuf_)-1);
        strncpy(ruleFBuf_,p2.ruleF,sizeof(ruleFBuf_)-1);
        strncpy(ruleXBuf_,p2.ruleX,sizeof(ruleXBuf_)-1);
        strncpy(ruleYBuf_,p2.ruleY,sizeof(ruleYBuf_)-1);
        levels_=p2.levels; angle_=p2.angle; startAngle_=p2.startAngle;
        dirty_=true;
    }

    ImGui::SeparatorText("Rules");
    if(ImGui::InputText("Axiom",axiomBuf_,sizeof(axiomBuf_))) dirty_=true;
    if(ImGui::InputText("F ->",ruleFBuf_,sizeof(ruleFBuf_))) dirty_=true;
    if(ImGui::InputText("X/A->",ruleXBuf_,sizeof(ruleXBuf_))) dirty_=true;
    if(ImGui::InputText("Y/B/G->",ruleYBuf_,sizeof(ruleYBuf_))) dirty_=true;

    ImGui::SeparatorText("Parameters");
    if(ImGui::SliderInt("levels",&levels_,1,10)) dirty_=true;
    if(ImGui::SliderFloat("angle",  &angle_,1.f,180.f,"%.1f°")) dirty_=true;
    if(ImGui::SliderFloat("start°",&startAngle_,-180.f,180.f,"%.1f°")) dirty_=true;

    ImGui::SeparatorText("Info");
    ImGui::TextDisabled("Length: %zu chars", turtle_.size());
    ImGui::End();
}
