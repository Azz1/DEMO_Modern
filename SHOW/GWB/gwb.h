/* gwb.h — standalone (no Windows), adapted from original GWB/Gwb.h */
#ifndef _GWB_H
#define _GWB_H 1

#include "gwb_mat.h"
#include "gwb_3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C extern
#endif

/* Windows stubs */
#ifndef _WIN32
  #define MB_OK 0
#endif

typedef int             Id;
typedef struct solid    Solid;
typedef struct face     Face;
typedef struct loop     Loop;
typedef struct halfedge HalfEdge;
typedef struct vertex   Vertex;
typedef struct edge     Edge;
typedef union  node     Node;

struct solid    { Id solidno; Face *sfaces; Edge *sedges; Vertex *sverts; Solid *nexts; Solid *prevs; };
struct face     { Id faceno; Solid *fsolid; Loop *flout; Loop *floops; vector feq; Face *nextf; Face *prevf; };
struct loop     { HalfEdge *ledg; Face *lface; Loop *nextl; Loop *prevl; };
struct edge     { HalfEdge *he1; HalfEdge *he2; Edge *nexte; Edge *preve; };
struct halfedge { Edge *edg; Vertex *vex; Loop *wloop; HalfEdge *nxt; HalfEdge *prv; };
struct vertex   { Id vertexno; HalfEdge *vedge; Vertex *vex; vector vcoord; Vertex *nextv; Vertex *prevv; };
union  node     { Solid s; Face f; Loop l; HalfEdge h; Vertex v; Edge e; };

typedef struct eulerop EulerOp;
struct eulerop { char opcode; Id solidno; Id ip1,ip2,ip3,ip4,ip5,ip6;
                 double fp1,fp2,fp3,fp4; EulerOp *opnext; };

extern EulerOp *OpHead;
extern int      OpCount;
extern int      Generatelog;
extern char     msg[100];

/* Constants */
#define SOLID    0
#define FACE     1
#define LOOP     2
#define HALFEDGE 3
#define EDGE     4
#define VERTEX   5
#define X 0
#define Y 1
#define Z 2
#define PLUS  0
#define MINUS 1

#define mate(he) (((he)==(he)->edg->he1)?(he)->edg->he2:(he)->edg->he1)

/* UnDo opcodes */
#define MVFS 0
#define KVFS 1
#define MEV  2
#define KEV  3
#define MEF  4
#define KEF  5
#define KEMR 6
#define MEKR 7
#define KFMRH 8
#define MFKRH 9
#define RINGMV 10
#define REVERT 11
#define MODIFYNAMES 12
#define MOVEFAC 13
#define ERROR   -2

/* Global variables */
extern Solid *firsts;
extern Id     maxs, maxf, maxv;

/* Core allocators */
EXTERN_C void      addlist(int what, Node *which, Node *where);
EXTERN_C void      dellist(int what, Node *which, Node *where);
EXTERN_C Node    * New(int what, Node *where);
EXTERN_C void      Del(int what, Node *which, Node *where);
EXTERN_C HalfEdge* addhe(Edge *e, Vertex *v, HalfEdge *where, int sign);
EXTERN_C HalfEdge* delhe(HalfEdge *where);
EXTERN_C void      Remove(Solid *s);
EXTERN_C void      RemoveAll(void);
EXTERN_C void      SetFirsts(Solid *f);
EXTERN_C Solid   * GetFirsts(void);

/* Basic Euler operators */
EXTERN_C Solid   * mvfs(Id s, Id f, Id v, double x, double y, double z);
EXTERN_C void      lmev(HalfEdge *he1, HalfEdge *he2, Id vn, double x, double y, double z);
EXTERN_C Face    * lmef(HalfEdge *he1, HalfEdge *he2, Id f);
EXTERN_C void      lkemr(HalfEdge *h1, HalfEdge *h2);
EXTERN_C void      lkvfs(Solid *s);
EXTERN_C void      lkev(HalfEdge *he1, HalfEdge *he2);
EXTERN_C void      lkef(HalfEdge *he1, HalfEdge *he2);
EXTERN_C void      lmekr(HalfEdge *he1, HalfEdge *he2);
EXTERN_C void      lkfmrh(Face *fac1, Face *fac2);
EXTERN_C Face    * lmfkrh(Loop *l, Id f);
EXTERN_C void      lringmv(Loop *l, Face *tofac, int inout);
EXTERN_C void      laringmv(Face *f1, Face *f2);
EXTERN_C Solid   * getsolid(Id sn);
EXTERN_C Face    * fface(Solid *s, Id fn);
EXTERN_C HalfEdge* fhe(Face *f, Id vn1, Id vn2);

