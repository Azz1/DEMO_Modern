#include <stdio.h>
/* euler.c */


#include "gwb.h"

char msg[100];

/* basic euler methods */

Solid * mvfs(Id s,Id f, Id v, double x, double y, double z)
	{
	Solid 	*newsolid;
	Face 	*newface;
	Vertex	*newvertex;
	HalfEdge *newhe;
	Loop	*newloop;

	newsolid = (Solid *)New(SOLID,NULL);
	newface  = (Face  *)New(FACE, (Node *)newsolid);
	newvertex= (Vertex *)New(VERTEX,(Node *)newsolid);
	newhe   = (HalfEdge *)New(HALFEDGE,NULL);
	newloop  = (Loop   *)New(LOOP,(Node *)newface);

	newsolid -> solidno = s;
	newface  -> faceno  = f;
	newface  -> flout   = newloop;
	newloop  -> ledg   = newhe;
	newhe    -> wloop   = newloop;
	newhe    -> nxt     = newhe -> prv = newhe;
	newhe    -> vex     = newvertex;
	newhe    -> edg     = NULL;
	newvertex -> vertexno = v;
	newvertex -> vcoord[0] = x;
	newvertex -> vcoord[1] = y;
	newvertex -> vcoord[2] = z;
	newvertex -> vcoord[3] = 1.0;

	return newsolid;
	}


void lmev(HalfEdge *he1, HalfEdge *he2, Id vn,double x,double y,double z)
	{
	HalfEdge *he;
	Edge     *newedge;
	Vertex   *newvertex;

	if( Generatelog )
		addeulerop( KEV,
			he1 -> wloop -> lface -> fsolid -> solidno,
			he1 -> wloop -> lface -> faceno,
			he1 -> vex -> vertexno,
			vn,
			0, 0, 0, 0.0, 0.0, 0.0, 0.0 );

	newedge   = (Edge *)New(EDGE,(Node *)he1 -> wloop -> lface -> fsolid);
	newvertex = (Vertex *)New(VERTEX,(Node *)he1 -> wloop -> lface -> fsolid);

	newvertex -> vcoord[0] = x;
	newvertex -> vcoord[1] = y;
	newvertex -> vcoord[2] = z;
	newvertex -> vcoord[3] = 1.0;
	newvertex -> vertexno = vn;

	he = he1;
	while(he != he2)
		{
		he -> vex = newvertex;
		he = mate(he) -> nxt;
		}
	addhe(newedge,he2 -> vex,he1,MINUS);
	addhe(newedge,newvertex,he2,PLUS);

	newvertex -> vedge = he2 -> prv;
	he2 -> vex -> vedge = he2;

	}

Face * lmef(HalfEdge *he1, HalfEdge *he2, Id f)
	{
	HalfEdge *he, *nhe1, *nhe2, *temp;
	Face *newface;
	Loop *newloop;
	Edge *newedge;

	newface = (Face *)New(FACE,(Node *)he2 -> wloop -> lface -> fsolid);
	newloop = (Loop *)New(LOOP,(Node *)newface);
	newedge = (Edge *)New(EDGE,(Node *)he2 -> wloop -> lface -> fsolid);

	newface -> faceno = f;
	newface -> flout = newloop;

	he = he1;
	while(he != he2)
		{
		he -> wloop = newloop;
		he = he -> nxt;
		}
	nhe1 = addhe(newedge,he2 -> vex,he1,MINUS);
	nhe2 = addhe(newedge,he1 -> vex,he2,PLUS);
	nhe1 -> wloop = newloop;

	nhe1 -> prv -> nxt = nhe2;
	nhe2 -> prv -> nxt = nhe1;

	temp = nhe1 -> prv;
	nhe1 -> prv = nhe2 -> prv;
	nhe2 -> prv = temp;

	newloop -> ledg = nhe1;
	he2 -> wloop -> ledg = nhe2;

	return newface;

	}

void lkemr(HalfEdge *h1 , HalfEdge *h2)
	{
	register HalfEdge *h3, *h4;
	Loop     *nl;
	Loop     *ol;
	Edge     *killedge;

	ol = h1 -> wloop;
	nl = (Loop *)New(LOOP,(Node *)ol -> lface);
	killedge = h1 -> edg;

	h3 = h1 -> nxt;
	h1 -> nxt = h2 -> nxt;
	h2 -> nxt -> prv = h1;
	h2 -> nxt = h3;
	h3 -> prv = h2;

	h4 = h2;
	do
		{
		h4 -> wloop = nl;
		}while((h4 = h4 -> nxt) != h2);

	ol -> ledg = h3 = delhe(h1);
	nl -> ledg = h4 = delhe(h2);

	h3 -> vex -> vedge = (h3 -> edg != NULL)? h3:(HalfEdge *)NULL;
	h4 -> vex -> vedge = (h4 -> edg != NULL)? h4:(HalfEdge *)NULL;

	Del(EDGE, (Node *)killedge, (Node *)ol -> lface -> fsolid);

	}

