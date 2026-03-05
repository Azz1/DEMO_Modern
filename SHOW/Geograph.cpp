#include "Geograph.hpp"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cstdlib>

static const double kPI = 3.14159265358979;
static const float  kFPS = 1e-6f;

// ── Built-in: generate damped wave (50×50) ────────────────────────────────────
void GeographPlugin::loadBuiltin() {
    m_=50; n_=50; stepx_=stepy_=10.f;
    sx_ = -(n_-1)*stepx_/2.f;
    sy_ = -(m_-1)*stepy_/2.f;
    grid_.assign(m_, std::vector<float>(n_));
    for (int j=0;j<m_;++j) for (int i=0;i<n_;++i) {
        float x=sx_+i*stepx_, y=sy_+j*stepy_;
        float r=sqrt(x*x+y*y);
        grid_[j][i] = (r<kFPS) ? 30.f : 30.f*sinf(r/15.f*kPI)*expf(-r/80.f);
    }
    getExt();
    contours_.clear(); builtContourLevels_=0;
    serializeToText();
    dirty_=true;
}

// Serialize current grid to dataBuf_ text
void GeographPlugin::serializeToText() {
    char* p = dataBuf_;
    const int BUF = (int)sizeof(dataBuf_)-4;
    int used = 0;
    auto W = [&](const char* fmt, auto... args) {
        int n2 = snprintf(p+used, BUF-used, fmt, args...);
        if(n2>0) used+=n2;
    };
    W("# .pan format: title N M stepX stepY, then M rows of N values\n");
    W("Wave %d %d %.2f %.2f\n", n_, m_, stepx_, stepy_);
    for (int j=0;j<m_;++j) {
        for (int i=0;i<n_;++i)
            W(i<n_-1 ? "%.4f\t" : "%.4f\n", grid_[j][i]);
    }
    dataBuf_[used]=0;
    dataDirty_=false;
}

void GeographPlugin::parseData() {
    const char* p = dataBuf_;

    // Helper: skip whitespace (not newlines)
    auto skipSp  = [](const char*& q){ while(*q==' '||*q=='\t') ++q; };
    // Helper: skip to end of line then past newline(s)
    auto skipLine = [](const char*& q){ while(*q&&*q!='\n'&&*q!='\r') ++q;
                                        while(*q=='\n'||*q=='\r') ++q; };

    // 1. Skip comment and blank lines
    while (*p) {
        skipSp(p);
        if (*p=='#') { skipLine(p); continue; }
        if (*p=='\n'||*p=='\r') { skipLine(p); continue; }
        break;
    }

    // 2. Skip title word (read until first whitespace, title and numbers are on same line)
    while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r') ++p;

    // 3. N M stepX stepY — still on the same line as title
    char* end;
    int nn = (int)strtol(p,&end,10); if(end==p) return; p=end;
    int mm = (int)strtol(p,&end,10); if(end==p) return; p=end;
    float sx = strtof(p,&end);       if(end==p) return; p=end;
    float sy = strtof(p,&end);       if(end==p) return; p=end;
    skipLine(p);   // consume rest of header line (newline)

    if(nn<=0||mm<=0||nn>1000||mm>1000) return;
    n_=nn; m_=mm; stepx_=sx; stepy_=sy;
    sx_=-(n_-1)*stepx_/2.f;
    sy_=-(m_-1)*stepy_/2.f;
    grid_.assign(m_, std::vector<float>(n_, 0.f));

    // 4. Read all values with strtof (handles any whitespace/newline separation)
    for(int j=0; j<m_; ++j) {
        for(int i=0; i<n_; ++i) {
            // skip whitespace including newlines
            while(*p==' '||*p=='\t'||*p=='\n'||*p=='\r') ++p;
            if(!*p) break;
            grid_[j][i] = strtof(p, &end);
            if(end==p) break;   // parse error
            p = end;
        }
    }

    getExt();
    contours_.clear(); builtContourLevels_=0;
    dataDirty_=false;
    dirty_=true;
}

bool GeographPlugin::loadFile(const char* path) {
    FILE* fp=fopen(path,"r"); if(!fp) return false;
    size_t n2=fread(dataBuf_,1,sizeof(dataBuf_)-1,fp); dataBuf_[n2]=0;
    fclose(fp);
    parseData();
    return m_>0&&n_>0;
}

void GeographPlugin::getExt() {
    minZ_=maxZ_=grid_[0][0];
    for (auto& row:grid_) for (float v:row) {
        if(v<minZ_)minZ_=v; if(v>maxZ_)maxZ_=v;
    }
    if (maxZ_-minZ_<1.f) maxZ_=minZ_+1.f;
}

void GeographPlugin::loadPreset(int idx) {
    strncpy(dataBuf_, kGeoPresets[idx].data, sizeof(dataBuf_)-1);
    dataBuf_[sizeof(dataBuf_)-1]=0;
    parseData();
    preset_=idx;
}

