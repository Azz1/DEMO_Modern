#include <stdio.h>
/* geo.c -- geometry algoriums */

#include "gwb.h"

int  faceeq( Loop * l, vector eq )
	{
	HalfEdge	*he;
	double 		a,b,c,norm;
	double 		xi,yi,zi,xj,yj,zj,xc,yc,zc;
	int 		len;

	a = b = c = xc = yc = zc = 0.0;
	len = 0;
	he  = l -> ledg;

	do
		{
		xi = he -> vex -> vcoord[0];
		yi = he -> vex -> vcoord[1];
		zi = he -> vex -> vcoord[2];

		xj = he -> nxt -> vex -> vcoord[0];
		yj = he -> nxt -> vex -> vcoord[1];
		zj = he -> nxt -> vex -> vcoord[2];

		a += ( yi - yj )*( zi + zj );
		b += ( zi - zj )*( xi + xj );
		c += ( xi - xj )*( yi + yj );

		xc += xi;
		yc += yi;
		zc += zi;

		len ++;

		}while( ( he = he -> nxt ) != l -> ledg );
	if( ( norm = sqrt( a*a + b*b + c*c ) ) > EPS )
		{
		eq[0] = a/norm;
		eq[1] = b/norm;
		eq[2] = c/norm;
		eq[3] = ( eq[0]*xc + eq[1]*yc + eq[2]*zc )/( - len );
		return 1;
		}
	else
		{
		sprintf( msg, "faceeq: null face %d", l -> lface -> faceno );
		fprintf(stderr, "%s\n", msg);
		return 0;
		}
	}

int  contvv( Vertex * v1, Vertex * v2 )
	{
	double dx,dy,dz;
	double diff;

	dx = v1 -> vcoord[0] - v2 -> vcoord[0];
	dy = v1 -> vcoord[1] - v2 -> vcoord[1];
	dz = v1 -> vcoord[2] - v2 -> vcoord[2];

	diff = dx*dx + dy*dy + dz*dz;

	return comp( diff, 0.0, EPS*EPS ) == 0;
	}

int  comp( double a, double b, double tol )
	{
	double delta = fabs( a - b );
	if( delta < tol )	return 0;
	else if( a > b )	return 1;
	else			return -1;
	}

int  intrev( Vertex * v1, Vertex * v2, Vertex * v3, double * t )
	{
	Vertex testv;
	double r1[3],r2[3],r1r1,tprime;

	r1[0] = v2 -> vcoord[0] - v1 -> vcoord[0];
	r1[1] = v2 -> vcoord[1] - v1 -> vcoord[1];
	r1[2] = v2 -> vcoord[2] - v1 -> vcoord[2];

	r1r1 = dot( r1, r1 );
	if( r1r1 < EPS*EPS )
		{
		*t = 0.0;
		return contvv( v1, v3 );
		}
	else
		{
		r2[0] = v3 -> vcoord[0] - v1 -> vcoord[0];
		r2[1] = v3 -> vcoord[1] - v1 -> vcoord[1];
		r2[2] = v3 -> vcoord[2] - v1 -> vcoord[2];

		tprime = dot( r1, r2 )/r1r1;

		testv.vcoord[0] = v1 -> vcoord[0] + tprime*r1[0];
		testv.vcoord[1] = v1 -> vcoord[1] + tprime*r1[1];
		testv.vcoord[2] = v1 -> vcoord[2] + tprime*r1[2];

		*t = tprime;

		return contvv( &testv, v3 );
		}
	}

int  contev( Vertex * v1, Vertex * v2, Vertex * v3 )
	{
	double t;

	if( intrev( v1, v2, v3, &t ) )
		if( t >= ( - EPS ) && t <= ( 1.0 + EPS ) )
			return 1;
	return 0;
	}


HalfEdge	*hithe;
Vertex		*hitvertex;

