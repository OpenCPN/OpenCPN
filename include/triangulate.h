
/*
    This code is in the public domain. Specifically, we give to the public
    domain all rights for future licensing of the source code, all resale
    rights, and all publishing rights.

    UNC-CH GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE SOFTWARE
    AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION, WARRANTY
    OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.


                                    - Atul Narkhede (narkhede@cs.unc.edu)
*/
#ifndef _triangulate_h
#define _triangulate_h

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

//    Short section from old interface.h
#define TRUE 1
#define FALSE 0

#ifndef NULL
#define NULL 0
#endif


//

typedef struct {
  double x, y;
} point_t, vector_t;


/* Segment attributes */

typedef struct {
  point_t v0, v1;       /* two endpoints */
  int is_inserted;            /* inserted in trapezoidation yet ? */
  int root0, root1;           /* root nodes in Q */
  int next;             /* Next logical segment */
  int prev;             /* Previous segment */
} segment_t;


/* Trapezoid attributes */

typedef struct {
  int lseg, rseg;       /* two adjoining segments */
  point_t hi, lo;       /* max/min y-values */
  int u0, u1;
  int d0, d1;
  int sink;             /* pointer to corresponding in Q */
  int usave, uside;           /* I forgot what this means */
  int state;
} trap_t;


/* Node attributes for every node in the query structure */

typedef struct {
  int nodetype;               /* Y-node or S-node */
  int segnum;
  point_t yval;
  int trnum;
  int parent;                 /* doubly linked DAG */
  int left, right;            /* children */
} node_t;


typedef struct {
  int vnum;
  int next;             /* Circularly linked list  */
  int prev;             /* describing the monotone */
  int marked;                 /* polygon */
} monchain_t;


typedef struct {
  point_t pt;
  int vnext[4];               /* next vertices for the 4 chains */
  int vpos[4];                /* position of v in the 4 chains */
  int nextfree;
} vertexchain_t;


typedef struct {
      int is_valid;
      int id_poly;
      int nvert;
      int *vertex_index_list;
      void *poly_next;
      int index_hash;
} polyout;



//    Integer types

typedef struct {
      int x, y;
} ipoint_t, ivector_t;

/* Trapezoid attributes */
typedef struct {
      int lseg, rseg;       /* two adjoining segments */
      ipoint_t hi, lo;       /* max/min y-values */
      int u0, u1;
      int d0, d1;
      int sink;             /* pointer to corresponding in Q */
      int usave, uside;           /* I forgot what this means */
      int state;
      int inside;
      int ase;                // TODO remove debug
} itrap_t;

/* Segment attributes */
typedef struct {
      ipoint_t v0, v1;       /* two endpoints */
      int is_inserted;            /* inserted in trapezoidation yet ? */
      int root0, root1;           /* root nodes in Q */
      int next;             /* Next logical segment */
      int prev;             /* Previous segment */
} isegment_t;




typedef struct {
      int nodetype;               /* Y-node or S-node */
      int segnum;
      ipoint_t yval;
      int trnum;
      int parent;                 /* doubly linked DAG */
      int left, right;            /* children */
} inode_t;




#ifdef __cplusplus
extern "C" polyout *triangulate_polygon(int, int[], double (*)[2]);
extern "C" int int_trapezate_polygon(int, int[], double (*)[2], itrap_t **, isegment_t **, int *);
#else /* __cplusplus */
extern polyout *triangulate_polygon(int, int[], double (*)[2]);
extern int int_trapezate_polygon(int, int[], double (*)[2], itrap_t **, isegment_t **, int *);
#endif

//extern int triangulate_polygon(int, int *, double (*)[2], int (*)[3]);
extern int is_point_inside_polygon(double *);
//extern polyout *trapezate_polygon(int, int *, double (*)[2]);

polyout  *do_triangulate_polygon(int ncontours, int cntr[], double (*vertices)[2]);




/* Node types */

#define T_X     1
#define T_Y     2
#define T_SINK  3




#define TRUE  1
#define FALSE 0


#define FIRSTPT 1       /* checking whether pt. is inserted */
#define LASTPT  2


#ifndef TRI_INFINITY
 #define TRI_INFINITY 1<<30
#endif

#define C_EPS 1.0e-8          /* tolerance value: Used for making */
                        /* all decisions about collinearity or */
                        /* left/right of segment. Decrease */
                        /* this value if the input points are */
                        /* spaced very close together */


#define S_LEFT 1        /* for merge-direction */
#define S_RIGHT 2


#define ST_VALID 1            /* for trapezium state */
#define ST_INVALID 2


#define SP_SIMPLE_LRUP 1      /* for splitting trapezoids */
#define SP_SIMPLE_LRDN 2
#define SP_2UP_2DN     3
#define SP_2UP_LEFT    4
#define SP_2UP_RIGHT   5
#define SP_2DN_LEFT    6
#define SP_2DN_RIGHT   7
#define SP_NOSPLIT    -1

#define TR_FROM_UP 1          /* for traverse-direction */
#define TR_FROM_DN 2

#define TRI_LHS 1
#define TRI_RHS 2


//#define MAX(a, b) (((a) > (b)) ? (a) : (b))
//#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define CROSS(v0, v1, v2) (((v1).x - (v0).x)*((v2).y - (v0).y) - \
                     ((v1).y - (v0).y)*((v2).x - (v0).x))

#define DOT(v0, v1) ((v0).x * (v1).x + (v0).y * (v1).y)

#define FP_EQUAL(s, t) (fabs(s - t) <= C_EPS)


/* Global variables */

//extern node_t qs[QSIZE];            /* Query structure */
//extern trap_t tr[TRSIZE];           /* Trapezoid structure */
//extern segment_t seg[SEGSIZE];            /* Segment table */


/* Functions */

extern int monotonate_trapezoids(int);
extern int triangulate_monotone_polygons(int, int, int (*)[3]);

extern int _greater_than(point_t *, point_t *);
extern int _equal_to(point_t *, point_t *);
extern int _greater_than_equal_to(point_t *, point_t *);
extern int _less_than(point_t *, point_t *);
extern int locate_endpoint(point_t *, point_t *, int);
extern int construct_trapezoids(int);

extern int generate_random_ordering(int);
extern int choose_segment(void);
extern int read_segments(char *, int *);
extern int math_logstar_n(int);
extern int math_N(int, int);

#if __GNUC__ >= 5
extern inline int int_greater_than(ipoint_t *v0, ipoint_t *v1);
extern inline int int_equal_to(ipoint_t *v0, ipoint_t *v1);
extern inline int int_greater_than_equal_to(ipoint_t *v0, ipoint_t *v1);
extern inline int int_less_than(ipoint_t *v0, ipoint_t *v1);
extern inline int int_is_left_of(int segnum, ipoint_t *v);
extern inline int int_max(ipoint_t *yval, ipoint_t *v0, ipoint_t *v1);
extern inline int int_min(ipoint_t *yval, ipoint_t *v0, ipoint_t *v1);
#ifndef __clang__
extern inline int int_locate_endpoint_a(ipoint_t *v, ipoint_t *vo, int r);
#endif
extern inline int int_locate_endpoint(ipoint_t *v, ipoint_t *vo, int r);
#endif

#endif /* triangulate_h */