void GeographPlugin::setup(SDL_Renderer*) { loadPreset(0); }
void GeographPlugin::teardown() { if(tex_){SDL_DestroyTexture(tex_);tex_=nullptr;} }

// ── Color mapping ─────────────────────────────────────────────────────────────
void GeographPlugin::contColour(int op, float h, Uint8&r, Uint8&g, Uint8&b) {
    float M=std::max(fabs(maxZ_),fabs(minZ_)); if(M<kFPS)M=1.f;
    if (op) { r=g=b=(Uint8)(127.f*(h+M)/M); return; }
    if (h<-kFPS)      { r=0;     b=(Uint8)(255.f*(h+M)/M); g=b/2; }
    else if (h>kFPS)  { r=(Uint8)(255.f*h/M); g=120; b=0; }
    else              { r=0; g=120; b=120; }
    r=std::min(r,(Uint8)255); g=std::min(g,(Uint8)255); b=std::min(b,(Uint8)255);
}

// ── 2D heatmap ────────────────────────────────────────────────────────────────
void GeographPlugin::render2DMap(SDL_Renderer* r, int W, int H, int top) {
    int dH=H-top;
    if(!tex_||texW_!=W||texH_!=dH){
        if(tex_)SDL_DestroyTexture(tex_);
        tex_=SDL_CreateTexture(r,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_STREAMING,W,dH);
        texW_=W; texH_=dH;
    }
    void* px; int pitch; SDL_LockTexture(tex_,nullptr,&px,&pitch);
    Uint32* p=(Uint32*)px;
    for(int y=0;y<dH;++y) for(int x=0;x<W;++x) p[y*(pitch/4)+x]=0x101020FF;

    float ex_=sx_+(n_-1)*stepx_, ey_=sy_+(m_-1)*stepy_;
    float ratex=W/(ex_-sx_), ratey=dH/(ey_-sy_);
    float dh=(maxZ_-minZ_)/((levels_==0)?5:levels_);

    for (int j=0;j<m_;++j) for (int i=0;i<n_;++i) {
        float gx=sx_+i*stepx_, gy=sy_+j*stepy_;
        int x0=(int)((gx-sx_)*ratex), y0=(int)(dH-(gy-sy_)*ratey);
        int x1=(int)((gx-sx_+stepx_)*ratex), y1=(int)(dH-(gy-sy_+stepy_)*ratey);
        if(x0>x1)std::swap(x0,x1); if(y1>y0)std::swap(y0,y1);

        float h2=grid_[j][i]; h2=((int)(h2/dh))*dh;
        Uint8 cr,cg,cb; contColour(colorOp_,h2,cr,cg,cb);
        for(int py2=y1;py2<y0&&py2<dH;++py2)
            for(int px2=x0;px2<x1&&px2<W;++px2)
                if(px2>=0&&py2>=0) p[py2*(pitch/4)+px2]=(cr<<24)|(cg<<16)|(cb<<8)|0xFF;
    }
    SDL_UnlockTexture(tex_);
    SDL_Rect dst={0,top,W,dH};
    SDL_RenderCopy(r,tex_,nullptr,&dst);
    dirty_=false;
}

// ── Contour marching squares ──────────────────────────────────────────────────
void GeographPlugin::getI(float h, std::vector<std::vector<short>>& I) {
    I.assign(m_,std::vector<short>(n_,0));
    for(int iy=0;iy<m_;++iy){
        for(int ix=0;ix<n_-1;++ix)
            if((grid_[iy][ix]<h)!=(grid_[iy][ix+1]<h)) I[iy][ix]=1; else I[iy][ix]=0;
        I[iy][n_-1]=0;
    }
    for(int iy=0;iy<m_-1;++iy)
        for(int ix=0;ix<n_;++ix)
            if((grid_[iy][ix]<h)!=(grid_[iy+1][ix]<h)) I[iy][ix]+=2;
}

void GeographPlugin::getP(float h,int ix,int iy,int v,float&x,float&y){
    if(v==1){
        float dz=grid_[iy][ix+1]-grid_[iy][ix];
        float t=(fabs(dz)>kFPS)?(h-grid_[iy][ix])/dz:0.f;
        x=sx_+(ix+t)*stepx_; y=sy_+iy*stepy_;
    } else {
        float dz=grid_[iy+1][ix]-grid_[iy][ix];
        float t=(fabs(dz)>kFPS)?(h-grid_[iy][ix])/dz:0.f;
        x=sx_+ix*stepx_; y=sy_+(iy+t)*stepy_;
    }
}