int bndrlv( Loop * l, Vertex * v )
	{
	HalfEdge * he;

	he = l -> ledg;
	do
		{
		if( contvv( he -> vex, v ) )
			{
			hitvertex = he -> vex;
			hithe = NULL;
			return 3;
			}

		}while( (he = he -> nxt) != l -> ledg );

	he = l -> ledg;
        do
		{
		if( contev( he -> vex, he -> nxt -> vex, v ) )
			{
			hitvertex = NULL;
			hithe =he;
			return 2;
			}

		}while( (he = he -> nxt) != l -> ledg );

	return 1;
	}


int  contlv( Loop * l, Vertex * v, int drop )
      {
	HalfEdge	*he1, *he2;
	Vertex		*v1, *v2, aux;
	double 		t1, t2;
	int 		count, intr, c1, c2;

	if( (intr = bndrlv( l, v )) > 0 ) return intr;
	he2 = l -> ledg;

      retry:

	v1 = he2 -> vex;
	v2 = he2 -> nxt -> vex;
	aux.vcoord[0] = ( v1 -> vcoord[0] + v2 -> vcoord[0] )/2.0;
	aux.vcoord[1] = ( v1 -> vcoord[1] + v2 -> vcoord[1] )/2.0;
	aux.vcoord[2] = ( v1 -> vcoord[2] + v2 -> vcoord[2] )/2.0;

	he1 = l -> ledg;
	count = 0;

	do
		{
		intr = int2ee( v, &aux, v1, v2, drop, &t1, &t2 );
		if( intr == 1 )
			{
			c1 = comp( t2, 0.0, EPS );
			c2 = comp( t2, 1.0, EPS );

			if( c1 == 0 || c2 == 0 )
				{
				he2 = he2 -> nxt;
				if( he2 == l -> ledg ) return ERROR;

				goto retry;

				}

			if( c1 == 1 && c2 == -1 )
				if( t1 >= 0.0 ) count ++;
			}

		}while( (he1 = he1 -> nxt) != l -> ledg );

	count %= 2;
	return count;
      }

int  int2ee( Vertex * v1, Vertex * v2, Vertex * v3, Vertex * v4, int drop, double * t1, double * t2 )
	{
	double d, a1, a2, b1, b2, c1, c2;

	switch( drop )
		{
		case X:
			a1 = v2 -> vcoord[1] - v1 -> vcoord[1];
			a2 = v2 -> vcoord[2] - v1 -> vcoord[2];
			b1 = v3 -> vcoord[1] - v4 -> vcoord[1];
			b2 = v3 -> vcoord[2] - v4 -> vcoord[2];
			c1 = v1 -> vcoord[1] - v3 -> vcoord[1];
			c2 = v1 -> vcoord[2] - v3 -> vcoord[2];
			break;

		case Y:
			a1 = v2 -> vcoord[0] - v1 -> vcoord[0];
			a2 = v2 -> vcoord[2] - v1 -> vcoord[2];
			b1 = v3 -> vcoord[0] - v4 -> vcoord[0];
			b2 = v3 -> vcoord[2] - v4 -> vcoord[2];
			c1 = v1 -> vcoord[0] - v3 -> vcoord[0];
			c2 = v1 -> vcoord[2] - v3 -> vcoord[2];
			break;

		case Z:
			a1 = v2 -> vcoord[0] - v1 -> vcoord[0];
			a2 = v2 -> vcoord[1] - v1 -> vcoord[1];
			b1 = v3 -> vcoord[0] - v4 -> vcoord[0];
			b2 = v3 -> vcoord[1] - v4 -> vcoord[1];
			c1 = v1 -> vcoord[0] - v3 -> vcoord[0];
			c2 = v1 -> vcoord[1] - v3 -> vcoord[1];
			break;

		}
	if( comp( (d = a1*b2 - a2*b1), 0.0, EPS ) == 0 )
		return 0;

	*t1 = ( c2*b1 - c1*b2 )/d;
	*t2 = ( a2*c1 - a1*c2 )/d;

	return 1;
	}