void lkvfs(Solid *s)
	{
	Del( VERTEX, (Node *)s -> sverts, (Node *)s );
	delhe( s -> sfaces -> floops -> ledg );
	Del( LOOP, (Node *)s -> sfaces -> floops, (Node *)s -> sfaces );
	Del( FACE, (Node *)s -> sfaces, (Node *)s );
	Del( SOLID, (Node *)s, NULL );
	}

void lkev(HalfEdge *he1, HalfEdge *he2)
	{
	HalfEdge *h;

	h = mate( he1 ) -> nxt;
	while( h != he1 )
		{
		h -> vex = he2 -> vex;
		h = mate( h ) -> nxt;
		}

	Del( EDGE, (Node *)he1 -> edg, (Node *)he1 -> wloop -> lface -> fsolid );
	if( he1 -> vex != he2 -> vex )
		Del( VERTEX, (Node *)he1 -> vex, (Node *)he1 -> wloop -> lface -> fsolid );

	delhe( he1 );
	delhe( he2 );

	}

void lkef(HalfEdge *he1, HalfEdge *he2)		/* he1 -> edg == he2 -> edg && he1 -> wloop -> lface != he2 -> wloop -> lface */
	{
	Face		*f, *f1, *f2;
	HalfEdge        *temp;
	Loop		*l, *l1, *l2;

	l1 = he1 -> wloop;
	f1 = l1 -> lface;
	l2 = he2 -> wloop;
	f2 = l2 -> lface;

	if( Generatelog )
		{
		for( l = f1 -> floops; l != NULL; l = l -> nextl )
			{
			if( l != l2 )
				addeulerop( RINGMV,
					f1 -> fsolid -> solidno,
					l -> ledg -> vex -> vertexno,
					l -> ledg -> nxt -> vex -> vertexno,
					f1 -> faceno, f2 -> faceno,
					( l == f1 -> flout )? 1:0,
					0, 0.0, 0.0, 0.0, 0.0 );

			}
		addeulerop( MEF, f1 -> fsolid -> solidno, f1 -> faceno,
			he2 -> vex -> vertexno, he2 -> nxt -> vex -> vertexno,
			he1 -> vex -> vertexno, he1 -> nxt -> vex -> vertexno,
			f2 -> faceno, 0.0, 0.0, 0.0, 0.0 );

		}


	Del( EDGE, (Node *)he1 -> edg, (Node *)he1 -> wloop -> lface -> fsolid );
	f = he1 -> wloop -> lface;
	Del( LOOP, (Node *)he1 -> wloop, (Node *)f );
	Del( FACE, (Node *)f, (Node *)f -> fsolid );

	temp = he1 -> prv;
	he1 -> prv = he2 -> prv;
	he2 -> prv -> nxt = he1;

	he2 -> prv = temp;
	temp -> nxt = he2;				/* join to rings */

	temp = he1;
	l = he2 -> wloop;
	while( temp != he2 )
		{
		temp -> wloop = l;
		temp = temp -> nxt;
		}

	delhe( he1 );
	l -> ledg = delhe( he2 );
	l -> ledg -> vex -> vedge = ( l -> ledg -> edg != NULL )? l -> ledg : (HalfEdge *) NULL;

	}

void lmekr(HalfEdge *he1, HalfEdge *he2)	/* he1 -> wloop != he2 -> wloop && he1 -> wloop -> lface == he2 -> wloop -> lface */
	{
	Edge 		* newedge;
	HalfEdge    *temp1, *temp2;

	Del( LOOP, (Node *)he2 -> wloop, (Node *)he2 -> wloop -> lface );

	newedge = ( Edge * ) New ( EDGE, (Node *)he1 -> wloop -> lface -> fsolid );

	temp1 = he1 -> prv;
	temp2 = he2 -> prv;

	he1 -> prv -> nxt = he2;
	he2 -> prv = temp1;
	he1 -> prv = temp2;
	temp2 -> nxt = he1;

	temp1 = he2;
	while( temp1 != he1 )
		{
		temp1 -> wloop = he1 -> wloop;
		temp1 = temp1 -> nxt;
		}

	addhe( newedge, he2 -> vex, he1, MINUS );
	addhe( newedge, he1 -> vex, he2, PLUS );

	}

void  lkfmrh(Face *fac1, Face *fac2)
	{
	addlist( LOOP, (Node *)fac2 -> floops, (Node *)fac1 );
	Del( FACE, (Node *)fac2, (Node *)fac2 -> fsolid );
	}