int GeographPlugin::extP(float h,int&ix,int&iy,int&v,float&x,float&y,
                          std::vector<std::vector<short>>&I){
    int v0=v;
    auto tryV=[&](int nix,int niy,int nv)->bool{
        if(nix<0||nix>=n_||niy<0||niy>=m_) return false;
        short bits=I[niy][nix];
        if(nv==1&&(bits&1)){I[niy][nix]&=~1;ix=nix;iy=niy;v=1;getP(h,ix,iy,v,x,y);return true;}
        if(nv==2&&(bits&2)){I[niy][nix]&=~2;ix=nix;iy=niy;v=2;getP(h,ix,iy,v,x,y);return true;}
        return false;
    };
    if(v==1){
        if(tryV(ix,iy,2)||tryV(ix+1,iy,2)||tryV(ix+1,iy-1,2)||tryV(ix,iy-1,2)||
           tryV(ix,iy-1,1)||(iy<m_-1&&tryV(ix,iy+1,1))) return 1;
    } else {
        if(tryV(ix,iy,1)||tryV(ix,iy+1,1)||tryV(ix-1,iy+1,1)||tryV(ix-1,iy,2)||
           (ix<n_-1&&tryV(ix+1,iy,2))) return 1;
    }
    v=v0; return 0;
}

void GeographPlugin::getCurve(float h,int idrec,int ix,int iy,int v,
                               fLevel&lev,std::vector<std::vector<short>>&I){
    fCurve c; float x,y;
    if(v==1) I[iy][ix]&=~1; else I[iy][ix]&=~2;
    getP(h,ix,iy,v,x,y); c.push_back({x,y});
    while(extP(h,ix,iy,v,x,y,I)) c.push_back({x,y});
    if(!c.empty()) lev.push_back(c);
}

void GeographPlugin::contourh(float h, fLevel& lev) {
    std::vector<std::vector<short>> I;
    getI(h,I);
    // boundary edges
    for(int ix=0;ix<n_;++ix){ int v=I[0][ix]&1; if(v) getCurve(h,1,ix,0,v,lev,I); }
    for(int iy=0;iy<m_;++iy){ if(I[iy][n_-1]&2) getCurve(h,2,n_-1,iy,2,lev,I); }
    for(int ix=0;ix<n_;++ix){ if(I[m_-1][ix]&1) getCurve(h,3,ix,m_-1,1,lev,I); }
    for(int iy=0;iy<m_;++iy){ int v=I[iy][0]&2; if(v) getCurve(h,4,0,iy,v,lev,I); }
    // interior
    for(int iy=0;iy<m_;++iy) for(int ix=0;ix<n_;++ix){
        if(I[iy][ix]&2) getCurve(h,5,ix,iy,2,lev,I);
        if(I[iy][ix]&1) getCurve(h,5,ix,iy,1,lev,I);
    }
}

void GeographPlugin::buildContour(int lev) {
    if(lev==builtContourLevels_&&!contours_.empty()) return;
    contours_.clear();
    if(lev<1||m_<=0||n_<=0) return;
    float h=minZ_, dh=(maxZ_-minZ_)/lev;
    if(dh<kFPS) dh=kFPS;
    while(h<=maxZ_+kFPS){
        fLevel fl; contourh(h,fl);
        contours_.push_back(fl);
        h+=dh;
    }
    builtContourLevels_=lev;
}

// ── 2D contour draw ───────────────────────────────────────────────────────────
void GeographPlugin::render2DContour(SDL_Renderer* r, int W, int H, int top) {
    buildContour(levels_);
    int dH=H-top;
    float ex2=sx_+(n_-1)*stepx_, ey2=sy_+(m_-1)*stepy_;
    float rate=(float)dH/(ey2-sy_);
    float ratex=(float)W/(ex2-sx_);

    SDL_SetRenderDrawColor(r,10,10,20,255);
    SDL_RenderClear(r);

    float dh=(maxZ_-minZ_)/levels_;
    float hz=minZ_;
    for(auto& fl : contours_){
        Uint8 cr,cg,cb; contColour(colorOp_,hz,cr,cg,cb);
        SDL_SetRenderDrawColor(r,cr,cg,cb,255);
        for(auto& fc:fl){
            for(int k=0;k<(int)fc.size()-1;++k){
                int x1=(int)((fc[k][0]-sx_)*ratex);
                int y1=(int)(dH-(fc[k][1]-sy_)*rate)+top;
                int x2=(int)((fc[k+1][0]-sx_)*ratex);
                int y2=(int)(dH-(fc[k+1][1]-sy_)*rate)+top;
                SDL_RenderDrawLine(r,x1,y1,x2,y2);
            }
        }
        hz+=dh;
    }
    dirty_=false;
}

