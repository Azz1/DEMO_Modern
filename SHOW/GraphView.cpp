#include "GraphView.hpp"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdio>
#include <cstring>

static void sdlFillCircle(SDL_Renderer* r, int cx, int cy, int rad) {
    for (int y=-rad; y<=rad; ++y) {
        int hw = (int)sqrt((double)(rad*rad - y*y));
        SDL_RenderDrawLine(r, cx-hw, cy+y, cx+hw, cy+y);
    }
}

static const char* kBuiltinGph =
    "# Built-in graph (test.gph)\n"
    "[Nodes]\n"
    "# label  weight  x    y    z\n"
    "x0  0.1  100  100  0\n"
    "x1  0.2  200  100  0\n"
    "x2  0.1  300  100  0\n"
    "x3  0.5  150  200  0\n"
    "x4  0.2  250  200  0\n"
    "x5  0.3  200  300  0\n"
    "[DEdges]\n"
    "# label  from  to  stress\n"
    "e0  x0  x1  0.1\n"
    "e1  x1  x2  0.2\n"
    "e2  x0  x3  0.3\n"
    "e3  x1  x3  0.2\n"
    "e4  x1  x4  0.5\n"
    "e5  x2  x4  0.4\n"
    "e6  x3  x4  0.2\n"
    "e7  x3  x5  0.7\n"
    "e8  x4  x5  0.2\n";

void GraphViewPlugin::setup(SDL_Renderer*) { loadBuiltin(); }

void GraphViewPlugin::loadBuiltin() {
    strncpy(dataBuf_, kBuiltinGph, sizeof(dataBuf_)-1);
    parseData();
}

void GraphViewPlugin::parseData() {
    nodes_.clear(); edges_.clear();
    directed_ = false;
    const char* p = dataBuf_;
    int step = 0;
    char line[256];
    while (*p) {
        // read one line
        int li = 0;
        while (*p && *p!='\n' && *p!='\r' && li<254) line[li++]=*p++;
        while (*p=='\n'||*p=='\r') ++p;
        line[li]=0;
        if (!li || line[0]=='#') continue;
        if (strncmp(line,"[Nodes]",7)==0)  { step=1; continue; }
        if (strncmp(line,"[DEdges]",8)==0) { step=2; directed_=true; continue; }
        if (strncmp(line,"[Edges]",7)==0)  { step=2; continue; }
        if (step==1) {
            GNode n{}; float z=0;
            if (sscanf(line,"%31s %f %f %f %f",n.lbl,&n.weight,&n.x,&n.y,&z)==5)
                nodes_.push_back(n);
        } else if (step==2) {
            GEdge e{}; char l1[32],l2[32];
            if (sscanf(line,"%31s %31s %31s %f",e.lbl,l1,l2,&e.stress)==4) {
                e.n0=e.n1=-1;
                for (int i=0;i<(int)nodes_.size();++i) {
                    if (!strcmp(nodes_[i].lbl,l1)) e.n0=i;
                    if (!strcmp(nodes_[i].lbl,l2)) e.n1=i;
                }
                if (e.n0>=0&&e.n1>=0) edges_.push_back(e);
            }
        }
    }
    dataDirty_ = false;
}

bool GraphViewPlugin::loadFile(const char* path) {
    FILE* fp = fopen(path,"r");
    if (!fp) return false;
    size_t n = fread(dataBuf_, 1, sizeof(dataBuf_)-1, fp);
    dataBuf_[n] = 0;
    fclose(fp);
    parseData();
    return !nodes_.empty();
}

void GraphViewPlugin::drawArrow(SDL_Renderer* r, int x1, int y1, int x2, int y2,
                                 Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r,R,G,B,255);
    SDL_RenderDrawLine(r,x1,y1,x2,y2);
    if (!directed_) return;
    float dx=x2-x1, dy=y2-y1, L=sqrt(dx*dx+dy*dy);
    if (L<1) return;
    float ux=dx/L, uy=dy/L;
    float mx=x1+dx*0.75f, my=y1+dy*0.75f;
    float alpha=0.5f;
    float a1x=mx+(float)(-ux*10+uy*6), a1y=my+(float)(-uy*10-ux*6);
    float a2x=mx+(float)(-ux*10-uy*6), a2y=my+(float)(-uy*10+ux*6);
    SDL_RenderDrawLine(r,(int)mx,(int)my,(int)a1x,(int)a1y);
    SDL_RenderDrawLine(r,(int)mx,(int)my,(int)a2x,(int)a2y);
}

void GraphViewPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_RIGHT) {
        panning_=true; panStartX_=e.button.x; panStartY_=e.button.y;
        panStartOX_=panX_; panStartOY_=panY_;
    }
    if (e.type==SDL_MOUSEBUTTONUP&&e.button.button==SDL_BUTTON_RIGHT) panning_=false;
    if (e.type==SDL_MOUSEMOTION&&panning_) {
        panX_=panStartOX_+(e.motion.x-panStartX_);
        panY_=panStartOY_+(e.motion.y-panStartY_);
    }
    if (e.type==SDL_MOUSEWHEEL) {
        zoom_ *= (e.wheel.y>0) ? 1.1f : 0.9f;
        if (zoom_<0.1f) zoom_=0.1f;
        if (zoom_>8.f)  zoom_=8.f;
    }
}

