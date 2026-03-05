#if !defined(_GWB_MAT_H)
#define  _GWB_MAT_H		1

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#include <math.h>

#define	PI		3.141592653589793
#define 	FPS	0.0001

extern  double 	EPS;		/* tolerance for geometric tests */
extern  double 	BIGEPS;		/* a more permissive tolerance */

typedef double  	vector[4];
typedef double		matrix[4][4];

EXTERN_C void matcopy( matrix m1, matrix m2 );		/* m1 <== m2 */
EXTERN_C void veccopy( vector v1, vector v2 );		/* v1 <== v2 */

EXTERN_C void matident( matrix m );			/* make a unit mat */

EXTERN_C void matmult( matrix m, matrix m1, matrix m2 );
EXTERN_C void mattrans( matrix m, double tx, double ty, double tz );
EXTERN_C void matrotat( matrix m, double rx, double ry, double rz );
EXTERN_C void matscale( matrix m, double sx, double sy, double sz );
EXTERN_C void vecmult( vector v1, vector v2, matrix m );	/* v1 = v2 * m */

/* the functions below only acts to the first three dim of a vector, the forth one keeps to 1.0  */

EXTERN_C double dot( vector v1, vector v2 );
EXTERN_C double dist( vector v1, vector v2 );
EXTERN_C void cross( vector v1, vector v2, vector v3 );
EXTERN_C double normalize( vector v );
EXTERN_C void vecplus( vector res, vector v1, vector v2 );
EXTERN_C void vecminus( vector res, vector v1, vector v2 );
EXTERN_C void vecmulv( vector res, vector v1, double val );
EXTERN_C int  vecdivv( vector res, vector v1, double val );
EXTERN_C int vecnull( vector v, double tol );
EXTERN_C int vecequal( vector v1, vector v2 );
EXTERN_C int relpf( vector v, vector faceq, double * d );
EXTERN_C int relpfXYZ( double x, double y, double z, vector faceq, double * d );
EXTERN_C int jointpf( vector p1, vector p2, vector f, vector j );

#endif

/* End of file */