// ── 3D projection ─────────────────────────────────────────────────────────────
void GeographPlugin::initProj(int W, int H) {
    float ox=0,oy=0,oz=0;
    float th=theta_*kPI/180.f, rz=rz_*kPI/180.f;
    proj_.d=ox; proj_.e=oy; proj_.f=oz;
    proj_.a=radiu_*cos(rz)*sin(th);
    proj_.b=radiu_*sin(rz)*sin(th);
    proj_.c=radiu_*cos(th);
    proj_.v=sqrt(proj_.a*proj_.a+proj_.b*proj_.b);
    proj_.u=sqrt(proj_.a*proj_.a+proj_.b*proj_.b+proj_.c*proj_.c);
    proj_.D=depF_;
    proj_.Vsx=W/2; proj_.Vsy=(H/2);
    proj_.valid=(proj_.u>1e-4&&proj_.v>1e-4&&proj_.D>1e-4);
    // Eye position in grid-index coords (always available after initProj)
    eyeGX_=((float)proj_.a-sx_)/stepx_;
    eyeGY_=((float)proj_.b-sy_)/stepy_;
    eyeGZ_=(float)proj_.c;
}

bool GeographPlugin::trans3D(float wx,float wy,float wz,int&sx2,int&sy2){
    if(!proj_.valid) return false;
    double xw=wx-proj_.d, yw=wy-proj_.e, zw=wz-proj_.f;
    double ab=proj_.a*xw+proj_.b*yw;
    double xe=(-proj_.b*xw+proj_.a*yw)/proj_.v;
    double ye=(-ab*proj_.c+zw*proj_.v*proj_.v)/(proj_.u*proj_.v);
    double ze=(-ab-proj_.c*zw+proj_.u*proj_.u)/proj_.u;
    if(ze<1e-4) return false;
    double w2=ze/proj_.D;
    sx2=(int)(xe/w2)+proj_.Vsx;
    sy2=(int)(ye/w2)+proj_.Vsy;
    return true;
}

// ── Visibility: port of original pvisible / getposi / faceeq ────────────────

// Plane through (ix,iy,iz)→(tx,ty,tz)
void GeographPlugin::faceEq(float ix,float iy,float iz,
                             float tx,float ty,float tz,float feq[4]){
    float mx=tx-ix,my=ty-iy,mz=tz-iz,mz2=mz*mz;
    feq[0]=-mz2*mx; feq[1]=-mz2*my;
    feq[2]=(mx*mx+my*my)*mz;
    feq[3]=-(feq[0]*ix+feq[1]*iy+feq[2]*iz);
}

// Sign of (x,y,z) relative to plane feq
int GeographPlugin::relpf(float x,float y,float z,const float feq[4]){
    float v=feq[0]*x+feq[1]*y+feq[2]*z+feq[3];
    return (v>1e-5f)?1:(v<-1e-5f)?-1:0;
}

// Sample terrain at grid crossing idx; wh=0→column crossing, wh=1→row crossing
bool GeographPlugin::getPosi(int wh,int idx,float ix,float iy,float tanv,
                              const float feq[4],int& pos) const {
    float v,rate,zs; int fr;
    if(!wh){
        v=iy+tanv*(idx-ix); fr=(int)floorf(v);
        if(fr<0||fr>=m_-1) return false;
        rate=v-fr; zs=grid_[fr][idx]+rate*(grid_[fr+1][idx]-grid_[fr][idx]);
        pos=relpf((float)idx,v,zs,feq);
    } else {
        v=ix+tanv*(idx-iy); fr=(int)floorf(v);
        if(fr<0||fr>=n_-1) return false;
        rate=v-fr; zs=grid_[idx][fr]+rate*(grid_[idx][fr+1]-grid_[idx][fr]);
        pos=relpf(v,(float)idx,zs,feq);
    }
    return true;
}

// Port of pvisible: ray-march from (gx,gy,gz) toward eye in grid-index space
bool GeographPlugin::isVisible(float gx,float gy,float gz) const {
    float tox=eyeGX_, toy=eyeGY_, toz=eyeGZ_;
    float feq[4]; faceEq(gx,gy,gz,tox,toy,toz,feq);

    int upx=0,upy=0, bx=0,dx=0,by=0,dy=0;
    float tany=0,tanx=0;

    if(fabsf(tox-gx)>1.f){
        if(tox>gx){
            upx=1; bx=(int)ceilf(gx);
            if(fabsf(gx-bx)<=kFPS) bx++;
            dx=(int)floorf(tox); if(dx>n_-1)dx=n_-1;
            if(bx>dx) upx=0;
        } else {
            upx=-1; bx=(int)floorf(gx);
            if(fabsf(gx-bx)<=kFPS) bx--;
            dx=(int)ceilf(tox); if(dx<0)dx=0;
            if(bx<dx) upx=0;
        }
        if(upx) tany=(toy-gy)/(tox-gx);
    }
    if(fabsf(toy-gy)>1.f){
        if(toy>gy){
            upy=1; by=(int)ceilf(gy);
            if(fabsf(gy-by)<=kFPS) by++;
            dy=(int)floorf(toy); if(dy>m_-1)dy=m_-1;
            if(by>dy) upy=0;
        } else {
            upy=-1; by=(int)floorf(gy);
            if(fabsf(gy-by)<=kFPS) by--;
            dy=(int)ceilf(toy); if(dy<0)dy=0;
            if(by<dy) upy=0;
        }
        if(upy) tanx=(tox-gx)/(toy-gy);
    }

    int up=-2,down=2,pos;
    if(upx){
        for(int i=bx;i!=dx+upx;i+=upx){
            if(!getPosi(0,i,gx,gy,tany,feq,pos)) break;
            if(pos>up)up=pos; if(pos<down)down=pos;
            if(down<=0&&up>=0) return false;
        }
    }
    if(upy){
        for(int i=by;i!=dy+upy;i+=upy){
            if(!getPosi(1,i,gx,gy,tanx,feq,pos)) break;
            if(pos>up)up=pos; if(pos<down)down=pos;
            if(down<=0&&up>=0) return false;
        }
    }
    int fx=(int)floorf(tox),fy=(int)floorf(toy);
    if(fx>=0&&fx<n_-1&&fy>=0&&fy<m_-1){
        float dd=(grid_[fy][fx]+grid_[fy][fx+1]+grid_[fy+1][fx]+grid_[fy+1][fx+1])*0.25f;
        pos=relpf(tox,toy,dd,feq);
        if(pos>up)up=pos; if(pos<down)down=pos;
        if(down<=0&&up>=0) return false;
    }
    return true;
}