Face * lmfkrh(Loop *l, Id f)			/* l is a inner ring */
	{
	Face 		* newface;

	newface = ( Face * ) New ( FACE, (Node *)l -> lface -> fsolid );
	newface -> faceno = f;

	dellist( LOOP, (Node *)l, (Node *)l -> lface );
	addlist( LOOP, (Node *)l, (Node *)newface );

	newface -> flout = l;

	return newface;
	}

void lringmv(Loop *l,Face *tofac, int inout)	/* inout: 0 -- inner ring;  1 -- outer ring */
	{
	if( l -> lface != tofac )
		{
		dellist( LOOP, (Node *)l, (Node *)l -> lface );
		addlist( LOOP, (Node *)l, (Node *)tofac );
		}

	if( inout )
		tofac -> flout = l;

	}

void laringmv( Face * f1, Face * f2 )
	{
	vector 	v, rv, vo;
	Loop	*l, *lo, *ln;

	l = ln = f1 -> floops;
	lo = f1 -> flout;

	faceeq( lo, vo );
	normalize( vo );

	while( l != NULL )
		{
		if( l != lo )
			{
			faceeq( l, v );
			normalize( v );

			rv[0] = - v[0];
			rv[1] = - v[1];
			rv[2] = - v[2];

			if( vecequal( v, vo ) || vecequal( rv, vo ) )
				{
				if( onface( vo, l -> ledg -> vex -> vcoord ) == 0 )
					{
					l = l -> nextl;
					continue;
					}
				}

			ln = l -> nextl;
			lringmv( l, f2, 0 );
			l = ln;
			}
		}
	}

Solid * getsolid(Id sn)
	{
	Solid *s;

	for( s = firsts; s != NULL; s = s -> nexts )
		if( s -> solidno == sn )
			return s;
	return NULL;
	}

Face * fface(Solid *s, Id fn)
	{
	Face *f;
	for(f = s -> sfaces; f != NULL; f = f -> nextf)
		if(f -> faceno == fn)
			return f;
	return NULL;
	}

HalfEdge * fhe(Face *f, Id vn1, Id vn2)
	{
	Loop	 *l;
	HalfEdge *h;
	for(l = f -> floops; l != NULL ; l = l -> nextl)
		{
		h = l -> ledg;
		do
			{
			if(h -> vex -> vertexno == vn1 && ( ( vn2 == -1 ) || ( h -> nxt -> vex -> vertexno == vn2 )))
				return h;
			}while((h = h -> nxt) != l -> ledg);
		}
	return NULL;
	}


/* advanced euler methods */

int kvfs(Id s)
	{
	Solid 		* oldsolid;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "kvfs:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}

	lkvfs( oldsolid );

	return 1;
	}

int mev(Id s, Id f1, Id f2, Id v1, Id v2, Id v3, Id v4, double x, double y, double z)
	{
	Solid   *oldsolid;
	Face    *oldface1, *oldface2;
	HalfEdge *he1, *he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "mev:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface1 = fface(oldsolid,f1)) == NULL)
		{
		sprintf(msg, "mev:face %d not found in solid %d",f1,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface2 = fface(oldsolid,f2)) == NULL)
		{
		sprintf(msg, "mev:face %d not found in solid %d",f2,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface1,v1,v2)) == NULL)
		{
		sprintf(msg, "mev:edge %d - %d not found in face %d",v1,v2,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface2,v1,v3)) == NULL)
		{
		sprintf(msg, "mev:edge %d - %d not found in face %d",v1,v3,f2);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lmev(he1,he2,v4,x,y,z);
	return 1;
	}

int  smev(Id s, Id f1, Id v1, Id v4, double x, double y, double z)
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "smev:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f1)) == NULL)
		{
		sprintf(msg, "smev:face %d not found in solid %d",f1,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he = fhe(oldface,v1,-1)) == NULL)
		{
		sprintf(msg, "smev:edge %d -  not found in face %d",v1,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lmev(he,he,v4,x,y,z);
	return 1;

	}

int  kev(Id s, Id f, Id v1, Id v2)
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "kev:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f)) == NULL)
		{
		sprintf(msg, "kev:face %d not found in solid %d",f,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface,v1,v2)) == NULL)
		{
		sprintf(msg, "kev:edge %d - %d not found in face %d",v1,v2,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface,v2,v1)) == NULL)
		{
		sprintf(msg, "kev:edge %d - %d not found in face %d",v2,v1,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lkev(he1,he2);
	return 1;

	}

int  mef(Id s, Id f1, Id v1, Id v2, Id v3, Id v4, Id f2)
	{
	Solid 		* oldsolid;
	Face		* oldface1;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "mef:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface1 = fface(oldsolid,f1)) == NULL)
		{
		sprintf(msg, "mef:face %d not found in solid %d",f1,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface1,v1,v2)) == NULL)
		{
		sprintf(msg, "mef:edge %d - %d not found in face %d",v1,v2,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface1,v3,v4)) == NULL)
		{
		sprintf(msg, "mef:edge %d - %d not found in face %d",v3,v4,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lmef(he1,he2,f2);
	return 1;

	}

int  smef(Id s, Id f1, Id v1, Id v3, Id f2)
	{
	Solid 		* oldsolid;
	Face		* oldface1;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "smef:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface1 = fface(oldsolid,f1)) == NULL)
		{
		sprintf(msg, "smef:face %d not found in solid %d",f1,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface1,v1,-1)) == NULL)
		{
		sprintf(msg, "smef:edge %d -  not found in face %d",v1,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface1,v3,-1)) == NULL)
		{
		sprintf(msg, "smef:edge %d -  not found in face %d",v3,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}

	lmef(he1,he2,f2);
	return 1;

	}

