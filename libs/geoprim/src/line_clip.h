//-------------------------------------------------------------------------------------------------------
//  Cohen & Sutherland Line clipping algorithms
//-------------------------------------------------------------------------------------------------------
/*
 *
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
 *
 */

typedef enum { Visible, Invisible } ClipResult;

#ifdef __cplusplus
extern "C" ClipResult cohen_sutherland_line_clip_d(double *x0, double *y0,
                                                   double *x1, double *y1,
                                                   double xmin_, double xmax_,
                                                   double ymin_, double ymax_);

extern "C" ClipResult cohen_sutherland_line_clip_i(int *x0, int *y0, int *x1,
                                                   int *y1, int xmin_,
                                                   int xmax_, int ymin_,
                                                   int ymax_);

#endif