// ── 3D wireframe ─────────────────────────────────────────────────────────────
void GeographPlugin::render3DWire(SDL_Renderer* r, int W, int H, int top) {
    int dH=H-top;
    initProj(W,dH);
    SDL_SetRenderDrawColor(r,8,8,18,255);
    SDL_RenderClear(r);

    for(int j=0;j<m_;++j) for(int i=0;i<n_-1;++i){
        float x0=sx_+i*stepx_,  y0=sy_+j*stepy_;
        float x1=sx_+(i+1)*stepx_, y1=sy_+j*stepy_;
        int sx0,sy0,sx1,sy1;
        if(trans3D(x0,y0,grid_[j][i],sx0,sy0)&&
           trans3D(x1,y1,grid_[j][i+1],sx1,sy1)){
            sy0+=top; sy1+=top;
            Uint8 cr,cg,cb; contColour(colorOp_,grid_[j][i],cr,cg,cb);
            SDL_SetRenderDrawColor(r,cr,cg,cb,255);
            SDL_RenderDrawLine(r,sx0,sy0,sx1,sy1);
        }
    }
    for(int j=0;j<m_-1;++j) for(int i=0;i<n_;++i){
        float x0=sx_+i*stepx_, y0=sy_+j*stepy_;
        float x1=sx_+i*stepx_, y1=sy_+(j+1)*stepy_;
        int sx0,sy0,sx1,sy1;
        if(trans3D(x0,y0,grid_[j][i],sx0,sy0)&&
           trans3D(x1,y1,grid_[j+1][i],sx1,sy1)){
            sy0+=top; sy1+=top;
            Uint8 cr,cg,cb; contColour(colorOp_,grid_[j][i],cr,cg,cb);
            SDL_SetRenderDrawColor(r,cr,cg,cb,255);
            SDL_RenderDrawLine(r,sx0,sy0,sx1,sy1);
        }
    }
    dirty_=false;
}

