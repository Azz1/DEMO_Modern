#include <stdio.h>
/* gwb.c */


#include "gwb.h"
#include <stdlib.h>

Solid  *firsts;
Id		maxs;
Id		maxf;
Id		maxv;

unsigned nodesize[] =
	{
	sizeof(Solid),sizeof(Face),sizeof(Loop),sizeof(HalfEdge),
	sizeof(Edge),sizeof(Vertex),0
	};

void SetFirsts( Solid * f )
{
	firsts = f;
}

Solid * GetFirsts()
{
	return firsts;
}

void addlist(int what, Node *which, Node *where)
	{
	switch(what)
		{
		case SOLID:
			which -> s.nexts = firsts;
			which -> s.prevs = (Solid *)NULL;
			if(firsts != NULL)
				firsts -> prevs = (Solid *)which;
			firsts = (Solid *)which;

			break;

		case FACE:
			which -> f.nextf = where -> s.sfaces;
			which -> f.prevf = (Face *)NULL;
			if(where -> s.sfaces != NULL)
				where -> s.sfaces -> prevf = (Face *)which;
			where -> s.sfaces = (Face *)which;
			which -> f.fsolid = (Solid *)where;
			break;

		case LOOP:
			which -> l.nextl = where -> f.floops;
			which -> l.prevl = (Loop *)NULL;
			if(where -> f.floops != NULL)
				where -> f.floops -> prevl = (Loop *)which;
			where -> f.floops = (Loop *)which;
			which -> l.lface = (Face *)where;
			break;

		case EDGE:
			which -> e.nexte = where -> s.sedges;
			which -> e.preve = (Edge *)NULL;
			if(where -> s.sedges  != NULL)
				where -> s.sedges -> preve = (Edge *)which;
			where -> s.sedges = (Edge *)which;
			break;

		case VERTEX:
			which -> v.nextv = where -> s.sverts;
			which -> v.prevv = (Vertex *)NULL;
			if(where -> s.sverts != NULL)
				where -> s.sverts -> prevv = (Vertex *)which;
			where -> s.sverts = (Vertex *)which;
			break;
		}
	}

void dellist(int what, Node *which, Node *where)
	{
	switch(what)
		{
		case SOLID:
			if(which -> s.nexts != NULL)
				which -> s.nexts -> prevs = which -> s.prevs;
			if(which -> s.prevs != NULL)
				which -> s.prevs -> nexts = which -> s.nexts;
			else
				firsts = which -> s.nexts;
			break;

		case FACE:
			if(which -> f.nextf != NULL)
				which -> f.nextf -> prevf = which -> f.prevf;
			if(which -> f.prevf != NULL)
				which -> f.prevf -> nextf = which -> f.nextf;
			else
				where -> s.sfaces = which -> f.nextf;
			break;

		case LOOP:
			if(which -> l.nextl != NULL)
				which -> l.nextl -> prevl = which -> l.prevl;
			if(which -> l.prevl != NULL)
				which -> l.prevl -> nextl = which -> l.nextl;
			else
				where -> f.floops = which -> l.nextl;
			break;

		case EDGE:
			if(which -> e.nexte != NULL)
				which -> e.nexte -> preve = which -> e.preve;
			if(which -> e.preve != NULL)
				which -> e.preve -> nexte = which -> e.nexte;
			else
				where -> s.sedges = (Edge *)which -> e.nexte;
			break;

		case VERTEX:
			if(which -> v.nextv != NULL)
				which -> v.nextv -> prevv = which -> v.prevv;
			if(which -> v.prevv != NULL)
				which -> v.prevv -> nextv = which -> v.nextv;
			else
				where -> s.sverts = (Vertex *)which -> v.nextv;
			break;
		}
	}

HalfEdge * addhe(Edge *e, Vertex *v, HalfEdge *where, int sign)
	{
	HalfEdge *he;

	if(where -> edg == NULL)
		{
		he = where;
		}
	else
		{
		he = (HalfEdge *)New(HALFEDGE,NULL);
		where -> prv -> nxt = he;
		he -> prv = where -> prv;
		where -> prv = he;
		he -> nxt = where;
		}
	he -> edg = e;
	he -> vex = v;
	he -> wloop = where -> wloop;
	if(sign == PLUS)
		e -> he1 = he;
	else
		e -> he2 = he;
	return he;
	}

HalfEdge *  delhe(HalfEdge *he)
	{
	HalfEdge * temp;

	if(he -> edg == NULL)
		{
		he -> wloop -> ledg = NULL;
		Del(HALFEDGE,(Node *)he,NULL);
		return NULL;
		}
	else if(he -> nxt == he)
		{
		he -> edg = NULL;
		return he;
		}
	else
		{
		he -> prv -> nxt = he -> nxt;
		he -> nxt -> prv = he -> prv;

		temp = he -> prv;
		if( temp == he ) temp = NULL;

		if(he -> wloop -> ledg == he)
			he -> wloop -> ledg = temp;
		Del(HALFEDGE,(Node *)he,NULL);
		return temp;
		}
	}