double svolume( Solid * s )
	{
	Face 	 * f;
	Loop 	 * l;
	HalfEdge * he1, * he2;
	vector	   c;
	double	   res;

	res = 0.0;
	f = s -> sfaces;

	while( f != NULL )
		{
		l = f -> floops;

		while( l != NULL )
			{
			he1 = l -> ledg;
			he2 = he1 -> nxt;

			do
				{

				cross( c, he1 -> vex -> vcoord, he2 -> vex -> vcoord );
				res += dot( he2 -> nxt -> vex -> vcoord, c );

				}while( (he2 = he2 -> nxt) != he1 );

			l = l -> nextl;
			}

		f = f -> nextf;
		}

	return res/6.0;
	}

double larea( Loop * l )
	{
	HalfEdge 	* he;
	Vertex		* v1, * v2, * v3;
	vector		  aa, bb, cc, dd, norm;

	dd[0] = dd[1] = dd[2] = 0.0;
	faceeq( l, norm );
	he = l -> ledg;
	v1 = he -> vex;
	he = he -> nxt;

	do
		{

		v2 = he -> vex;
		v3 = he -> nxt -> vex;

		aa[0] = v2 -> vcoord[0] - v1 -> vcoord[0];
		aa[1] = v2 -> vcoord[1] - v1 -> vcoord[1];
		aa[2] = v2 -> vcoord[2] - v1 -> vcoord[2];
		bb[0] = v3 -> vcoord[0] - v1 -> vcoord[0];
		bb[1] = v3 -> vcoord[1] - v1 -> vcoord[1];
		bb[2] = v3 -> vcoord[2] - v1 -> vcoord[2];

		cross( cc, aa, bb );

		dd[0] += cc[0];
		dd[1] += cc[1];
		dd[2] += cc[2];

		}while( (he = he -> nxt) != l -> ledg );

	return 0.5*dot( norm, dd );

	}

int  dropcoord( vector v )
	{
	if( fabs( v[0] ) > fabs( v[1] ) )
		{
		if( fabs( v[0] ) > fabs( v[2] ) )
			return X;
		return Z;
		}
	if( fabs( v[1] ) > fabs( v[2] ) )
		return Y;
	return Z;
	}

int  onface( vector feq, vector v )
	{
	double res;

	res = dot( feq, v ) + feq[3];

	if( res > EPS )
		return 1;

	else if( res < -EPS )
		return -1;

	return 0;
	}

int  contfv( Face * f, Vertex * v )
	{
	Loop 	* l;
	vector 	  eq;

	faceeq( f -> flout, f -> feq );
	if( onface( f -> feq, v -> vcoord ) != 0 )
		return ERROR;				/* v is not on face */

	l = f -> floops;
	while( l != NULL )
		{
		if( l == f -> flout )
			{
			if( contlv( l, v, dropcoord( f -> feq ) ) == 0 )
				return 0;
			}
		else
			{
			faceeq( l, eq );
			if( contlv( l, v, dropcoord( eq ) ) == 1 )
				return 0;
			}

		l = l -> nextl;
		}
	return 1;
	}

int  contfp( Face * f, double x, double y, double z )
	{
	Vertex * v;
	int  	 ret;

	v = ( Vertex * )New( VERTEX, (Node *)f -> fsolid );
	v -> vcoord[0] = x;
	v -> vcoord[1] = y;
	v -> vcoord[2] = z;
        v -> vcoord[3] = 1.0;
	ret = contfv( f, v );

	Del( VERTEX, (Node *)v, (Node *)f -> fsolid );

	return ret;
	}

int checkwideness( HalfEdge * he )
	{
	double res;
	vector v1, v2, eq;

	vecminus( v1, he -> nxt -> vex -> vcoord, he -> vex -> vcoord );
	vecminus( v2, he -> prv -> vex -> vcoord, he -> vex -> vcoord );

	cross( v1, v1, eq );
	v1[3] = v2[3] = 0.0;

	res = dot( v1, v2 );
	if( res >= 0.0 )
		return 1;
	return 0;
	}


/* End of file */