// ── 3D with true hidden surface (port of DrawPane op=1) ─────────────────────
void GeographPlugin::render3DHidden(SDL_Renderer* r, int W, int H, int top) {
    int dH=H-top;
    initProj(W,dH);  // also sets eyeGX_/eyeGY_/eyeGZ_

    // Pre-compute screen positions and ze for all grid points
    struct SP { int sx,sy; float ze; bool front; };
    std::vector<std::vector<SP>> sc(m_,std::vector<SP>(n_));
    for(int j=0;j<m_;++j) for(int i=0;i<n_;++i){
        float wx=sx_+i*stepx_,wy=sy_+j*stepy_,wz=grid_[j][i];
        double xw=wx,yw=wy,zw=wz;   // proj_.d/e/f=0
        float ze=(float)((-proj_.a*xw-proj_.b*yw-proj_.c*zw+proj_.u*proj_.u)/proj_.u);
        sc[j][i].ze=ze; sc[j][i].front=(ze>0.1f);
        if(sc[j][i].front){
            trans3D(wx,wy,wz,sc[j][i].sx,sc[j][i].sy);
            sc[j][i].sy+=top;
        }
    }

    SDL_SetRenderDrawColor(r,8,8,18,255);
    SDL_RenderClear(r);

    // Visibility cache (per grid point, lazy evaluated)
    std::vector<std::vector<int8_t>> visCache(m_,std::vector<int8_t>(n_,-1));
    auto getVis=[&](int i,int j)->bool{
        if(visCache[j][i]>=0) return visCache[j][i];
        bool v=sc[j][i].front && isVisible((float)i,(float)j,grid_[j][i]);
        visCache[j][i]=(int8_t)v;
        return v;
    };

    auto drawSeg=[&](int i0,int j0,int i1,int j1){
        bool v0=getVis(i0,j0),v1=getVis(i1,j1);
        if(!v0&&!v1) return;
        float h=(grid_[j0][i0]+grid_[j1][i1])*0.5f;
        Uint8 cr,cg,cb; contColour(colorOp_,h,cr,cg,cb);
        SDL_SetRenderDrawColor(r,cr,cg,cb,255);
        if(v0&&v1){
            SDL_RenderDrawLine(r,sc[j0][i0].sx,sc[j0][i0].sy,
                                 sc[j1][i1].sx,sc[j1][i1].sy);
        } else {
            // Binary search for visibility boundary (port of Divedge)
            float ax=i0,ay=j0,az=grid_[j0][i0];
            float bx=i1,by=j1,bz=grid_[j1][i1];
            if(!v0){std::swap(ax,bx);std::swap(ay,by);std::swap(az,bz);}
            // ax,ay = visible end; bx,by = hidden end
            for(int k=0;k<8;++k){
                float mx=(ax+bx)*0.5f,my=(ay+by)*0.5f;
                // bilinear z at midpoint
                int gi=(int)mx,gj=(int)my; gi=std::max(0,std::min(gi,n_-2));gj=std::max(0,std::min(gj,m_-2));
                float tx=mx-gi,ty=my-gj;
                float mz=grid_[gj][gi]*(1-tx)*(1-ty)+grid_[gj][gi+1]*tx*(1-ty)
                        +grid_[gj+1][gi]*(1-tx)*ty  +grid_[gj+1][gi+1]*tx*ty;
                if(isVisible(mx,my,mz)){ax=mx;ay=my;az=mz;}
                else                  {bx=mx;by=my;bz=mz;}
            }
            // Draw from visible start to boundary
            int sxA,syA,sxB,syB;
            float wxA=sx_+ax*stepx_,wyA=sy_+ay*stepy_;
            float wxB=sx_+bx*stepx_,wyB=sy_+by*stepy_;
            if(trans3D(wxA,wyA,az,sxA,syA)&&trans3D(wxB,wyB,bz,sxB,syB))
                SDL_RenderDrawLine(r,sxA,syA+top,sxB,syB+top);
        }
    };

    // Horizontal lines (along i for each row j)
    for(int j=0;j<m_;++j)
        for(int i=0;i<n_-1;++i) drawSeg(i,j,i+1,j);
    // Vertical lines (along j for each col i)
    for(int i=0;i<n_;++i)
        for(int j=0;j<m_-1;++j) drawSeg(i,j,i,j+1);

    dirty_=false;
}

// ── 3D + contour ─────────────────────────────────────────────────────────────
void GeographPlugin::render3DCont(SDL_Renderer* r, int W, int H, int top) {
    buildContour(levels_);
    int dH=H-top;
    initProj(W,dH);
    SDL_SetRenderDrawColor(r,8,8,18,255);
    SDL_RenderClear(r);

    float dh=(maxZ_-minZ_)/levels_;
    float hz=minZ_;
    for(auto& fl:contours_){
        Uint8 cr,cg,cb; contColour(colorOp_,hz,cr,cg,cb);
        SDL_SetRenderDrawColor(r,cr,cg,cb,255);
        for(auto& fc:fl){
            for(int k=0;k<(int)fc.size()-1;++k){
                // Find z by bilinear interpolation in grid
                auto getZ=[&](float x,float y)->float{
                    int i=(int)((x-sx_)/stepx_), j=(int)((y-sy_)/stepy_);
                    i=std::max(0,std::min(i,n_-2)); j=std::max(0,std::min(j,m_-2));
                    float tx=(x-sx_-i*stepx_)/stepx_, ty=(y-sy_-j*stepy_)/stepy_;
                    return grid_[j][i]*(1-tx)*(1-ty)+grid_[j][i+1]*tx*(1-ty)
                          +grid_[j+1][i]*(1-tx)*ty  +grid_[j+1][i+1]*tx*ty;
                };
                float z0=getZ(fc[k][0],fc[k][1]), z1=getZ(fc[k+1][0],fc[k+1][1]);
                int sx0,sy0,sx1,sy1;
                if(trans3D(fc[k][0],fc[k][1],z0,sx0,sy0)&&
                   trans3D(fc[k+1][0],fc[k+1][1],z1,sx1,sy1))
                    SDL_RenderDrawLine(r,sx0+0,sy0+top,sx1+0,sy1+top);
            }
        }
        hz+=dh;
    }
    dirty_=false;
}

