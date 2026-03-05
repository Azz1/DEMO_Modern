/* adv.c */

#include "gwb.h"

/* advanced methods */
void  getmaxnames( Id sn )
	{
	Solid 	* s;
	Vertex  * v;
	Face 	* f;

	s = getsolid( sn );

	for( v = s -> sverts, maxv = 0; v != NULL; v = v -> nextv )
		if( v -> vertexno > maxv ) maxv = v -> vertexno;

	for( f = s -> sfaces, maxf = 0; f != NULL; f = f -> nextf )
		if( f -> faceno > maxf ) maxf = f -> faceno;

	}

void GArc( Id s, Id f, Id v, double cx, double cy, double rad, double h, double phi1, double phi2, int n )
	{
	double 	x,y,angle,inc;
	Id      prev;
	int	i;

	angle = phi1 * PI / 180.0;
	inc   = ( phi2 - phi1 ) * PI / ( 180.0 * n );
	prev  = v;
	getmaxnames( s );

	for( i = 0; i < n; i ++ )
		{
		angle  += inc;
		x 	= cx + cos( angle ) * rad;
		y	= cy + sin( angle ) * rad;
		smev( s, f, prev, ++ maxv, x, y, h );
		prev 	= maxv;
		}

	}

Solid * GCircle( Id sn, double cx, double cy, double rad, double h, int n )
	{
	Solid 	* s;

	s   = mvfs( sn, 1, 1, cx+rad, cy, h );
	GArc( sn, 1, 1, cx, cy, rad, h, 0.0, (( n-1 ) * 360.0 / n), n-1 );
	smef( sn, 1, n, 1, 2 );
	return s;

	}

void sweep( Face * fac, double dx, double dy, double dz )
	{
	Loop 		* l;
	HalfEdge        * first, * scan;
	Vertex 		* v;

	getmaxnames( fac -> fsolid -> solidno );
	l = fac -> floops;
	while( l != NULL )
		{
		first = l -> ledg;
		scan  = first -> nxt;
		v     = scan -> vex;

		lmev( scan, scan, ++ maxv,
		      v -> vcoord[0] + dx,
		      v -> vcoord[1] + dy,
		      v -> vcoord[2] + dz );

		while( scan != first )
			{
			v = scan -> nxt -> vex;
			lmev( scan -> nxt, scan -> nxt, ++ maxv,
				      v -> vcoord[0] + dx,
				      v -> vcoord[1] + dy,
				      v -> vcoord[2] + dz );
			lmef( scan -> prv, scan -> nxt -> nxt, ++ maxf );
			scan = mate( scan -> nxt ) -> nxt;
			}

		lmef( scan -> prv, scan -> nxt -> nxt, ++ maxf );
		l = l -> nextl;
		}
	}

Solid  * Gblock( Id sn, double dx, double dy, double dz )
	{
	Solid 	   * s;

	s = mvfs( sn, 1, 1, 0.0, 0.0, 0.0 );
	smev( sn, 1, 1, 2, dx, 0.0, 0.0 );
	smev( sn, 1, 2, 3, dx, dy, 0.0 );
	smev( sn, 1, 3, 4, 0.0, dy, 0.0 );
	smef( sn, 1, 1, 4, 2 );
	sweep( fface( s, 2 ), 0.0, 0.0, dz );

	return s;
	}

Solid  * Gcyl( Id sn, double rad, double h, double n )
	{
	Solid 	 * s;

	s = GCircle( sn, 0.0, 0.0, rad, 0.0, n );
	sweep( fface( s, 2 ), 0.0, 0.0, h );

	return s;
	}

int    shrink( Face * f )
	{
	Loop 		* l;
	HalfEdge        * he, * nhe;

	l = f -> floops;
	if( l -> nextl != NULL )
		return 0;		/* f has more than 1 loops */

	he = l -> ledg;
	nhe = he -> nxt;
	while( nhe != he )
		{
		if( ! vecequal( he -> vex -> vcoord, nhe -> vex -> vcoord ) )
			return 0;
		nhe = nhe -> nxt;
		}			/* test if all points are the same */

	while( he -> nxt != he )
		{
		lkev( he, mate( he ) );
		he = l -> ledg;
		}

	lkef( he, mate( he ) );
	return 1;

	}