int  kef(Id s, Id f, Id v1, Id v2)
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "kef:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f)) == NULL)
		{
		sprintf(msg, "kef:face %d not found in solid %d",f,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface,v1,v2)) == NULL)
		{
		sprintf(msg, "kef:edge %d - %d not found in face %d",v1,v2,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface,v2,v1)) == NULL)
		{
		sprintf(msg, "kef:edge %d - %d not found in face %d",v2,v1,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lkef(he1,he2);
	return 1;

	}

int  mekr(Id s,Id f, Id v1, Id v2, Id v3, Id v4)
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "mekr:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f)) == NULL)
		{
		sprintf(msg, "mekr:face %d not found in solid %d",f,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface,v1,v2)) == NULL)
		{
		sprintf(msg, "mekr:edge %d - %d not found in face %d",v1,v2,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface,v3,v4)) == NULL)
		{
		sprintf(msg, "mekr:edge %d - %d not found in face %d",v3,v4,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lmekr(he1,he2);
	return 1;

	}

int  smekr( Id s, Id f, Id v1, Id v3 )
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "smekr:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f)) == NULL)
		{
		sprintf(msg, "smekr:face %d not found in solid %d",f,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface,v1,-1)) == NULL)
		{
		sprintf(msg, "smekr:edge %d -  not found in face %d",v1,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface,v3,-1)) == NULL)
		{
		sprintf(msg, "mekr:edge %d -  not found in face %d",v3,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lmekr(he1,he2);
	return 1;

	}

int  kemr(Id s,Id f,Id v1,Id v2)
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he1, * he2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "kemr:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f)) == NULL)
		{
		sprintf(msg, "kemr:face %d not found in solid %d",f,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he1 = fhe(oldface,v1,v2)) == NULL)
		{
		sprintf(msg, "kemr:edge %d - %d not found in face %d",v1,v2,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he2 = fhe(oldface,v2,v1)) == NULL)
		{
		sprintf(msg, "kemr:edge %d - %d not found in face %d",v2,v1,f);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lkemr(he1,he2);
	return 1;

	}

int  kfmrh(Id s,Id f1,Id f2)
	{
	Solid   *oldsolid;
	Face    *oldface1, *oldface2;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "kfmrh:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface1 = fface(oldsolid,f1)) == NULL)
		{
		sprintf(msg, "kfmrh:face %d not found in solid %d",f1,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface2 = fface(oldsolid,f2)) == NULL)
		{
		sprintf(msg, "kfmrh:face %d not found in solid %d",f2,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lkfmrh(oldface1,oldface2);
	return 1;
	}

int  mfkrh(Id s,Id f1,Id v1,Id v2, Id f2)
	{
	Solid 		* oldsolid;
	Face		* oldface;
	HalfEdge	* he;

	if((oldsolid = getsolid(s)) == NULL)
		{
		sprintf(msg, "mfkrh:solid %d not found",s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface = fface(oldsolid,f1)) == NULL)
		{
		sprintf(msg, "mfkrh:face %d not found in solid %d",f1,s);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he = fhe(oldface,v1,v2)) == NULL)
		{
		sprintf(msg, "mfkrh:edge %d - %d not found in face %d",v1,v2,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lmfkrh( he -> wloop, f2 );
	return 1;

	}

int  ringmv(Solid *s, Id f1,Id f2,Id v1,Id v2,int inout)
	{
	Face		* oldface1, * oldface2;
	HalfEdge	* he;

	if((oldface1 = fface(s,f1)) == NULL)
		{
		sprintf(msg, "ringmv:face %d not found in solid",f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((oldface2 = fface(s,f2)) == NULL)
		{
		sprintf(msg, "ringmv:face %d not found in solid",f2);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	if((he = fhe(oldface1,v1,v2)) == NULL)
		{
		sprintf(msg, "ringmv:edge %d - %d not found in face %d",v1,v2,f1);
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	lringmv( he -> wloop, oldface2, inout );
	return 1;

	}


/* End of file */