// ── 3D contour + hidden surface (ray-march occlusion per contour point) ──────
void GeographPlugin::render3DContHidden(SDL_Renderer* r, int W, int H, int top) {
    buildContour(levels_);
    int dH=H-top;
    initProj(W,dH);   // sets eyeGX_/GY_/GZ_

    SDL_SetRenderDrawColor(r,8,8,18,255);
    SDL_RenderClear(r);

    // Bilinear z at any world (x,y)
    auto getZ=[&](float x,float y)->float{
        int i=(int)((x-sx_)/stepx_), j=(int)((y-sy_)/stepy_);
        i=std::max(0,std::min(i,n_-2)); j=std::max(0,std::min(j,m_-2));
        float tx=(x-sx_-i*stepx_)/stepx_, ty=(y-sy_-j*stepy_)/stepy_;
        return grid_[j][i]*(1-tx)*(1-ty)+grid_[j][i+1]*tx*(1-ty)
              +grid_[j+1][i]*(1-tx)*ty  +grid_[j+1][i+1]*tx*ty;
    };

    // World (x,y,z) → grid-index (gx,gy,gz) for visibility test
    auto wToG=[&](float x,float y,float z,float&gx,float&gy,float&gz){
        gx=(x-sx_)/stepx_; gy=(y-sy_)/stepy_; gz=z;
    };

    float dh=(maxZ_-minZ_)/levels_;
    float hz=minZ_;

    for(auto& fl:contours_){
        Uint8 cr2,cg2,cb2; contColour(colorOp_,hz,cr2,cg2,cb2);
        SDL_SetRenderDrawColor(r,cr2,cg2,cb2,255);

        for(auto& fc:fl){
            for(int k=0;k<(int)fc.size()-1;++k){
                float ax=fc[k][0],  ay=fc[k][1],  az=getZ(ax,ay);
                float bx=fc[k+1][0],by=fc[k+1][1],bz=getZ(bx,by);
                float agx,agy,agz, bgx,bgy,bgz;
                wToG(ax,ay,az,agx,agy,agz);
                wToG(bx,by,bz,bgx,bgy,bgz);

                bool va=isVisible(agx,agy,agz);
                bool vb=isVisible(bgx,bgy,bgz);

                if(!va&&!vb) continue;

                if(va&&vb){
                    // Both visible: project and draw
                    int sx0,sy0,sx1,sy1;
                    if(trans3D(ax,ay,az,sx0,sy0)&&trans3D(bx,by,bz,sx1,sy1))
                        SDL_RenderDrawLine(r,sx0,sy0+top,sx1,sy1+top);
                } else {
                    // One hidden: binary search for boundary (Divedge-style)
                    float vx=ax,vy=ay,vz=az;   // visible end (world)
                    float hx=bx,hy=by,hz2=bz;  // hidden end
                    float vgx=agx,vgy=agy,vgz=agz;
                    if(!va){
                        std::swap(vx,hx);std::swap(vy,hy);std::swap(vz,hz2);
                        std::swap(vgx,bgx);std::swap(vgy,bgy);std::swap(vgz,bgz);
                    }
                    for(int s=0;s<8;++s){
                        float mx=(vx+hx)*0.5f,my=(vy+hy)*0.5f;
                        float mz=getZ(mx,my);
                        float mgx,mgy,mgz; wToG(mx,my,mz,mgx,mgy,mgz);
                        if(isVisible(mgx,mgy,mgz)){vx=mx;vy=my;vz=mz;}
                        else                       {hx=mx;hy=my;hz2=mz;}
                    }
                    int sx0,sy0,sx1,sy1;
                    if(trans3D(vx,vy,vz,sx0,sy0)&&trans3D(hx,hy,hz2,sx1,sy1))
                        SDL_RenderDrawLine(r,sx0,sy0+top,sx1,sy1+top);
                }
            }
        }
        hz+=dh;
    }
    dirty_=false;
}

// ── Color bar ─────────────────────────────────────────────────────────────────
void GeographPlugin::drawColorBar(SDL_Renderer* r, int W, int H, int top) {
    int barH=H-top-20, barX=W-18, steps=barH;
    for(int i=0;i<steps;++i){
        float h2=maxZ_-(maxZ_-minZ_)*i/steps;
        Uint8 cr,cg,cb; contColour(colorOp_,h2,cr,cg,cb);
        SDL_SetRenderDrawColor(r,cr,cg,cb,255);
        SDL_RenderDrawLine(r,barX,top+i,barX+14,top+i);
    }
}

// ── renderScene ───────────────────────────────────────────────────────────────
void GeographPlugin::renderScene(const RenderContext& ctx) {
    int W=ctx.width, H=ctx.height, top=ctx.topOffset;
    switch(mode_){
        case 0: render2DMap    (ctx.renderer,W,H,top); break;
        case 1: render2DContour(ctx.renderer,W,H,top); break;
        case 2: render3DWire   (ctx.renderer,W,H,top); break;
        case 3: render3DHidden (ctx.renderer,W,H,top); break;
        case 4: render3DCont      (ctx.renderer,W,H,top); break;
        case 5: render3DContHidden(ctx.renderer,W,H,top); break;
    }
    drawColorBar(ctx.renderer,W,H,top);

    // Axis labels
    ImDrawList* dl=ImGui::GetBackgroundDrawList();
    char buf[32];
    snprintf(buf,31,"%.1f",maxZ_); dl->AddText(ImVec2(W-52,top+4),IM_COL32(255,255,255,200),buf);
    snprintf(buf,31,"%.1f",minZ_); dl->AddText(ImVec2(W-52,H-18),IM_COL32(255,255,255,200),buf);
}