void   rsweep( Solid * s, int nfaces )
	{
	HalfEdge        * first, * cfirst, * last, * scan;
	Face 		* tailf;
	matrix		m;
	vector 		v;
	HalfEdge        * h;
	Face		* headf;
	int 		closed_figure;

	if( s -> sfaces -> nextf != NULL )	/* does the solid have > 1 faces ? */
		{
		/* assume it's a lamina */
		closed_figure = 1;

		h = s -> sfaces -> floops -> ledg;

		lmev( h, mate( h ) -> nxt, ++ maxv,
			h -> vex -> vcoord[0],
			h -> vex -> vcoord[1],
			h -> vex -> vcoord[2] );
		lkef( h -> prv, mate( h -> prv ) );

		}
	else
		closed_figure = 0;		/* it's a wire */

	headf = s -> sfaces;

	getmaxnames( s -> solidno );
	first = s -> sfaces -> floops -> ledg;

	while( first -> edg != first -> nxt -> edg ) first = first -> nxt;
	last  = first -> nxt;

	while( last -> edg != last -> nxt -> edg ) last = last -> nxt;
	cfirst  = first;
	matident( m );
	matrotat( m, ( 360.0 / nfaces ), 0.0, 0.0 );

	while( -- nfaces )
		{
		vecmult( v, cfirst -> nxt -> vex -> vcoord, m );
		lmev( cfirst -> nxt, cfirst -> nxt, ++ maxv, v[0], v[1], v[2] );
		scan  = cfirst -> nxt;

		while( scan != last -> nxt )
			{
			vecmult( v, scan -> prv -> vex -> vcoord, m );
			lmev( scan -> prv, scan -> prv, ++ maxv, v[0], v[1], v[2] );
			lmef( scan -> prv -> prv, scan -> nxt, ++ maxf );

			scan = mate( scan -> nxt -> nxt );
			}

		last   = scan;
		cfirst = mate( cfirst -> nxt -> nxt );
		}

	tailf = lmef( cfirst -> nxt, mate( first ), ++ maxf );
	while( cfirst != scan )
		{
		lmef( cfirst, cfirst -> nxt -> nxt -> nxt, ++ maxf );
		cfirst = mate( cfirst -> prv ) -> prv;
		}

	shrink( headf );
	shrink( tailf );

	if( closed_figure == 1 )
		{
		lkfmrh( headf, tailf );
		loopglue( headf );
		}

	}

Solid  * GBall( Id sn, double r, int nver, int nhor )
	{
	Solid	   * s;

	s = mvfs( sn, 1, 1, -r, 0.0, 0.0 );
	GArc( s -> solidno , 1, 1, 0.0, 0.0, r, 0.0, 180.0, 0.0, nver );
	rsweep( s, nhor );
	return s;

	}


void   glue( Solid * s1, Solid * s2, Face * f1, Face * f2 )
	{
	merge( s1, s2 );
	lkfmrh( f1, f2 );
	loopglue( f1 );
	}

void   merge( Solid * s1, Solid * s2 )
	{
	while( s2 -> sfaces != NULL )
		{
		dellist( FACE, (Node *)s2 -> sfaces, (Node *)s2 );
		addlist( FACE, (Node *)s2 -> sfaces, (Node *)s1 );
		}

	while( s2 -> sedges != NULL )
		{
		dellist( EDGE, (Node *)s2 -> sedges, (Node *)s2 );
		addlist( EDGE, (Node *)s2 -> sedges, (Node *)s1 );
		}

	while( s2 -> sverts != NULL )
		{
		dellist( VERTEX, (Node *)s2 -> sverts, (Node *)s2 );
		addlist( VERTEX, (Node *)s2 -> sverts, (Node *)s1 );
		}
	Del( SOLID, (Node *)s2, NULL );
	}

void   loopglue( Face * fac )
	{
	HalfEdge  *h1, *h2, *h1next;

	h1 = fac -> floops -> ledg;
	h2 = fac -> floops -> nextl -> ledg;

	while( ! match( h1, h2 ) )
		h2 = h2 -> nxt;

	lmekr( h1, h2 );
	lkev( h1 -> prv, h2 -> prv );

	while( h1 -> nxt != h2 )
		{
		h1next = h1 -> nxt;

		lmef( h1 -> prv, h1 -> nxt, -1 );
		lkev( h1 -> nxt, mate( h1 -> nxt ) );
		lkef( h1, mate( h1 ) );

		h1 = h1next;
		}

	lkef( mate( h1 ), h1 );
	}

int    match( HalfEdge * h1, HalfEdge * h2 )
	{
	return vecequal( h1->vex->vcoord, h2->vex->vcoord )
	    && vecequal( h1->prv->vex->vcoord, h2->nxt->vex->vcoord );

	}

Solid  * Gtorus( Id sn, double r1, double r2, int nf1, int nf2 )
	{
	Solid * s;

	s = GCircle( sn, 0.0, r1, r2, 0.0, nf2 );
	rsweep( s, nf1 );
	return s;
	}

int  unsweep( Face * f )
	{
	return 0;
	}

int  fillet( HalfEdge * he, double radius )
	{
	return 0;
	}

/* End of file */

