/*
 * ssfn.h
 *
 * Copyright (C) 2019 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @brief Scalable Screen Font renderer in a single ANSI C/C++ header
 *
 */

#ifndef _SSFN_H_
#define _SSFN_H_

#ifdef  __cplusplus
extern "C" {
#endif

/* if stdint.h was not included before us */
#ifndef uint8_t
typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;
#endif

/***** file format *****/

/* magic bytes */
#define SSFN_MAGIC "SSFN"
#define SSFN_COLLECTION "SFNC"
#define SSFN_ENDMAGIC "NFSS"

/* font family group */
#define SSFN_FAMILY_SERIF       0
#define SSFN_FAMILY_SANS        1
#define SSFN_FAMILY_DECOR       2
#define SSFN_FAMILY_MONOSPACE   3
#define SSFN_FAMILY_HAND        4

/* font style flags */
#define SSFN_STYLE_REGULAR      0
#define SSFN_STYLE_BOLD         1
#define SSFN_STYLE_ITALIC       2

/* file format features */
#define SSFN_FEAT_HASBMAP       1       /* there's at least one bitmap fragment */
#define SSFN_FEAT_HASCMAP       2       /* there's at least one pixmap fragment or one color command, so a color map too */
#define SSFN_FEAT_HASHINT       4       /* there's at least one hinting fragment */
#define SSFN_FEAT_KBIGLKP       8       /* big offsets in kerning look up table */
#define SSFN_FEAT_KBIGCHR      16       /* big characters in kerning look up tables */
#define SSFN_FEAT_KBIGCRD      32       /* big coordinates in kerning groups */
#define SSFN_FEAT_HBIGCRD      64       /* bit coordinates in autohinting fragments */

/* contour commands for vector fragments */
#define SSFN_CONTOUR_MOVE       0
#define SSFN_CONTOUR_LINE       1
#define SSFN_CONTOUR_QUAD       2
#define SSFN_CONTOUR_CUBIC      3
#define SSFN_CONTOUR_COLOR      4

/* bitmap and pixmap fragments and hinting grid info */
#define SSFN_FRAG_BITMAP        0
#define SSFN_FRAG_LBITMAP       1
#define SSFN_FRAG_PIXMAP        2
#define SSFN_FRAG_HINTING       3

/* glyph variants */
#define SSFN_VARIANT_DEFAULT    0       /* default, also isolated glyphs */
#define SSFN_VARIANT_LOCAL0     0
#define SSFN_VARIANT_INITIAL    1       /* currently not needed because contextual glyphs has their own UNICODE code points, */
#define SSFN_VARIANT_LOCAL1     1       /* so its only purpose is to store Serbian / Macedonian B D G P T */
#define SSFN_VARIANT_MEDIAL     2
#define SSFN_VARIANT_LOCAL2     2
#define SSFN_VARIANT_FINAL      3
#define SSFN_VARIANT_LOCAL3     3
#define SSFN_VARIANT_LOCAL4     4
#define SSFN_VARIANT_LOCAL5     5
#define SSFN_VARIANT_LOCAL6     6
#define SSFN_NUMVARIANTS        7

/* main SSFN header */
typedef struct {
    uint8_t     magic[4];               /* SSFN magic bytes */
    uint32_t    size;                   /* total size in bytes */
    uint8_t     family;                 /* font family group */
    uint8_t     style;                  /* font style, zero or OR'd SSFN_STYLE_BOLD and SSFN_STYLE_ITALIC */
    uint8_t     quality;                /* quality, defines grid size, 0 - 8 */
    uint8_t     features;               /* feature flags, OR'd SSFN_FEAT_* */
    uint8_t     revision;               /* format revision, must be zero */
    uint8_t     reserved0;              /* must be zero */
    uint16_t    reserved1;
    uint16_t    baseline;               /* horizontal baseline in grid pixels */
    uint16_t    underline;              /* position of under line in grid pixels */
    uint16_t    bbox_left;              /* overall bounding box for all glyphs in grid pixels */
    uint16_t    bbox_top;
    uint16_t    bbox_right;
    uint16_t    bbox_bottom;
    uint32_t    fragments_offs;         /* offset of fragments table relative to magic */
    uint32_t    characters_offs[SSFN_NUMVARIANTS];  /* offset of characters tables per variant relative to magic */
    uint32_t    kerning_offs;           /* kerning table offset relative to magic */
}  ssfn_font_t;

/***** renderer API *****/
#define SSFN_FAMILY_ANY      0xff       /* select the first loaded font */
#define SSFN_FAMILY_BYNAME   0xfe       /* select font by its unique name */

#define SSFN_STYLE_UNDERLINE    4       /* under line glyph */
#define SSFN_STYLE_STHROUGH     8       /* strike through glyph */
#define SSFN_STYLE_NOHINTING 0x40       /* no auto hinting grid */
#define SSFN_STYLE_ABS_SIZE  0x80       /* use absolute size value */

#define SSFN_FRAG_CONTOUR     255

/* error codes */
#define SSFN_OK                 0       /* success */
#define SSFN_ERR_ALLOC          1       /* allocation error */
#define SSFN_ERR_NOFACE         2       /* no font face selected */
#define SSFN_ERR_INVINP         3       /* invalid input */
#define SSFN_ERR_BADFILE        4       /* bad SSFN file format */
#define SSFN_ERR_BADSTYLE       5       /* bad style */
#define SSFN_ERR_BADSIZE        6       /* bad size */
#define SSFN_ERR_BADMODE        7       /* bad mode */
#define SSFN_ERR_NOGLYPH        8       /* glyph (or kerning info) not found */
#define SSFN_ERR_NOVARIANT      9       /* no such glyph variant */

/* rendering modes */
#define SSFN_MODE_NONE          0       /* just select the font to get the glyph (for kerning) */
#define SSFN_MODE_OUTLINE       1       /* return the glyph's outlines */
#define SSFN_MODE_BITMAP        2       /* render into bitmap */
#define SSFN_MODE_ALPHA         3       /* render into alpha channel */
#define SSFN_MODE_CMAP          4       /* render into color map indexed buffer */

/* grid fitting */
#define SSFN_HINTING_THRESHOLD 16       /* don't change unless you really know what you're doing */

/* returned bitmap struct */
#define SSFN_DATA_MAX       65536
typedef struct {
    uint8_t mode;                       /* returned glyph's data format */
    uint8_t baseline;                   /* baseline of glyph, scaled to size */
    uint8_t w;                          /* width */
    uint8_t h;                          /* height */
    uint8_t adv_x;                      /* advance x */
    uint8_t adv_y;                      /* advance y */
    uint16_t pitch;                     /* data buffer bytes per line */
    uint32_t *cmap;                     /* pointer to color map */
    uint8_t data[SSFN_DATA_MAX];        /* data buffer */
} ssfn_glyph_t;

/* renderer context */
typedef struct {
    const ssfn_font_t **fnt[5];         /* font registry */
    const ssfn_font_t *s;               /* explicitly selected font */
    const ssfn_font_t *f;               /* font selected by best match */
    ssfn_glyph_t *ret;                  /* glyph to return */
    uint16_t *p;                        /* outline points */
    uint16_t *r[256];                   /* raster for scanlines */
    uint16_t *h;                        /* auto hinting grid */
    int len[5];                         /* number of fonts in registry */
    int mp;                             /* memory allocated for points */
    int np;                             /* how many points actually are there */
    int nr[256];                        /* number of coordinates in each raster line */
    int err;                            /* returned error code */
    int family;                         /* required family */
    int style;                          /* required style */
    int size;                           /* required size */
    int mode;                           /* required mode */
    int variant;                        /* required variant */
    int g;                              /* shift value for grid size */
    int m, ix, u,uix,uax, lx,ly, mx,my; /* helper variables */
} ssfn_t;

/***** API function protoypes *****/

/* normal renderer */
int ssfn_load(ssfn_t *ctx, const ssfn_font_t *font);                                    /* add an SSFN to context */
int ssfn_select(ssfn_t *ctx, int family, char *name, int style, int size, int mode);    /* select font to use */
int ssfn_variant(ssfn_t *ctx, int variant);                                             /* select glyph variant (optional) */
uint32_t ssfn_utf8(char **str);                                                         /* decode UTF-8 sequence */
ssfn_glyph_t *ssfn_render(ssfn_t *ctx, uint32_t unicode);       /* return allocated glyph bitmap */
int ssfn_kern(ssfn_t *ctx, uint32_t unicode, uint32_t nextunicode, int *x, int *y);     /* get kerning values */
int ssfn_bbox(ssfn_t *ctx, char *str, int usekern, int *w, int *h);                     /* get bounding box of a rendered string */
int ssfn_mem(ssfn_t *ctx);                                                              /* return how much memory is used */
void ssfn_free(ssfn_t *ctx);                                                            /* free context */
#define ssfn_lasterr(ctx) ((ssfn_t*)ctx)->err                                           /* return last error code */
#define ssfn_error(err) (err>=0&&err<=9?ssfn_errstr[err]:"Unknown error")               /* return string for error code */
extern const char *ssfn_errstr[];

/* simple renderer */
int ssfn_putc(uint32_t unicode);                                                        /* render console bitmap font */

/***** renderer implementations *****/

#ifndef SSFN_NOIMPLEMENTATION
/*** normal renderer (ca. 22k, fully featured with error checking) ***/

# ifndef NULL
#  define NULL (void*)0
# endif
//# ifndef size_t
//   typedef __SIZE_TYPE__ size_t;
//# endif
# ifndef ssfn_private
//#  define ssfn_private __attribute__((__visibility__("hidden")))
#define ssfn_private
# endif
//# ifndef inline
//#  define inline __inline__
//# endif

/* Clang does not have built-in versions of these but gcc has */
# ifndef SSFN_memcmp
#  ifdef __builtin_memcmp
#   define SSFN_memcmp __builtin_memcmp
#  else
#   define SSFN_memcmp memcmp
    extern int memcmp (const void *__s1, const void *__s2, size_t __n);
#  endif
# endif

# ifndef SSFN_memset
#  ifdef __builtin_memset
#   define SSFN_memset __builtin_memset
#  else
#   define SSFN_memset memset
    extern void *memset (void *__s, int __c, size_t __n);
#  endif
# endif

# ifndef SSFN_realloc
#  ifdef __builtin_realloc
#   define SSFN_realloc __builtin_realloc
#  else
#   define SSFN_realloc realloc
#ifdef __MSVC__
    extern void *realloc (void *__ptr, size_t __size);
#else
    extern void *realloc (void *__ptr, size_t __size) noexcept;
#endif
#  endif
# endif

# ifndef SSFN_free
#  ifdef __builtin_free
#   define SSFN_free __builtin_free
#  else
#   define SSFN_free free
#ifdef __MSVC__
    extern void free (void *p);
#else
    extern void free (void *p) __THROW;
#endif
#  endif
# endif

/**
 * Error code strings
 */
const char *ssfn_errstr[] = { "",
    "Memory allocation error",
    "No font face found",
    "Invalid input value",
    "Bad file format",
    "Invalid style",
    "Invalid size",
    "Invalid mode",
    "Glyph not found",
    "Glyph variant not found"
};

/*** Private functions ***/
/* f = file scale, g = grid 4095.15, o = screen point 255.255, i = screen pixel 255, c = ceil */
# define _ssfn_i2g(x)   ((x)? (((x) << 16) - (1 << 15)) / ctx->m : 0)
# define _ssfn_g2o(x)   (((x) * ctx->m + (1 << 7)) >> 8)
# define _ssfn_g2i(x)   (((x) * ctx->m + (1 << 15)) >> 16)
# define _ssfn_g2ic(x)  (((x) * ctx->m + (1 << 16) - 1) >> 16)
# define _ssfn_f2i(x)   ((((x) << s) * ctx->m + (1 << 15)) >> 16)
# define _ssfn_o2i(x)   (((x) + (1 << 7)) >> 8)
# define _ssfn_o2ic(x)  ((x + (1<<8) - 1) >> 8)
# define _ssfn_g2ox(x)  ((x)>=(4095 << 4) ? _ssfn_g2o(x) : ctx->h[((x) >> 4)])
# define _ssfn_g2ix(x)  ((x)>=(4095 << 4) ? _ssfn_g2i(x) : _ssfn_o2i(ctx->h[((x) >> 4)]))
# define _ssfn_g2ixc(x) ((x)>=(4095 << 4) ? _ssfn_g2ic(x) : _ssfn_o2ic(ctx->h[((x) >> 4)]))
# define _ssfn_g2oy(y)  (_ssfn_g2o(y))
# define _ssfn_g2iy(y)  (_ssfn_g2i(y))
# define _ssfn_g2iyc(y) (_ssfn_g2ic(y))
# define _ssfn_igg(y)   (((4096<<4)-(y)) >> (2))
# define _ssfn_igi(y)   ((((4096<<4)-(y)) * ctx->m + (1 << (15+3))) >> (16+3))

/* parse character table */
ssfn_private uint8_t *_ssfn_c(const ssfn_font_t *font, uint32_t unicode, int v)
{
    uint32_t i, j, l;
    uint8_t *ptr;

    if(!font->characters_offs[v]) return NULL;

    ptr = (uint8_t*)font + font->characters_offs[v];
    l = (font->quality < 5 && font->characters_offs[v] < 65536) ? 4 : (font->characters_offs[v] < 1048576 ? 5 : 6);

    for(j=i=0;i<0x110000;i++) {
        if(ptr[0] & 0x80) {
            if(ptr[0] & 0x40) { i += ptr[1] | ((ptr[0] & 0x3f) << 8); ptr += 2; }
            else { i += ptr[0] & 0x3f; ptr++; }
        } else {
            if(i == unicode) return ptr;
            ptr += ptr[0] * l + 10;
            j++;
        }
    }
    return NULL;
}

/* add a line to contour */
ssfn_private void _ssfn_l(ssfn_t *ctx, int x, int y, int l)
{
    if(x > (4096<<4) - 16) x = (4096<<4) - 16;
    if(y > (4096<<4) - 16) y = (4096<<4) - 16;
    if(x < -1 || y < -1 || (x == ctx->lx && y == ctx->ly)) return;

    if(ctx->np+2 >= ctx->mp) {
        ctx->mp += 512;
        ctx->p = (uint16_t *)SSFN_realloc(ctx->p, ctx->mp*sizeof(uint16_t));
        if(!ctx->p) { ctx->err = SSFN_ERR_ALLOC; return; }
    }
    if(!ctx->np || !l || _ssfn_g2i(ctx->p[ctx->np-2]) != _ssfn_g2i(x) ||
        _ssfn_g2i(ctx->p[ctx->np-1]) != _ssfn_g2i(y)) {
            ctx->p[ctx->np++] = x;
            ctx->p[ctx->np++] = y;
            ctx->lx = x; ctx->ly = y;
    }
    if((ctx->style & 0x200) && x >= 0 && ctx->ix > x) ctx->ix = x;
}

/* add a Bezier curve to contour */
void _ssfn_b(ssfn_t *ctx, int x0,int y0, int x1,int y1, int x2,int y2, int x3,int y3, int l)
{
    int m0x, m0y, m1x, m1y, m2x, m2y, m3x, m3y, m4x, m4y,m5x, m5y;
    if(l<8 && (x0!=x3 || y0!=y3)) {
        m0x = ((x1-x0)/2) + x0;     m0y = ((y1-y0)/2) + y0;
        m1x = ((x2-x1)/2) + x1;     m1y = ((y2-y1)/2) + y1;
        m2x = ((x3-x2)/2) + x2;     m2y = ((y3-y2)/2) + y2;
        m3x = ((m1x-m0x)/2) + m0x;  m3y = ((m1y-m0y)/2) + m0y;
        m4x = ((m2x-m1x)/2) + m1x;  m4y = ((m2y-m1y)/2) + m1y;
        m5x = ((m4x-m3x)/2) + m3x;  m5y = ((m4y-m3y)/2) + m3y;
        _ssfn_b(ctx, x0,y0, m0x,m0y, m3x,m3y, m5x,m5y, l+1);
        _ssfn_b(ctx, m5x,m5y, m4x,m4y, m2x,m2y, x3,y3, l+1);
    }
    _ssfn_l(ctx, x3, y3, l);
}

/* rasterize contour */
ssfn_private void _ssfn_r(ssfn_t *ctx)
{
    int i, k, l, m, n = 0, x, y, Y, M = 0;
    uint16_t *r;
    uint8_t *pix = ctx->ret->data;

    for(y = 0; y < ctx->ret->h; y++) {
        Y = _ssfn_i2g(y); r = ctx->r[y];
        for(n = 0, i = 0; i < ctx->np - 3; i += 2) {
            if((ctx->p[i] == 0xffff && ctx->p[i+1] == 0xffff) ||
                (ctx->p[i+2] == 0xffff && ctx->p[i+3] == 0xffff)) continue;
            if( (ctx->p[i+1] < Y && ctx->p[i+3] >= Y) ||
                (ctx->p[i+3] < Y && ctx->p[i+1] >= Y)) {
                    if(_ssfn_g2iy(ctx->p[i+1]) == _ssfn_g2iy(ctx->p[i+3]))
                        x = (((int)ctx->p[i]+(int)ctx->p[i+2])>>1);
                    else
                        x = ((int)ctx->p[i]) + ((Y - (int)ctx->p[i+1])*
                            ((int)ctx->p[i+2] - (int)ctx->p[i])/
                            ((int)ctx->p[i+3] - (int)ctx->p[i+1]));
                    if(y == ctx->u) {
                        if(x < ctx->uix) { ctx->uix = x; }
                        if(x > ctx->uax) { ctx->uax = x; }
                    }
                    x = _ssfn_g2ox(x - ctx->ix);
                    for(k=0; k < n && x > r[k]; k++);
                    if(n >= ctx->nr[y]) {
                        ctx->nr[y] = (n < ctx->np) ? ctx->np : (n+1) << 1;
                        ctx->r[y] = (uint16_t *)SSFN_realloc(ctx->r[y], (ctx->nr[y] << 1));
                        if(!ctx->r[y]) { ctx->err = SSFN_ERR_ALLOC; return; }
                        r = ctx->r[y];
                    }
                    for(l = n; l > k; l--) r[l] = r[l-1];
                    r[k] = x;
                    n++;
            }
        }
        if(n>1 && n&1) { r[n-2] = r[n-1]; n--; }
        ctx->nr[y] = n;
        if(n) {
            if(y > M) M = y;
            k = y*ctx->ret->pitch;
            for(i=0; i < n-1; i += 2) {
                if(ctx->style & 0x100) {
                    l = (r[i] + r[i + 1]) >> 9;
                    if(ctx->mode == SSFN_MODE_BITMAP ? (pix[k + (l >> 3)] & (1 << (l & 7)) ? 1 : 0) : (pix[k + l] == 0xFF)) {
                        if(r[i] + 256 > r[i + 1]) r[i] = r[i + 1] - 128; else r[i] += 256;
                        if(r[i + 1] - 256 > r[i]) r[i + 1] -= 256;
                    } else {
                        if(i >= n-2 || r[i + 1] + 256 < r[i + 2]) r[i + 1] += 256;
                    }
                }
                l = ((r[i] + 128) >> 8);
                m = ((r[i + 1] + 128) >> 8);
                if(ctx->mode != SSFN_MODE_BITMAP && l + 1 < m) l++;
                for(; l < m; l++) {
                    switch(ctx->mode) {
                        case SSFN_MODE_BITMAP: pix[k + (l >> 3)] ^= 1 << (l & 7); break;
                        case SSFN_MODE_ALPHA: pix[k + l] ^= 0xFF; break;
                        case SSFN_MODE_CMAP: pix[k + l] ^= 0x0F; break;
                    }
                }
                if(l + 1 > ctx->ret->w) ctx->ret->w = l + 1;
            }
        }
    }
    /* fix rounding errors */
    if(M + 1 == ctx->ret->baseline) { ctx->ret->baseline--; ctx->u--; }
}

/* anti-alias a contour */
ssfn_private void _ssfn_a(ssfn_t * ctx)
{
    int i, x, y, x0,y0,x1,y1, sx,sy, dx,dy, m, o, p = ctx->ret->pitch;
    uint8_t *pix = ctx->ret->data, e;
    uint16_t *r;

    if(ctx->mode < SSFN_MODE_ALPHA || !ctx->p || ctx->np < 4) return;

    for(y = o = 0; y < ctx->ret->h; y++, o += p) {
        if(ctx->nr[y] && (r = ctx->r[y])) {
            for(i = 0; i < ctx->nr[y] - 1; i += 2) {
                x = (r[i] + 128) >> 8;
                e = ~((r[i] + 128) & 0xFF);
                if(e == 127) e = 255;
                else {
                    if(x && pix[o + x - 1] > e) e = pix[o + x - 1];
                    if(y && pix[o + x - p] > e) e = pix[o + x - p];
                }
                if(ctx->mode == SSFN_MODE_CMAP) e = (e >> 4) | 0xF0;
                if(e > pix[o + x]) pix[o + x] = e;
                e = ((r[i + 1] + 128) & 0xFF);
                if(e == 128) e = 255;
                pix[o + ((r[i + 1] + 128) >> 8)] = ctx->mode == SSFN_MODE_CMAP ? (e >> 4) | 0xF0 : e;
            }
        }
    }

    for(i = 0; i < ctx->np - 3; i += 2) {
        if((ctx->p[i] == 0xffff && ctx->p[i+1] == 0xffff) ||
            (ctx->p[i+2] == 0xffff && ctx->p[i+3] == 0xffff)) continue;

        x0 = ctx->p[i] - ctx->ix; y0 = ctx->p[i+1]; x1 = ctx->p[i+2] - ctx->ix; y1 = ctx->p[i+3];
        sx = x1 >= x0? 1 : -1; sy = y1 >= y0? 1 : -1; dx = x1 - x0; dy = y1 - y0;
        if(sx * dx >= sy * dy && dx) {
            for(x = x0, m = sx * dx; m > 0; m -= 16, x += sx * 16) {
                y = _ssfn_g2oy(y0 + ((x - x0) * dy / dx));
                y += 128; e = ~(y & 0xFF); y >>= 8; e >>= 1;
                o = y * p + ((_ssfn_g2ox(x) + 128) >> 8);
                if(ctx->mode == SSFN_MODE_CMAP) e = (e >> 4) | 0xF0;
                if(e > pix[o]) pix[o] = e;
            }
        }
    }
}

/* parse a glyph */
ssfn_private void _ssfn_g(ssfn_t *ctx, uint8_t *rg, int render)
{
    int i, j, nf, m, n, o, ox, oy, ol, t, x, y, a, b, c, d, w, h, s;
    uint8_t *raw, *ra, *re, *pix = ctx->ret->data;

    ctx->lx = ctx->ly = ctx->mx = ctx->my = -1;
    ol = (ctx->f->quality < 5 && ctx->f->characters_offs[ctx->variant] < 65536) ? 4 :
        ((ctx->f->characters_offs[ctx->variant] < 1048576) ? 5 : 6);
    s = 16 - ctx->g;
    nf = rg[0];
    rg += 10;

    for(h=0; nf-- && !ctx->err; rg += ol) {
        switch(ol) {
            case 4: o = ((rg[1] << 8) | rg[0]); ox = rg[2]; oy = rg[3]; break;
            case 5: o = (((rg[2] & 0xF) << 16) | (rg[1] << 8) | rg[0]);
                ox = (((rg[2] >> 4) & 3) << 8) | rg[3]; oy = (((rg[2] >> 6) & 3) << 8) | rg[4]; break;
            default: o = (((rg[3] & 0xF) << 24) | (rg[2] << 16) | (rg[1] << 8) | rg[0]);
                ox = ((rg[3] & 0xF) << 8) | rg[4]; oy = (((rg[3] >> 4) & 0xF) << 8) | rg[5]; break;
        }
        ox <<= s; oy <<= s; raw = (uint8_t*)ctx->f + o;
        if(raw[0] & 0x80) {
            t = (raw[0] & 0x60)>>5;
            if(!render && t != SSFN_FRAG_HINTING) break;
            switch(t) {
                case SSFN_FRAG_LBITMAP:
                    x = ((((raw[0]>>2)&3)<<8)+raw[1])+1;
                    y = (((raw[0]&3)<<8)|raw[2])+1;
                    raw += 3;
                    goto bitmap;

                case SSFN_FRAG_BITMAP:
                    x = (raw[0] & 0x1F)+1;
                    y = raw[1]+1;
                    raw += 2;
bitmap:             if(ctx->mode == SSFN_MODE_OUTLINE) {
                        x <<= 3;
outline:                ctx->lx = ctx->ly = -1;
                        x <<= s; y <<= s;
                        if(ctx->style & 0x200) {
                            a = (((4096<<4)-(oy)) >> (3));
                            b = (((4096<<4)-(oy + y)) >> (3));
                            if(ctx->ix > ox + a) ctx->ix = ox + a;
                        } else a = b = 0;
                        if(ctx->np) _ssfn_l(ctx, -1, -1, 0);
                        _ssfn_l(ctx, ox+a, oy, 0);
                        _ssfn_l(ctx, ox+x+a, oy, 0);
                        _ssfn_l(ctx, ox+x+b, oy+y, 0);
                        _ssfn_l(ctx, ox+b, oy+y, 0);
                        _ssfn_l(ctx, ox+a, oy, 0);
                    } else {
                        a = x << 3; b = y << s;
                        c = _ssfn_g2i(oy);
                        n = _ssfn_g2i(ox);
                        w = _ssfn_g2i(x << (3+s));
                        h = _ssfn_g2i(b);
                        if(c + h >= ctx->ret->h) c = ctx->ret->h - h;   /* due to rounding */
                        c = t = c * ctx->ret->pitch;
                        for(j=0; j < h; j++) {
                            o = (((j<<8) * (y<<8) / (h<<8)) >> 8) * x;
                            for(i=0; i < w; i++) {
                                m = ((i<<8) * (a<<8) / (w<<8)) >> 8;
                                if(raw[o + (m >> 3)] & (1 << (m & 7))) {
                                    d = n + ((ctx->style & 0x200) ? _ssfn_igi(oy + (j<<s) + 127) : 0) + i;
                                    switch(ctx->mode) {
                                        case SSFN_MODE_BITMAP:
                                            pix[c + (d >> 3)] |= 1 << (d & 7); d++;
                                            if((ctx->style & 0x100) && (d >> 3) < ctx->ret->pitch) {
                                                pix[c + (d >> 3)] |= 1 << (d & 7); d++;
                                                if(ctx->size > 127 && (d >> 3) < ctx->ret->pitch) {
                                                    pix[c + (d >> 3)] |= 1 << (d & 7);
                                                }
                                            }
                                        break;
                                        case SSFN_MODE_CMAP:
                                        case SSFN_MODE_ALPHA:
                                            pix[c + (d++)] = 0xFF;
                                            if((ctx->style & 0x100) && d < ctx->ret->pitch) {
                                                pix[c + (d++)] = 0xFF;
                                                if(ctx->size > 127 && d < ctx->ret->pitch) {
                                                    pix[c + d] = 0xFF;
                                                }
                                            }
                                        break;
                                    }
                                    if(d > ctx->ret->w) ctx->ret->w = d;
                                    d = ox + _ssfn_i2g(d);
                                    if((ctx->style & 0x200) && ctx->ix > d) ctx->ix = d;
                                    if(_ssfn_g2i(oy) + j == ctx->u) {
                                        if(d < ctx->uix) { ctx->uix = d; }
                                        if(d > ctx->uax) { ctx->uax = d; }
                                    }
                                }
                            }
                            c += ctx->ret->pitch;
                        }
                        if(ctx->mode != SSFN_MODE_BITMAP && h > y && w > 1) {
                            m = (ctx->mode == SSFN_MODE_CMAP) ? 0xF0 : 0;
                            x = h/y;
                            for(a = x; a; a--) {
                                b = ctx->style & 0x100 ? (64 + (128 * a / x)) : (192 * a / x);
                                if(ctx->mode == SSFN_MODE_CMAP) b = (b >> 4) | 0xF0;
                                c = t + ctx->ret->pitch;
                                for(j=1; j < h-1; j++) {
                                    for(i=1; i < w-1; i++) {
                                        d = n + ((ctx->style & 0x200) ? _ssfn_igi(oy + (j<<s)) : 0) + i;
                                        if(pix[c + d] == m &&
                                            (pix[c + d - ctx->ret->pitch] > b || pix[c + d + ctx->ret->pitch] > b) &&
                                            (pix[c + d - 1] > b || pix[c + d + 1] > b))
                                                pix[c + d] = b;
                                    }
                                    c += ctx->ret->pitch;
                                }
                            }
                        }
                    }
                    h = 0;
                break;

                case SSFN_FRAG_PIXMAP:
                    x = (((raw[0] & 12) << 6) | raw[1])+1;
                    y = (((raw[0] & 3) << 8) | raw[2])+1;
                    n = ((raw[4]<<8) | raw[3])+1;
                    raw += 5;
                    if(ctx->mode == SSFN_MODE_OUTLINE) goto outline;
                    if(raw[-5] & 0x10) {
                        /* todo: direct ARGB values in pixmap fragment */
                    }
                    a = x * y;
                    if(a >= (ctx->nr[0] << 1)) {
                        ctx->nr[0] = (a+1) >> 1;
                        ctx->r[0] = (uint16_t *)SSFN_realloc(ctx->r[0], (ctx->nr[0] << 1));
                        if(!ctx->r[0]) { ctx->err = SSFN_ERR_ALLOC; return; }
                    }
                    ctx->ret->cmap = (uint32_t*)((uint8_t*)ctx->f + ctx->f->size - 964);
                    for(re = raw+n, ra = (uint8_t*)ctx->r[0], i=0; i < a && raw < re;) {
                        c = (raw[0] & 0x7F)+1;
                        if(raw[0] & 0x80) { for(j=0; j < c; j++) ra[i++] = raw[1]; raw += 2; }
                        else { raw++; for(j=0; j < c; j++) ra[i++] = *raw++; }
                    }
                    b = y << s;
                    c = _ssfn_g2i(oy);
                    n = _ssfn_g2i(ox);
                    w = _ssfn_g2i(x << s);
                    h = _ssfn_g2i(b);
                    if(c + h >= ctx->ret->h) c = ctx->ret->h - h;   /* due to rounding */
                    c = t = c * ctx->ret->pitch;
                    for(j=0; j < h; j++) {
                        o = (((j<<8) * (y<<8) / (h<<8)) >> 8) * x;
                        for(i=0; i < w; i++) {
                            m = ((i<<8) * (x<<8) / (w<<8)) >> 8;
                            if(ra[o + m] < 0xF0) {
                                d = n + ((ctx->style & 0x200) ? _ssfn_igi(oy + (j<<s) + 127) : 0) + i;
                                re = (uint8_t*)&ctx->ret->cmap[ra[o + m]];
                                a = (re[0] + re[1] + re[2] + 255) >> 2;
                                switch(ctx->mode) {
                                    case SSFN_MODE_BITMAP:
                                        if(a > 127)
                                            pix[c + (d >> 3)] |= 1 << (d & 7);
                                    break;
                                    case SSFN_MODE_CMAP:
                                        pix[c + d] = ra[o + m];
                                    break;
                                    case SSFN_MODE_ALPHA:
                                        a >>= 1; a = ~a;
                                        pix[c + d] = a;
                                    break;
                                }
                                if(d > ctx->ret->w) ctx->ret->w = d;
                            }
                        }
                        c += ctx->ret->pitch;
                    }
                    h = 0;
                break;

                case SSFN_FRAG_HINTING:
                    if(raw[0] & 0x10) { n = ((raw[0]&0xF)<<8) | raw[1]; raw += 2; } else { n = raw[0] & 0xF; raw++; }
                    if(render || !ox) { raw += n << (ctx->f->features & SSFN_FEAT_HBIGCRD ? 1 : 0); continue; }
                    y = 4096; x = ((ox >> s) - 1) << (s-4);
                    ctx->h[y++] = x;
                    for(n++;n-- && x < 4096;) {
                        x = raw[0]; raw++;
                        if(ctx->f->features & SSFN_FEAT_HBIGCRD) { x |= (raw[0] << 8); raw++; }
                        x <<= (s-4);
                        ctx->h[y++] = x;
                    }
                    if(y < 4096) ctx->h[y++] = 65535;
                    h = 1;
                break;
            }
        } else {
            if(!render && h) break;
            if(raw[0] & 0x40) { n = ((raw[0] & 0x3F) << 8) | raw[1]; raw += 2; } else { n = raw[0] & 0x3F; raw++; }
            if(ctx->f->quality < 5) { x = raw[0]; y = raw[1]; raw += 2; }
            else { x = ((raw[0] & 3) << 8) | raw[1]; y = ((raw[0] & 0x30) << 4) | raw[2]; raw += 3; }
            x <<= s; y <<= s;  y += oy;
            x += ox + (ctx->style & 0x200 ? _ssfn_igg(y) : 0);
            if(render) {
                if(ctx->np) { _ssfn_l(ctx, ctx->mx, ctx->my, 0); _ssfn_l(ctx, -1, -1, 0); }
                _ssfn_l(ctx, x, y, 0);
            }
            ctx->lx = ctx->mx = x; ctx->ly = ctx->my = y;
            for(n++;n--;) {
                t = ctx->g < 8 ? (raw[0] >> 7) | ((raw[1] >> 6) & 2) : raw[0] & 3;
                x = y = a = b = c = d = j = 0;
                switch(ctx->g) {
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        x = raw[0] & 0x7F; y = raw[1] & 0x7F;
                        switch(t) {
                            case 0: raw += raw[0] & 4 ? 5 : 2; break;
                            case 1: raw += 2; break;
                            case 2: a = raw[2] & 0x7F; b = raw[3] & 0x7F; raw += 4; break;
                            case 3: a = raw[2] & 0x7F; b = raw[3] & 0x7F; c = raw[4] & 0x7F; d = raw[5] & 0x7F; raw += 6; break;
                        }
                    break;

                    case 8:
                        x = raw[1]; y = raw[2];
                        switch(t) {
                            case 0: raw += raw[0] & 4 ? 5 : 2; break;
                            case 1: raw += 3; break;
                            case 2: a = raw[3]; b = raw[4]; raw += 5; break;
                            case 3: a = raw[3]; b = raw[4]; c = raw[5]; d = raw[6]; raw += 7; break;
                        }
                    break;

                    case 9:
                        x = ((raw[0] & 4) << 6) | raw[1]; y = ((raw[0] & 8) << 5) | raw[2];
                        switch(t) {
                            case 0: raw += raw[0] & 4 ? 5 : 2; break;
                            case 1: raw += 3; break;
                            case 2: a = ((raw[0] & 16) << 4) | raw[3]; b = ((raw[0] & 32) << 3) | raw[4]; raw += 5; break;
                            case 3: a = ((raw[0] & 16) << 4) | raw[3]; b = ((raw[0] & 32) << 3) | raw[4];
                                c = ((raw[0] & 64) << 2) | raw[5]; d = ((raw[0] & 128) << 1) | raw[6]; raw += 7; break;
                        }
                    break;

                    default:
                        x = ((raw[0] & 12) << 6) | raw[1]; y = ((raw[0] & 48) << 4) | raw[2];
                        switch(t) {
                            case 0: raw += raw[0] & 4 ? 5 : 2; break;
                            case 1: raw += 3; break;
                            case 2: a = ((raw[3] & 3) << 8) | raw[4]; b = ((raw[3] & 12) << 6) | raw[5]; raw += 6; break;
                            case 3: a = ((raw[3] & 3) << 8) | raw[4]; b = ((raw[3] & 12) << 6) | raw[5];
                                c = ((raw[3] & 48) << 4) | raw[6]; d = ((raw[3] & 192) << 2) | raw[7]; raw += 8; break;
                        }
                    break;
                }
                x <<= s; y <<= s; a <<= s; b <<= s; c <<= s; d <<= s;
                x += ox; y += oy; a += ox; b += oy; c += ox; d += oy;
                if(ctx->style & 0x200) {
                    x += _ssfn_igg(y);
                    a += _ssfn_igg(b);
                    c += _ssfn_igg(d);
                }
                if(render) {
                    switch(t) {
                        case 0: /* this v1.0 renderer does not support colored contours */ break;
                        case 1: _ssfn_l(ctx, x, y, 0); break;
                        case 2: _ssfn_b(ctx, ctx->lx,ctx->ly, ((a-ctx->lx)>>1)+ctx->lx,((b-ctx->ly)>>1)+ctx->ly,
                                ((x-a)>>1)+a,((y-b)>>1)+b, x,y, 0); break;
                        case 3: _ssfn_b(ctx, ctx->lx,ctx->ly, a,b, c,d, x,y, 0); break;
                    }
                } else if(t == 1 && x >= 0 && y >= 0) {
                    a = ((ctx->lx < x) ? x - ctx->lx : ctx->lx - x) >> 4;
                    b = ((ctx->ly < y) ? y - ctx->ly : ctx->ly - y) >> 4;
                    c = (ctx->lx + x) >> 5;
                    if(a < 2) ctx->h[4096 + (!ctx->h[4096 + c] && c && ctx->h[4096 + c - 1]? c - 1 : c)] += b;
                }
                ctx->lx = x; ctx->ly = y;
            }
        }
    }

    if(!render && !h) {
        for(j = m = x = y = 0; j < 4096; j++) {
            if(ctx->h[4096 + j] >= 4096/SSFN_HINTING_THRESHOLD) {
                if(!j) m++;
                else { ctx->h[4096 + m++] = j - x; x = j; }
            }
        }
        if(m < 4096) ctx->h[4096 + m] = 65535;
    }
}

/*** public API implementation ***/

/**
 * Decode a color map pixel into ARGB
 *
 * @param p uint8_t color map pixel
 * @param c uint32_t* pointer to color map
 * @param fg uint32_t foreground color
 * @return uint32_t ARGB pixel
 */
#define SSFN_CMAP_TO_ARGB(p,c,fg) (p>=0xF0?(uint32_t)((p<<28)|((p&0xF)<<24)|fg):c[p])

/**
 * Load a font or font collection into renderer context
 *
 * @param ctx rendering context
 * @param font SSFN font or font collection in memory
 * @return error code
 */
int ssfn_load(ssfn_t *ctx, const ssfn_font_t *font)
{
    ssfn_font_t *ptr, *end;

    if(!ctx || !font) {
        if(ctx) ctx->err = SSFN_ERR_INVINP;
        return SSFN_ERR_INVINP;
    }
    ctx->err = SSFN_OK;
    if(!SSFN_memcmp(font->magic, SSFN_COLLECTION, 4)) {
        end = (ssfn_font_t*)((uint8_t*)font + font->size);
        for(ptr = (ssfn_font_t*)((uint8_t*)font + 8); ptr < end && !ctx->err; ptr = (ssfn_font_t*)((uint8_t*)ptr + ptr->size))
            ssfn_load(ctx, ptr);
    } else {
        if(SSFN_memcmp(font->magic, SSFN_MAGIC, 4) || SSFN_memcmp((uint8_t*)font + font->size - 4, SSFN_ENDMAGIC, 4) ||
            font->family > SSFN_FAMILY_HAND || font->fragments_offs > font->size || font->characters_offs[0] > font->size ||
            font->kerning_offs > font->size || font->fragments_offs >= font->characters_offs[0] || font->quality > 8) {
                ctx->err = SSFN_ERR_BADFILE;
        } else {
            ctx->len[font->family]++;
            ctx->fnt[font->family] = (const ssfn_font_t**)SSFN_realloc(ctx->fnt[font->family], ctx->len[font->family]*sizeof(void*));
            if(!ctx->fnt[font->family])
                ctx->err = SSFN_ERR_ALLOC;
            else
                ctx->fnt[font->family][ctx->len[font->family]-1] = font;
        }
    }
    return ctx->err;
}

/**
 * Set up rendering parameters
 *
 * @param ctx rendering context
 * @param family one of SSFN_FAMILY_*
 * @param name NULL or UTF-8 string if family is SSFN_FAMILY_BYNAME
 * @param style OR'd values of SSFN_STYLE_*
 * @param size how big glyph it should render, 8 - 255
 * @param mode rendering mode, one of SSFN_MODE_*
 * @return error code
 */
int ssfn_select(ssfn_t *ctx, int family, char *name, int style, int size, int mode)
{
    int i, j, l;

    if(!ctx) return SSFN_ERR_INVINP;
    if((style & ~0xCF)) return (ctx->err = SSFN_ERR_BADSTYLE);
    if(size < 8 || size > 255) return (ctx->err = SSFN_ERR_BADSIZE);
    if(mode > SSFN_MODE_CMAP) return (ctx->err = SSFN_ERR_BADMODE);

    if(family == SSFN_FAMILY_BYNAME) {
        if(!name || !name[0]) {
invinp:     return (ctx->err = SSFN_ERR_INVINP);
        }
        for(l=0; name[l]; l++);
        for(i=0; i < 5; i++) {
            for(j=0; j < ctx->len[i]; j++) {
                if(!SSFN_memcmp(name, (uint8_t*)&ctx->fnt[i][j]->magic + sizeof(ssfn_font_t), l)) {
                        ctx->s = ctx->fnt[i][j];
                        goto familyfound;
                }
            }
        }
noface: return (ctx->err = SSFN_ERR_NOFACE);
    } else {
        if(family != SSFN_FAMILY_ANY) {
            if(family > SSFN_FAMILY_HAND) goto invinp;
            if(!ctx->len[family]) goto noface;
        }
        ctx->s = NULL;
    }
familyfound:
    ctx->np = ctx->mp = 0;
    if(ctx->p) {
        SSFN_free(ctx->p);
        ctx->p = NULL;
    }
    ctx->f = NULL;
    ctx->family = family;
    ctx->style = style;
    ctx->size = size;
    ctx->mode = mode;
    ctx->variant = 0;
    return (ctx->err = SSFN_OK);
}

/**
 * Set glyph variant
 *
 * @param ctx rendering context
 * @param variant one of SSFN_VARIANT_* defines
 * @return error code
 */
int ssfn_variant(ssfn_t *ctx, int variant)
{
    if(!ctx) return SSFN_ERR_INVINP;
    if(variant < 0 || variant > SSFN_NUMVARIANTS-1) return (ctx->err = SSFN_ERR_INVINP);
    ctx->variant = variant;
    return (ctx->err = SSFN_OK);
}

/**
 * Glyph renderer
 *
 * @param ctx rendering context
 * @param unicode character to render
 * @return newly allocated rasterized glyph
 */
ssfn_glyph_t *ssfn_render(ssfn_t *ctx, uint32_t unicode)
{
    ssfn_font_t **fl;
    int i, j, s, h, p, m, n, bt, bl;
    int l, k, x, y;
    uint8_t *rg = NULL, c, d;

    if(!ctx) return NULL;
    if(ctx->size < 8) { ctx->err = SSFN_ERR_NOFACE; return NULL; }
    ctx->err = SSFN_OK;
    if(ctx->s) {
        ctx->f = (ssfn_font_t*)ctx->s;
        rg = _ssfn_c(ctx->f, unicode, ctx->variant);
        if(!rg) rg = _ssfn_c(ctx->f, unicode, 0);
    } else {
        p = ctx->family;
        k = ctx->variant;
again:  if(p == SSFN_FAMILY_ANY) { n = 0; m = 4; } else n = m = p;
        for(; n <= m; n++) {
            fl = (ssfn_font_t **)ctx->fnt[n];
            if(ctx->style & 3) {
                /* check if we have a specific ctx->f for the requested style */
                for(i=0;i<ctx->len[n];i++)
                    if((fl[i]->style & 3) == (ctx->style & 3) && (rg = _ssfn_c(fl[i], unicode, k))) { ctx->f = fl[i]; break; }
                /* if bold italic was requested, check if we have at least bold or italic */
                if(!rg && (ctx->style & 3) == 3)
                    for(i=0;i<ctx->len[n];i++)
                        if((fl[i]->style & 3) && (rg = _ssfn_c(fl[i], unicode, k))) { ctx->f = fl[i]; break; }
            }
            /* last resort, get the first ctx->f which has a glyph for this unicode, no matter style */
            if(!rg) {
                for(i=0;i<ctx->len[n];i++)
                    if((rg = _ssfn_c(fl[i], unicode, k))) { ctx->f = fl[i]; break; }
            }
        }
        /* if glyph still not found, try any family group, finally only default variant */
        if(!rg) {
            if(p != SSFN_FAMILY_ANY) { p = SSFN_FAMILY_ANY; goto again; }
            if(k) { k = 0; goto again; }
        }
    }
    if(!rg) { ctx->err = ctx->variant ? SSFN_ERR_NOVARIANT : SSFN_ERR_NOGLYPH; return NULL; }

    ctx->style &= 0xFF;
    if((ctx->style & 1) && !(ctx->f->style & 1)) ctx->style |= 0x100;
    if((ctx->style & 2) && !(ctx->f->style & 2)) ctx->style |= 0x200;
    if(ctx->f->family == SSFN_FAMILY_MONOSPACE) ctx->style |= SSFN_STYLE_ABS_SIZE;

    ctx->g = 4 + ctx->f->quality;
    ctx->np = 0;

    s = 16 - ctx->g;
    if(ctx->mode == SSFN_MODE_OUTLINE) {
        h = ctx->size;
        p = 0;
    } else {
        if(!(((rg[2] & 0x0F) << 8) | rg[6]) || ctx->style & SSFN_STYLE_ABS_SIZE)
            h = ctx->size;
        else
            h = (4096 << 4) * ctx->size / ((ctx->f->baseline - ctx->f->bbox_top) << s);
        p = (h + (ctx->style & 0x100? 2 : 0) + (ctx->style & 0x200 ? h>>2 : 0));
        if(p > 255) {
            p = h = 255;
            if(ctx->style & 0x100) h -= 2;
            if(ctx->style & 0x200) h = h*4/5;
        }
        if(ctx->mode == SSFN_MODE_BITMAP) p = (p + 7) >> 3;
    }
    ctx->m = h;

    if(!ctx->h) ctx->h = (uint16_t*)SSFN_realloc(NULL, 4096*2*sizeof(uint16_t));
    if(!ctx->h) goto erralloc;

    if(!(ctx->style & SSFN_STYLE_NOHINTING)) {
        SSFN_memset(&ctx->h[4096], 0, 4096*sizeof(uint16_t));
        _ssfn_g(ctx, rg, 0);
    } else ctx->h[4096] = 65535;

    ctx->h[0] = 0;
    for(i = j = k = x = y = m = 0; i < 4096 && j < 4095; i++) {
        y = ctx->h[4096 + i] == 65535 ? 4095 - j : ctx->h[4096 + i];
        j += y;
        if(j == x) {
            ctx->h[j] = (((j<<4) * h + (1<<15)) >> 16) << 8;
            if(!y) j++;
        } else {
            y = _ssfn_g2o(y<<4);
            m += y;
            if((i&1) || y < 256) { continue; }
            m &= ~0xFF;
            n = ctx->h[x];
            for(l = 0; l + x <= j; l++)
                ctx->h[x + l] = n + ((m-n)*l / (j-x));
        }
        x = j;
    }
    ctx->uix = _ssfn_g2ixc((((rg[2] & 0x0F) << 8) | rg[6]) << s);
    ctx->uax = _ssfn_g2iyc((((rg[2] & 0xF0) << 4) | rg[7]) << s);
    if(ctx->mode == SSFN_MODE_NONE) return NULL;

    bl = ((((rg[3] & 0x0F) << 8) | rg[8])) << s;
    bt = ((((rg[3] & 0xF0) << 4) | rg[9])) << s;
    i = p * h;
    ctx->ret = (ssfn_glyph_t*)SSFN_realloc(NULL, i + 8 + sizeof(uint8_t*));
    if(!ctx->ret) {
erralloc:
        ctx->err = SSFN_ERR_ALLOC;
        return NULL;
    }
    SSFN_memset(&ctx->ret->data, ctx->mode == SSFN_MODE_CMAP ? 0xF0 : 0, i);
    ctx->ret->cmap = NULL;
    ctx->ret->mode = ctx->mode;
    ctx->ret->pitch = p;
    ctx->ret->w = 0;
    ctx->ret->h = h;
    ctx->ret->baseline = (((ctx->f->baseline << s) - bt) * h + (1<<16) - 1) >> 16;
    ctx->u = ctx->ret->baseline + ((((ctx->f->underline - ctx->f->baseline) << s) * h) >> 16);

    ctx->ret->adv_x = ctx->uix;
    ctx->ret->adv_y = ctx->uax;

    ctx->ix = ctx->uix = 4096 << 4; ctx->uax = 0;
    _ssfn_g(ctx, rg, 1);

    if(!ctx->err) {
        if(!(ctx->style & 0x200) || ctx->ix == 4096 << 4) ctx->ix = 0;
        if(ctx->mode == SSFN_MODE_OUTLINE) {
            if(ctx->np > p * h) {
                ctx->ret = (ssfn_glyph_t*)SSFN_realloc(ctx->ret, ctx->np + 8 + sizeof(uint8_t*));
                if(!ctx->ret) goto erralloc;
            }
            for(s=i=0; i < ctx->np; i += 2) {
                c = ctx->p[i+0] == 0xffff ? 0xff : _ssfn_g2ix(ctx->p[i+0] + bl);
                d = ctx->p[i+1] == 0xffff ? 0xff : _ssfn_g2iy(ctx->p[i+1] + bt);
                if(s<2 || ctx->ret->data[s-2] != c || ctx->ret->data[s-1] != d) {
                    ctx->ret->data[s++] = c;
                    ctx->ret->data[s++] = d;
                }
            }
            ctx->ret->pitch = s;
            ctx->ret = (ssfn_glyph_t*)SSFN_realloc(ctx->ret, s + 8 + sizeof(uint8_t*));
            if(!ctx->ret) goto erralloc;
        } else {
            _ssfn_r(ctx);
            if(ctx->mode != SSFN_MODE_BITMAP) _ssfn_a(ctx);
            if(ctx->style & SSFN_STYLE_STHROUGH) {
                if(ctx->ret->w < ctx->ret->adv_x) ctx->ret->w = ctx->ret->adv_x;
                SSFN_memset(&ctx->ret->data[(ctx->ret->baseline - (ctx->size >> 2)) * p], 0xFF, (ctx->size/64 + 2) * p);
            }
            if(ctx->style & SSFN_STYLE_UNDERLINE) {
                if(ctx->ret->w < ctx->ret->adv_x) ctx->ret->w = ctx->ret->adv_x;
                if(ctx->uax > ctx->ix) ctx->uax = _ssfn_g2i(ctx->uax - ctx->ix);
                if(ctx->uix != 4096<<4) ctx->uix = _ssfn_g2i(ctx->uix - ctx->ix);
                else ctx->uix = ctx->ret->w + 3;
                m = ctx->u * p;
                n = ctx->size > 127 ? 2 : 1;
                while(n--) {
                    if(ctx->uix > 3) {
                        j = ctx->uix - 3;
                        if(ctx->mode == SSFN_MODE_BITMAP)
                            for(i=0; i < j; i++) ctx->ret->data[m + (i >> 3)] |= 1 << (i & 7);
                        else
                            SSFN_memset(&ctx->ret->data[m], 0xFF, j);
                    }
                    if(ctx->uax) {
                        j = ctx->uax + 2;
                        if(ctx->mode == SSFN_MODE_BITMAP)
                            for(i=j; i < ctx->ret->w; i++) ctx->ret->data[m + (i >> 3)] |= 1 << (i & 7);
                        else
                            SSFN_memset(&ctx->ret->data[m + j], 0xFF, p - j);
                    }
                    m += p;
                }
            }
        }
        if(ctx->ret->adv_y)
            ctx->ret->baseline = ctx->ret->w >> 1;
    } else if(ctx->ret) {
        SSFN_free(ctx->ret);
        ctx->ret = NULL;
    }
    return ctx->ret;
}

/**
 * Return kerning information
 *
 * @param ctx rendering context
 * @param unicode current unicode character
 * @param nextunicode next unicode character
 * @param *x pointer to an integer
 * @param *y pointer to an integer
 * @return error code, and relative offsets adjusted to *x, *y
 */
int ssfn_kern(ssfn_t *ctx, uint32_t unicode, uint32_t nextunicode, int *x, int *y)
{
    const ssfn_font_t *font;
    uint32_t i, j, k, l, a, b, c;
    uint8_t *ptr;
    int m;

    if(!ctx || !x || !y) return SSFN_ERR_INVINP;
    if(!ctx->f && !ctx->s) return (ctx->err = SSFN_ERR_NOFACE);
    font = ctx->s ? ctx->s : ctx->f;
    if(unicode && nextunicode && font->kerning_offs) {
        ptr = (uint8_t*)font + font->kerning_offs;
        a = font->features & SSFN_FEAT_KBIGLKP ? 4 : 3;
        b = font->features & SSFN_FEAT_KBIGCHR;
        c = font->features & SSFN_FEAT_KBIGCRD;
        for(i=0;i<0x110000;i++) {
            if(ptr[0] & 0x80) {
                if(ptr[0] & 0x40) { i += ptr[1] | ((ptr[0] & 0x3f) << 8); ptr += 2; }
                else { i += ptr[0] & 0x3f; ptr++; }
            } else {
                m = ptr[0] & 0x7F;
                if(unicode >= i && unicode <= i+m) {
                    ptr = (uint8_t*)font + font->kerning_offs + ((a==4?(ptr[3]<<16):0)|(ptr[2]<<8)|ptr[1]);
                    if(ptr[0] & 0x80) { k = ptr[1] | ((ptr[0] & 0x7f) << 8); ptr += 2; }
                    else { k = ptr[0] & 0x7F; ptr++; }
                    for(m=0,a=SSFN_ERR_NOGLYPH,i=j=0;i<=k && j<=nextunicode;i++) {
                        if(b) { j = ptr[0] | (ptr[1] << 8) | ((ptr[2] & 0x7F) << 16); l = ptr[2] & 0x80; ptr += 3; }
                        else { j = ptr[0] | ((ptr[1] & 0x7F) << 8); l = ptr[1] & 0x80; ptr += 2; }
                        if(c) { m = (short)(ptr[0] | (ptr[1] << 8)); ptr += 2; } else { m = (signed char)ptr[0]; ptr++; }
                        if(j == nextunicode) {
                            a = SSFN_OK; m = (((m) << (16 - ctx->g)) * ctx->m + (1 << 16) - 1) >> 16;
                            if(l) *y += m; else *x += m;
                        }
                    }
                    return (ctx->err = a);
                }
                ptr += a;
                i += m;
            }
        }
    }
    return (ctx->err = SSFN_ERR_NOGLYPH);
}

/**
 * Returns the bounding box of the rendered text
 *
 * @param ctx rendering context
 * @param *str string
 * @param usekern use kerning when calculating size
 * @param *w pointer to an integer, returned width
 * @param *h pointer to an integer, returned height
 * @return error code, and bounding box size in *w, *h
 */
int ssfn_bbox(ssfn_t *ctx, char *str, int usekern, int *w, int *h)
{
    char *s;
    int u, v, m;

    if(!ctx) return SSFN_ERR_INVINP;
    if(!str || !w || !h) return (ctx->err = SSFN_ERR_INVINP);
    *w = *h = 0;
    m = ctx->mode; ctx->mode = SSFN_MODE_NONE; ctx->m = 0;
    for(s = str, u = ssfn_utf8(&s); u;) {
        ssfn_render(ctx, u);
        if(ctx->err == SSFN_OK) {
            *w += ctx->uix;
            *h += ctx->uax;
        }
        v = ssfn_utf8(&s);
        if(usekern) ssfn_kern(ctx, u, v, w, h);
        u = v;
    }
    if(!*w) *w = ctx->m;
    if(!*h) *h = ctx->m;
    ctx->mode = m;
    return ctx->err;
}

/**
 * Returns how much memory a context consumes
 *
 * @param ctx rendering context
 * @return total memory used by that context
 */
int ssfn_mem(ssfn_t *ctx)
{
    int i, ret = sizeof(ssfn_t);

    if(!ctx) return 0;

    for(i=0; i<5; i++) ret += ctx->len[i] * sizeof(ssfn_font_t*);
    if(ctx->p) ret += ctx->mp * sizeof(uint16_t);
    for(i=0; i<256; i++)
        if(ctx->r[i]) ret += ctx->nr[i] * sizeof(uint16_t);
    if(ctx->h) ret += 8192 * sizeof(uint16_t);
    return ret;
}

/**
 * Free renderer context
 *
 * @param ctx rendering context
 */
void ssfn_free(ssfn_t *ctx)
{
    int i;

    if(!ctx) return;

    for(i=0;i<5;i++)
        if(ctx->fnt[i]) SSFN_free(ctx->fnt[i]);
    if(ctx->p) SSFN_free(ctx->p);
    for(i=0; i<256; i++)
        if(ctx->r[i]) SSFN_free(ctx->r[i]);
    if(ctx->h) SSFN_free(ctx->h);
    SSFN_memset(ctx, 0, sizeof(ssfn_t));
}

#endif

/*** this function goes for both renderers ***/
#if !defined(SSFN_NOIMPLEMENTATION) || defined(SSFN_CONSOLEBITMAP_PALETTE) || \
    defined(SSFN_CONSOLEBITMAP_HICOLOR) || defined(SSFN_CONSOLEBITMAP_TRUECOLOR)
/**
 * Decode an UTF-8 multibyte, advance string pointer and return UNICODE. Watch out, no input checks
 *
 * @param **s pointer to an UTF-8 string pointer
 * @return unicode, and *s moved to next multibyte sequence
 */
uint32_t ssfn_utf8(char **s)
{
    uint32_t c = **s;

    if((**s & 128) != 0) {
        if(!(**s & 32)) { c = ((**s & 0x1F)<<6)|(*(*s+1) & 0x3F); *s += 1; } else
        if(!(**s & 16)) { c = ((**s & 0xF)<<12)|((*(*s+1) & 0x3F)<<6)|(*(*s+2) & 0x3F); *s += 2; } else
        if(!(**s & 8)) { c = ((**s & 0x7)<<18)|((*(*s+1) & 0x3F)<<12)|((*(*s+2) & 0x3F)<<6)|(*(*s+3) & 0x3F); *s += 3; }
        else c = 0;
    }
    *s += 1;
    return c;
}
#endif

#if defined(SSFN_CONSOLEBITMAP_PALETTE) || defined(SSFN_CONSOLEBITMAP_HICOLOR) || defined(SSFN_CONSOLEBITMAP_TRUECOLOR)
/*** special console bitmap font renderer (ca. 1k, no dependencies, no memory allocation and no error checking) ***/

/**
 * public variables to configure the renderer
 */
ssfn_font_t *ssfn_font;         /* font buffer */
uint8_t *ssfn_dst_ptr;          /* screen buffer */
uint32_t ssfn_dst_pitch;        /* screen dimensions */
uint32_t ssfn_dst_w = 0;
uint32_t ssfn_dst_h = 0;
uint32_t ssfn_fg;               /* colors in screen's native format */
#ifdef SSFN_CONSOLEBITMAP_CLEARBG
uint32_t ssfn_bg;
#endif
uint32_t ssfn_x;                /* coordinate to draw to */
uint32_t ssfn_y;
uint32_t ssfn_adv_x;            /* advance values */
uint32_t ssfn_adv_y;

/**
 * Minimal OS kernel console renderer
 *
 * @param unicode character to render
 * @return error code
 */
int ssfn_putc(uint32_t unicode)
{
    uint32_t i, j, a, b, m, p, t;
    uint8_t *rg, *o, *g, *s, *r, y, n;
#ifdef SSFN_CONSOLEBITMAP_CLEARBG
    uint8_t w;
#endif

    t = (ssfn_font->quality < 5 && ssfn_font->characters_offs[0] < 65536) ? 4 : (ssfn_font->characters_offs[0] < 1048576 ? 5 : 6);
    rg = (uint8_t*)ssfn_font + ssfn_font->characters_offs[0];
    ssfn_adv_x = ssfn_adv_y = 0;

    for(j=i=0;i<0x110000;i++) {
        if(rg[0] & 0x80) {
            n = (rg[0] & 0x3f);
            if(rg[0] & 0x40) { i += rg[1] | (n << 8); rg++; } else { i += n; }
            rg++;
        } else {
            if(i == unicode) {
                y = rg[9];
#ifdef SSFN_CONSOLEBITMAP_CLEARBG
                w = rg[4]; j = 0;
                if(ssfn_dst_w && (ssfn_x + w >= ssfn_dst_w)) w = ssfn_dst_w - ssfn_x;
                s = ssfn_dst_ptr + (ssfn_y)*ssfn_dst_pitch;
# ifdef SSFN_CONSOLEBITMAP_PALETTE
                s += ssfn_x;
# else
#  ifdef SSFN_CONSOLEBITMAP_HICOLOR
                s += ssfn_x << 1;
#  else
                s += ssfn_x << 2;
#  endif
# endif
#endif
                if((ssfn_dst_w && (ssfn_x > ssfn_dst_w)) || (ssfn_dst_h && (ssfn_y + ssfn_font->bbox_bottom >= ssfn_dst_h)))
                    return SSFN_ERR_INVINP;
                n = rg[0]; ssfn_adv_x = rg[6]; ssfn_adv_y = rg[7]; rg += 10;
                while(n--) {
#ifndef SSFN_CONSOLEBITMAP_CLEARBG
                    s = ssfn_dst_ptr + (ssfn_y + y + rg[t-1])*ssfn_dst_pitch;
# ifdef SSFN_CONSOLEBITMAP_PALETTE
                    s += ssfn_x;
# else
#  ifdef SSFN_CONSOLEBITMAP_HICOLOR
                    s += ssfn_x << 1;
#  else
                    s += ssfn_x << 2;
#  endif
# endif
#endif
                    o = (uint8_t*)ssfn_font; a = ((rg[1] << 8) | rg[0]);
                    switch(t) {
                        case 4: o += a; break;
                        case 5: o += ((rg[2] & 0xF) << 16) | a; break;
                        default: o += ((rg[3] & 0xF) << 24) | (rg[2] << 16) | a; break;
                    }
                    p = ((o[0] & 0xF)+1); a = p << 3; b = o[1]+1; o += 2;
                    if(ssfn_dst_w && (ssfn_x + a >= ssfn_dst_w)) a = ssfn_dst_w - ssfn_x;
#ifdef SSFN_CONSOLEBITMAP_CLEARBG
                    for(;j<y+rg[t-1];j++,s+=ssfn_dst_pitch)
                        for(r=s,i=0;i<w;i++) {
# ifdef SSFN_CONSOLEBITMAP_PALETTE
                                *((uint8_t*)r) = (uint8_t)ssfn_bg;
                                r++;
# else
#  ifdef SSFN_CONSOLEBITMAP_HICOLOR
                                *((uint16_t*)r) = (uint16_t)ssfn_bg;
                                r += 2;
#  else
                                *((uint32_t*)r) = (uint32_t)ssfn_bg;
                                r += 4;
#  endif
# endif
                        }
                    b += j;
#else
                    j = 0;
#endif
                    for(;j<b;j++,s+=ssfn_dst_pitch,o+=p) {
                        for(g=o,r=s,m=1,i=0;i<a;i++,m<<=1) {
                            if(m > 0x80) { g++; m = 1; }
#ifndef SSFN_CONSOLEBITMAP_CLEARBG
                            if(*g & m)
#endif
#ifdef SSFN_CONSOLEBITMAP_PALETTE
                                *((uint8_t*)r) = (uint8_t)(
# ifdef SSFN_CONSOLEBITMAP_CLEARBG
                                *g & m ? ssfn_fg : ssfn_bg
# else
                                ssfn_bg
# endif
                                );
                            r++;
#else
# ifdef SSFN_CONSOLEBITMAP_HICOLOR
                                *((uint16_t*)r) = (uint16_t)(
#  ifdef SSFN_CONSOLEBITMAP_CLEARBG
                                *g & m ? ssfn_fg : ssfn_bg
#  else
                                ssfn_fg
#  endif
                                );
                            r += 2;
# else
                                *((uint32_t*)r) = (uint32_t)(
#  ifdef SSFN_CONSOLEBITMAP_CLEARBG
                                *g & m ? ssfn_fg : ssfn_bg
#  else
                                ssfn_fg
#  endif
                                );
                            r += 4;
# endif
#endif
                        }
                    }
                    rg += t;
                }
#ifdef SSFN_CONSOLEBITMAP_CLEARBG
                for(;j<=ssfn_font->bbox_bottom;j++,s+=ssfn_dst_pitch)
                    for(r=s,i=0;i<w;i++) {
# ifdef SSFN_CONSOLEBITMAP_PALETTE
                            *((uint8_t*)r) = (uint8_t)ssfn_bg;
                            r++;
# else
#  ifdef SSFN_CONSOLEBITMAP_HICOLOR
                            *((uint16_t*)r) = (uint16_t)ssfn_bg;
                            r += 2;
#  else
                            *((uint32_t*)r) = (uint32_t)ssfn_bg;
                            r += 4;
#  endif
# endif
                    }
#endif
                ssfn_x += ssfn_adv_x; ssfn_y += ssfn_adv_y;
                return SSFN_OK;
            }
            rg += rg[0] * t + 10;
            j++;
        }
    }
    return SSFN_ERR_NOGLYPH;
}
#endif

#ifdef  __cplusplus
}
#endif

#endif