/* Advanced Euler */
EXTERN_C int       kvfs(Id s);
EXTERN_C int       mev(Id s,Id f1,Id f2,Id v1,Id v2,Id v3,Id v4,double x,double y,double z);
EXTERN_C int       smev(Id s,Id f1,Id v1,Id v4,double x,double y,double z);
EXTERN_C int       kev(Id s,Id f,Id v1,Id v2);
EXTERN_C int       mef(Id s,Id f1,Id v1,Id v2,Id v3,Id v4,Id f2);
EXTERN_C int       smef(Id s,Id f1,Id v1,Id v3,Id f2);
EXTERN_C int       kef(Id s,Id f,Id v1,Id v2);
EXTERN_C int       mekr(Id s,Id f,Id v1,Id v2,Id v3,Id v4);
EXTERN_C int       smekr(Id s,Id f,Id v1,Id v3);
EXTERN_C int       kemr(Id s,Id f,Id v1,Id v2);
EXTERN_C int       kfmrh(Id s,Id f1,Id f2);
EXTERN_C int       mfkrh(Id s,Id f1,Id v1,Id v2,Id f2);
EXTERN_C int       ringmv(Solid *s,Id f1,Id f2,Id v1,Id v2,int inout);

/* Advanced solid builders */
EXTERN_C void      getmaxnames(Id sn);
EXTERN_C void      GArc(Id s,Id f,Id v,double cx,double cy,double rad,double h,double phi1,double phi2,int n);
EXTERN_C Solid   * GCircle(Id sn,double cx,double cy,double rad,double h,int n);
EXTERN_C void      sweep(Face *fac,double dx,double dy,double dz);
EXTERN_C Solid   * Gblock(Id sn,double dx,double dy,double dz);
EXTERN_C Solid   * Gcyl(Id sn,double rad,double h,double n);
EXTERN_C void      rsweep(Solid *s,int nfaces);
EXTERN_C Solid   * GBall(Id sn,double r,int nver,int nhor);
EXTERN_C void      glue(Solid *s1,Solid *s2,Face *f1,Face *f2);
EXTERN_C void      merge(Solid *s1,Solid *s2);
EXTERN_C void      loopglue(Face *fac);
EXTERN_C int       match(HalfEdge *h1,HalfEdge *h2);
EXTERN_C Solid   * Gtorus(Id sn,double r1,double r2,int nf1,int nf2);
EXTERN_C int       unsweep(Face *f);
EXTERN_C int       fillet(HalfEdge *he,double radius);
EXTERN_C int       shrink(Face *f);

/* Geometry */
EXTERN_C int       faceeq(Loop *l,vector eq);
EXTERN_C int       contvv(Vertex *v1,Vertex *v2);
EXTERN_C int       comp(double a,double b,double tol);
EXTERN_C int       intrev(Vertex *v1,Vertex *v2,Vertex *v3,double *t);
EXTERN_C int       contev(Vertex *v1,Vertex *v2,Vertex *v3);
EXTERN_C int       bndrlv(Loop *l,Vertex *v);
EXTERN_C int       contlv(Loop *l,Vertex *v,int drop);
EXTERN_C int       int2ee(Vertex *v1,Vertex *v2,Vertex *v3,Vertex *v4,int drop,double *t1,double *t2);
EXTERN_C int       dropcoord(vector v);
EXTERN_C int       onface(vector feq,vector v);
EXTERN_C int       contfv(Face *f,Vertex *v);
EXTERN_C int       contfp(Face *f,double x,double y,double z);
EXTERN_C int       checkwideness(HalfEdge *he);
EXTERN_C double    svolume(Solid *s);
EXTERN_C double    larea(Loop *l);

/* Transform */
EXTERN_C void stranslate(Solid *s,double dx,double dy,double dz);
EXTERN_C void srotate(Solid *s,double rx,double ry,double rz);
EXTERN_C void sscale(Solid *s,double sx,double sy,double sz);
EXTERN_C void stransform(Solid *s,matrix m);

/* Undo */
EXTERN_C void addeulerop(char opcode,Id solidno,Id ip1,Id ip2,Id ip3,Id ip4,Id ip5,Id ip6,double fp1,double fp2,double fp3,double fp4);
EXTERN_C void BeginTransaction(void);
EXTERN_C void EndTransaction(void);

/* Display */
extern int gGwbSolidMode;
EXTERN_C void GwbOutLine(int W, int H, int top, void* renderer);

#endif /* _GWB_H */
