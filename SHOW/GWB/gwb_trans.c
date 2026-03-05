/* trans.c -- transform of solids */

#include "gwb.h"

/* transform methods */

void stranslate( Solid * s, double dx, double dy, double dz )
	{
	matrix m;

	matident( m );
	mattrans( m, dx, dy, dz );
	stransform( s, m );
	}

void srotate( Solid * s, double rx, double ry, double rz )
	{
	matrix m;

        matident( m );
	matrotat( m, rx, ry, rz );
	stransform( s, m );
	}

void sscale( Solid * s, double sx, double sy, double sz )
	{
	matrix m;

        matident( m );
	matscale( m, sx, sy, sz );
	stransform( s, m );
	}

void stransform( Solid * s, matrix m )
	{
	Vertex * v;

	v = s -> sverts;

	while( v != NULL )
		{
		vecmult( v -> vcoord, v -> vcoord, m );
		v = v -> nextv;
		}
	}

/* End of file */
