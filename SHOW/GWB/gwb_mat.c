/* mat.c */

#include "gwb_mat.h"

double 	EPS    = FPS;		/* tolerance for geometric tests */
double 	BIGEPS = FPS;		/* a more permissive tolerance */

void matcopy( matrix m1, matrix m2 )		/* m1 <== m2 */
	{
	int i, j;

	for( i = 0; i < 4; i ++ )
		for( j = 0; j < 4; j ++ )
			m1[i][j] = m2[i][j];
	}

void veccopy( vector v1, vector v2 )		/* v1 <== v2 */
	{
	int i;
	for( i = 0; i < 4; i ++ )
		v1[i] = v2[i];
	}

void matident( matrix m )			/* make a unit mat */
	{
	int i,j;

	for( i = 0; i < 4; i ++ )
		for( j = 0; j < 4; j ++ )
			m[i][j] = 0.0;

	for( i = 0; i < 4; i ++ )
		m[i][i] = 1.0;
	}

void mattrans( matrix m, double tx, double ty, double tz )
	{
	matrix t;

	matident( t );

	t[3][0] = tx;
	t[3][1] = ty;
	t[3][2] = tz;

	matmult( m, m, t );
	}

void matmult( matrix m, matrix m1, matrix m2 )
	{
	matrix t;
	int    i,j,k;

	for( i = 0; i < 4; i ++ )
		for( j = 0; j < 4; j ++ )
			{
			t[i][j] = 0.0;

			for( k = 0; k < 4; k ++ )
				t[i][j] += m1[i][k] * m2[k][j];
			}
	matcopy( m, t );
	}

void matrotat( matrix m, double rx, double ry, double rz )
	{
	double sx,sy,sz,cx,cy,cz;
	matrix r;

	rx = rx/180.0*PI;
	ry = ry/180.0*PI;
	rz = rz/180.0*PI;

	sx = sin( rx );		cx = cos( rx );
	sy = sin( ry );		cy = cos( ry );
	sz = sin( rz );		cz = cos( rz );

	matident( r );

	r[0][0] = cy * cz;
	r[0][1] = cy * sz;
	r[0][2] = - sy;
	r[1][0] = sx * sy * cz - cx * sz;
	r[1][1] = sx * sy * sz + cx * cz;
	r[1][2] = sx * cy;
	r[2][0] = cx * sy * cz + sx * sz;
	r[2][1] = cx * sy * sz - sx * cz;
	r[2][2] = cx * cy;

	matmult( m, m, r );
	}

void matscale( matrix m, double sx, double sy, double sz )
	{
	matrix t;

	matident( t );

	t[0][0] = sx;
	t[1][1] = sy;
	t[2][2] = sz;

	matmult( m, m, t );
	}

void vecmult( vector v1, vector v2, matrix m )	/* v1 = v2 * m */
	{
	vector t;
	int    i, j;

	for( i = 0; i < 4; i ++ )
		{
		t[i] = 0.0;

		for( j = 0; j < 4; j ++ )
			t[i] += v2[j] * m[j][i];
		}

	veccopy( v1, t );
	}

/* the functions below only acts to the first three dim of a vector, the forth one keeps to 1.0  */

double dot( vector v1, vector v2 )
	{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

double dist( vector v1, vector v2 )
	{
	return sqrt( (v1[0]-v2[0])*(v1[0]-v2[0]) +
			  (v1[1]-v2[1])*(v1[1]-v2[1]) +
		     (v1[2]-v2[2])*(v1[2]-v2[2]) );
	}

void cross( vector v1, vector v2, vector v3 )
	{
	vector t;

	t[0] = v2[1] * v3[2] - v3[1] * v2[2];
	t[1] = v2[2] * v3[0] - v2[0] * v3[2];
	t[2] = v2[0] * v3[1] - v3[0] * v2[1];
	t[3] = 1.0;

	veccopy( v1, t );
	}

double normalize( vector v )
	{
	int i;
	double l;

	l = sqrt( dot( v, v ) );
	if( l >= EPS )
		for( i = 0; i < 3; i ++ )
			v[i] /= l;

	return l;

	}

void vecplus( vector res, vector v1, vector v2 )
	{
	int i;
	for( i = 0; i < 3; i ++ )
		res[i] = v1[i] + v2[i];
	}

void vecminus( vector res, vector v1, vector v2 )
	{
	int i;
	for( i = 0; i < 3; i ++ )
		res[i] = v1[i] - v2[i];
	}

void vecmulv( vector res, vector v1, double val )
	{
	res[0] = v1[0]*val;
	res[1] = v1[1]*val;
	res[2] = v1[2]*val;
	}

int  vecdivv( vector res, vector v1, double val )
	{
	if( fabs(val) < EPS )
		return -1;

	res[0] = v1[0]/val;
	res[1] = v1[1]/val;
	res[2] = v1[2]/val;
	return 1;
	}

int vecnull( vector v, double tol )
	{
	if( dot( v, v ) < tol * tol )
		return 1;
	return 0;
	}

int vecequal( vector v1, vector v2 )
	{
	vector t;

	vecminus( t, v1, v2 );

	return vecnull( t, EPS );
	}

int relpf( vector v, vector faceq, double * d )
	{
	int  	 ret;

	(*d) = dot(v, faceq) + faceq[3];
	if( (*d) < 0.0 ) ret = -1;
	else if( (*d) > 0.0 ) ret = 1;
	else ret = 0;

	return ret;
	}

int relpfXYZ( double x, double y, double z, vector faceq, double * d )
	{
	int	 ret;

	(*d) = faceq[0]*x + faceq[1]*y + faceq[2]*z + faceq[3];
	if( (*d) < 0.0 ) ret = -1;
	else if( (*d) > 0.0 ) ret = 1;
	else ret = 0;

	return ret;
	}

int jointpf( vector p1, vector p2, vector f, vector j )
	{
	double d, factor;
	vector p, q;

	vecminus( p, p1, p2 );

	d = dot( p, f );
	if( fabs(d) < EPS ) return -1;  		// parallel

	factor = -( dot(p2,f) + f[3] )/d;
	j[0] = p2[0] + p[0]*factor;
	j[1] = p2[1] + p[1]*factor;
	j[2] = p2[2] + p[2]*factor;

	vecminus( p, j, p1 );
	vecminus( q, j, p2 );
	d = dot( p, q );
	if( fabs(d) <= EPS ) return 0;		// on the face
	if( d > EPS ) return 1;					// on same side
	return 2;									// on diff side
	}


/* End of file */