void GraphViewPlugin::renderScene(const RenderContext& ctx) {
    int W=ctx.width, H=ctx.height, top=ctx.topOffset;
    SDL_SetRenderDrawColor(ctx.renderer,12,12,20,255);
    SDL_RenderClear(ctx.renderer);

    // Center offset
    float ox = W/2.f + panX_;
    float oy = top + (H-top)/2.f + panY_;
    float sc = zoom_;

    // Edges
    for (auto& e : edges_) {
        if (e.n0<0||e.n1<0||(int)nodes_.size()<=e.n0||(int)nodes_.size()<=e.n1) continue;
        auto& n0=nodes_[e.n0]; auto& n1=nodes_[e.n1];
        int x1=(int)(ox+n0.x*sc), y1=(int)(oy-n0.y*sc);
        int x2=(int)(ox+n1.x*sc), y2=(int)(oy-n1.y*sc);
        Uint8 gr=(Uint8)(e.stress*200); 
        drawArrow(ctx.renderer,x1,y1,x2,y2,gr,gr,255);
        if (showLabels_&&e.stress>0) {
            // label at midpoint (skip SDL TTF — use ImGui overlay via DrawList)
        }
    }

    // Nodes
    for (auto& n : nodes_) {
        int cx=(int)(ox+n.x*sc), cy=(int)(oy-n.y*sc);
        int rad=(int)(nodeR_*sc);
        Uint8 gc=(Uint8)(n.weight*200);
        SDL_SetRenderDrawColor(ctx.renderer,0,gc,100,255);
        sdlFillCircle(ctx.renderer,cx,cy,rad);
        SDL_SetRenderDrawColor(ctx.renderer,200,200,255,255);
        // draw circle outline
        int steps=rad*4+16;
        for (int i=0;i<=steps;++i) {
            double a=2.0*M_PI*i/steps;
            SDL_RenderDrawPoint(ctx.renderer,cx+(int)((rad+1)*cos(a)),cy+(int)((rad+1)*sin(a)));
        }
    }

    // ImGui overlay for node labels
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    for (auto& n : nodes_) {
        int cx=(int)(ox+n.x*sc), cy=(int)(oy-n.y*sc);
        if (showLabels_)
            dl->AddText(ImVec2(cx-12,cy-7), IM_COL32(255,255,100,255), n.lbl);
        if (showWeights_) {
            char buf[32]; snprintf(buf,31,"%.1f",n.weight);
            dl->AddText(ImVec2(cx-8,cy+4), IM_COL32(180,255,180,255), buf);
        }
    }
    // Edge stress labels
    if (showLabels_) {
        for (auto& e : edges_) {
            if (e.n0<0||e.n1<0||(int)nodes_.size()<=e.n0||(int)nodes_.size()<=e.n1) continue;
            auto& n0=nodes_[e.n0]; auto& n1=nodes_[e.n1];
            float mx=ox+(n0.x+n1.x)/2.f*sc, my=oy-(n0.y+n1.y)/2.f*sc;
            if (e.stress>0) {
                char buf[32]; snprintf(buf,31,"%s(%.1f)",e.lbl,e.stress);
                dl->AddText(ImVec2(mx,my), IM_COL32(160,160,255,200), buf);
            }
        }
    }
}

void GraphViewPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({310,420},ImGuiCond_Once);
    ImGui::Begin("Graph View");

    ImGui::SeparatorText("Data (.gph format)");
    ImGui::TextDisabled("[Nodes] label w x y z  |  [DEdges]/[Edges] lbl n0 n1 stress");
    if (ImGui::InputTextMultiline("##data", dataBuf_, sizeof(dataBuf_),
                                  ImVec2(-1, 180))) dataDirty_ = true;
    if (dataDirty_) {
        if (ImGui::Button("Apply")) parseData();
        ImGui::SameLine();
    }
    if (ImGui::Button("Reset")) loadBuiltin();
#ifndef __EMSCRIPTEN__
    ImGui::SameLine();
    static char pathBuf[256]="";
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("##path",pathBuf,sizeof(pathBuf));
    ImGui::SameLine();
    if (ImGui::Button("Load File")) loadFile(pathBuf);
#endif

    ImGui::SeparatorText("Display");
    ImGui::Checkbox("Labels",&showLabels_);
    ImGui::SameLine(); ImGui::Checkbox("Weights",&showWeights_);
    ImGui::SameLine(); ImGui::Checkbox("Directed",&directed_);
    ImGui::SliderFloat("Node R",&nodeR_,5.f,40.f);

    ImGui::SeparatorText("Info");
    ImGui::Text("Nodes: %d  Edges: %d",(int)nodes_.size(),(int)edges_.size());
    ImGui::TextDisabled("Scroll=zoom  RMB=pan");
    if (ImGui::Button("Reset View")) { panX_=panY_=0; zoom_=1.f; }
    ImGui::End();
}