Node * New(int what , Node *where)
	{
	Node *node = (Node *)malloc(nodesize[what]);
	if( node == NULL )
		abort();

	switch(what)
		{
		case SOLID:
			addlist(SOLID,node,NULL);
			node -> s.sfaces = (Face *)NULL;
			node -> s.sedges = (Edge *)NULL;
			node -> s.sverts = (Vertex *)NULL;
			break;

		case FACE:
			addlist(FACE,node,where);
			node -> f.floops = (Loop *)NULL;
			node -> f.flout = (Loop *)NULL;
			break;

		case LOOP:
			addlist(LOOP,node, where);
			break;

		case EDGE:
			addlist(EDGE,node,where);
			break;

		case VERTEX:
			addlist(VERTEX,node,where);
			node ->  v.vedge = NULL;
			break;
		default:
			break;
		}
	return node;
	}

void Del(int what, Node *which, Node *where)
	{
	switch(what)
		{
		case SOLID:
			dellist(SOLID,which,where);
			free((Solid *)which);
			break;

		case FACE:
			dellist(FACE,which,where);
			free((Face *)which);
			break;

		case LOOP:
			dellist(LOOP,which,where);
			if((Node *)where -> f.flout == which)
				where -> f.flout = (Loop *)NULL;
			free((Loop *)which);
			break;

		case EDGE:
			dellist(EDGE,which,where);
			free((Edge *)which);
			break;

		case VERTEX:
			dellist(VERTEX,which,where);
			free((Vertex *)which);
			break;

		case HALFEDGE:
			free((HalfEdge *)which);
			break;
		}
	}

void Remove( Solid * s )
	{
	Face	 	* f;
	Loop		* l;
	HalfEdge	* he;

	f = s -> sfaces;
	while( f != NULL )
		{
		l = f -> floops;
		while( l != NULL )
			{
			he = l -> ledg;
			while( he != NULL )
				he = delhe( he );
			Del( LOOP, (Node *)l, (Node *)f );
			l = f -> floops;
			}
		Del( FACE, (Node *)f, (Node *)s );
		f = s -> sfaces;
		}

	while( s -> sedges != NULL )
		Del( EDGE, (Node *)s -> sedges, (Node *)s );

	while( s -> sverts != NULL )
		Del( VERTEX, (Node *)s -> sverts, (Node *)s );

	Del( SOLID, (Node *)s, NULL );
	}



void RemoveAll()
	{
	Solid * s;

	s = firsts;
	while( s != NULL )
		{
		Remove( s );
		s = firsts;
		}
	}

void listsolid( Solid *s, FILE * fp )
	{
	Face 		*f;
	Loop		*l;
	HalfEdge	*he;
	int 	 	 fnum, vnum;

	fnum = vnum = 0;

	fprintf( fp, "\nsolid %d :\n", s -> solidno );
	f = s -> sfaces;
	while( f )
		{
		fnum ++;

		fprintf( fp, "\tface %d :\n", f -> faceno );
		l = f -> floops;
		while( l )
			{
			fprintf( fp, "\t\tloop:\n" );
			he = l -> ledg;
			do
				{
				vnum ++;

				fprintf( fp, "\t\t\t%d:( %f %f %f )\n",
					he -> vex -> vertexno,
					he -> vex -> vcoord[0],
					he -> vex -> vcoord[1],
					he -> vex -> vcoord[2] );

				}while( ( he = he -> nxt ) != l -> ledg );

			fprintf( fp, "\n" );
			l = l -> nextl;
			}
		f = f -> nextf;
		}

	fprintf( fp, "\nsolid %d total facenum: %d\tvertexnum: %d\n", s -> solidno, fnum, vnum );
	}

void listall()
	{
	FILE  * fp;
	Solid * s = firsts;

	if( (fp=fopen("Solids.lst", "w")) == NULL )
		{
		fprintf(stderr, "Error\n");
		return;
		}

	while( s != NULL )
		{
		listsolid( s, fp );
		s = s -> nexts;
		}
	fclose( fp );
	}

void listneighbors( Vertex *v )
	{
	FILE      	* fp;
	HalfEdge 	* adj;

	if( (fp=fopen("Neighbor.lst", "w")) == NULL )
		{
		fprintf(stderr, "Error\n");
		return;
		}

	adj = v -> vedge;
	if( adj )
		do
			{
			fprintf( fp, " %d ",adj -> nxt -> vex -> vertexno );

			}while( ( adj = mate(adj) -> nxt ) != v -> vedge );
	else
		fprintf( fp, "no adjacent vertices" );

	fprintf( fp, "\n" );

	fclose( fp );
	}


/* End of file */