// ── Event: RMB drag to rotate 3D ─────────────────────────────────────────────
void GeographPlugin::handleEvent(const SDL_Event& e, const RenderContext& ctx) {
    if(ImGui::GetIO().WantCaptureMouse) return;
    if(mode_<2||ImGui::GetIO().WantCaptureMouse) return;
    if(e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT){
        pan_=true; panSX_=e.button.x; panSY_=e.button.y;
        panBaseTheta_=theta_; panBaseRz_=rz_;
    }
    if(e.type==SDL_MOUSEBUTTONUP&&e.button.button==SDL_BUTTON_LEFT) pan_=false;
    if(e.type==SDL_MOUSEMOTION&&pan_){
        float dx=e.motion.x-panSX_, dy=e.motion.y-panSY_;
        rz_   =panBaseRz_   - dx*0.4f;
        theta_=std::max(5.f,std::min(175.f,panBaseTheta_+dy*0.3f));
        dirty_=true;
    }
    if(e.type==SDL_MOUSEWHEEL){
        radiu_=std::max(50.f, radiu_-e.wheel.y*20.f);
        dirty_=true;
    }
}

// ── renderUI ─────────────────────────────────────────────────────────────────
void GeographPlugin::renderUI() {
    ImGui::SetNextWindowPos({10,35},ImGuiCond_Once);
    ImGui::SetNextWindowSize({280,340},ImGuiCond_Once);
    ImGui::Begin("Geograph");

    ImGui::SeparatorText("Preset");
    {
        const char* names[kNumGeoPresets];
        for(int k=0;k<kNumGeoPresets;++k) names[k]=kGeoPresets[k].name;
        if(ImGui::Combo("##pre",&preset_,names,kNumGeoPresets))
            loadPreset(preset_);
    }

    ImGui::SeparatorText("Data (.pan format)");
    ImGui::TextDisabled("title N M stepX stepY  (same line), then M rows of N values");
    if(ImGui::InputTextMultiline("##data",dataBuf_,sizeof(dataBuf_),
                                 ImVec2(-1,100))) dataDirty_=true;
    if(dataDirty_){
        if(ImGui::Button("Apply")) parseData();
        ImGui::SameLine();
    }
    if(ImGui::Button("Reset")) { if(preset_>=0)loadPreset(preset_); else loadBuiltin(); }
#ifndef __EMSCRIPTEN__
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110);
    ImGui::InputText("##f",fileBuf_,sizeof(fileBuf_));
    ImGui::SameLine();
    if(ImGui::Button("Load")) { if(!loadFile(fileBuf_)) loadPreset(0); }
#endif
    ImGui::Text("Grid: %d×%d  Z: %.1f~%.1f",n_,m_,minZ_,maxZ_);

    ImGui::SeparatorText("Mode");
    ImGui::RadioButton("2D Map",    &mode_,0); ImGui::SameLine();
    ImGui::RadioButton("2D Cont.",  &mode_,1);
    ImGui::RadioButton("3D Wire",      &mode_,2); ImGui::SameLine();
    ImGui::RadioButton("3D Solid",     &mode_,3);
    ImGui::RadioButton("3D+Cont",      &mode_,4); ImGui::SameLine();
    ImGui::RadioButton("3D+Cont(Hide)",&mode_,5);

    ImGui::SeparatorText("Parameters");
    if(ImGui::SliderInt("Levels",&levels_,2,300)){contours_.clear();builtContourLevels_=0;dirty_=true;}
    ImGui::RadioButton("Color",&colorOp_,0); ImGui::SameLine();
    ImGui::RadioButton("Gray", &colorOp_,1);

    if(mode_>=2){
        ImGui::SeparatorText("3D View");
        if(ImGui::SliderFloat("Elevation",&theta_,5.f,175.f)) dirty_=true;
        if(ImGui::SliderFloat("Azimuth",  &rz_,  -180.f,180.f)) dirty_=true;
        if(ImGui::SliderFloat("Distance", &radiu_,100.f,2000.f)) dirty_=true;
        if(ImGui::SliderFloat("Focal",    &depF_, 50.f,1000.f)) dirty_=true;
        if(ImGui::Button("Reset View")){theta_=45;rz_=30;radiu_=800;depF_=400;dirty_=true;}
        ImGui::TextDisabled("LMB drag = rotate");
    }

    ImGui::End();
}
