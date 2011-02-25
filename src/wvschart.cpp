/******************************************************************************
 * $Id: wvschart.cpp,v 1.13 2010/04/27 01:44:02 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  World Vector Shoreline (WVS) Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 *
 *
 */
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "dychart.h"

#include "wvschart.h"
#include "cutil.h"
#include "georef.h"
#include "chart1.h"

CPL_CVSID("$Id: wvschart.cpp,v 1.13 2010/04/27 01:44:02 bdbcat Exp $");

//      Local Prototypes
extern "C" int wvsrtv (const wxString& sfile, int latd, int lond, float **latray, float **lonray, int **segray);
//------------------------------------------------------------------------------
//      WVSChart Implementation
//------------------------------------------------------------------------------
WVSChart::WVSChart(wxWindow *parent, const wxString& wvs_chart_home)
{
        pwvs_home_dir = new wxString(wvs_chart_home);
        pwvs_file_name = new wxString(*pwvs_home_dir);
        pwvs_file_name->Append(_T("wvs43.dat"));
        for(int i=0 ; i < 360 ; i++)
        {
                for(int j=0 ; j < 180; j++)
                {
                         plat_ray[i][j] = NULL;
                         plon_ray[i][j] = NULL;
                         pseg_ray[i][j] = NULL;
                         nseg[i][j] = -1;
                }
        }
//      Allocate initial drawing point buffer
        cur_seg_cnt_max = 4;
        ptp = (wxPoint *)malloc(cur_seg_cnt_max * sizeof(wxPoint));
//      Attempt file open to validate the passed parameter
        FILE *fp = fopen (pwvs_file_name->mb_str(), "rb");
        if(NULL == fp)
        {
                m_ok = false;
                wxString msg(_T("   Unable to open WVSChart datafile: "));
                msg.Append(*pwvs_file_name);
                wxLogMessage(msg);
                return;
        }
        fclose(fp);
        m_ok = true;

        wxString s(_T("Using WVSChart datafile: "));
        s.Append(*pwvs_file_name);
        wxLogMessage(s);
}
WVSChart::~WVSChart()
{
        delete pwvs_home_dir;
        delete pwvs_file_name;
        for(int i=0 ; i < 360 ; i++)
        {
                for(int j=0 ; j < 180; j++)
                {
                         if(plat_ray[i][j])
                                 free(plat_ray[i][j]);
                         if(plon_ray[i][j])
                                 free(plon_ray[i][j]);
                         if(pseg_ray[i][j])
                                 free(pseg_ray[i][j]);
                         nseg[i][j] = -1;
                }
        }
        free(ptp);
}
void WVSChart::RenderViewOnDC(wxMemoryDC& dc, ViewPort& VPoint)
{
        float *platray = NULL;
        float *plonray = NULL;
        int *psegray = NULL;
        int x,y;
        if(!m_ok)
                return;

//      Set Color
        wxPen *pthispen = wxThePenList->FindOrCreatePen(GetGlobalColor(_T("BLUE3")), 1, wxSOLID);
        dc.SetPen(*pthispen);

//      Compute the 1 degree cell boundaries

        int lat_min = (int)floor(VPoint.GetBBox().GetMinY());
        int lat_max = (int)ceil(VPoint.GetBBox().GetMaxY());
        int lon_min = (int)floor(VPoint.GetBBox().GetMinX());
        int lon_max = (int)ceil(VPoint.GetBBox().GetMaxX());

        x = lon_min;
        y = lat_min;

//        printf("%d %d\n", lon_min, lon_max);

        //  Make positive definite longitude for easier integer math
        lon_min += 720;
        lon_max += 720;

        double ref_lon = VPoint.clon;

//      Loop around the lat/lon spec to get and draw the vector segments
        for(y = lat_min ; y < lat_max ; y++)
        {
                for(x = lon_min ; x < lon_max ; x++)
                {
//      Get the arrays of lat/lon vector segments
//      Sanity Check
                      int xt = x;
                      int yt = y;
//      Check the cache first
                        int ix = xt % 360; //xt + 180;                               // bias to positive
                        int iy = yt + 90;

                        if(  (ix > 359) || (ix < 0) || (iy > 179) || (iy < 0) )
                                continue;


                        if(-1 == nseg[ix][iy])                          // no data yet
                        {
                                                                                                     // so fill cache
                                platray = NULL;
                                plonray = NULL;
                                psegray = NULL;
                                int nsegments = wvsrtv (*pwvs_file_name,
                                        y, ix, &platray, &plonray, &psegray);
                                plat_ray[ix][iy] = platray;
                                plon_ray[ix][iy] = plonray;
                                pseg_ray[ix][iy] = psegray;
                                nseg[ix][iy] = nsegments;
//                                printf("load at %d %d \n", ix, iy);

                        }
 //                       else
 //                             printf("     from cache at %d %d \n", ix, iy);

                        if(nseg[ix][iy])
                        {
                                float *plat_seg = plat_ray[ix][iy];
                                float *plon_seg = plon_ray[ix][iy];
                                int *pseg_cnt = pseg_ray[ix][iy];
                                for(int iseg = 0 ; iseg < nseg[ix][iy] ; iseg++)
                                {
                                        int seg_cnt = *pseg_cnt++;
                                        if(seg_cnt > cur_seg_cnt_max)
                                        {
                                                cur_seg_cnt_max = seg_cnt;
                                                ptp = (wxPoint *)realloc(ptp, seg_cnt * sizeof(wxPoint));
                                        }
                                        wxPoint *pr = ptp;
                                        wxPoint p;



                                        for(int ip = 0 ; ip < seg_cnt ; ip++)
                                        {
                                                float plat = *plat_seg++;
                                                float plon = *plon_seg++;

                                                if(fabs(plon - ref_lon) > 180.)
                                                {
                                                      if(plon > ref_lon)
                                                            plon -= 360.;
                                                      else
                                                            plon += 360.;
                                                }


                                                double easting, northing;
                                                toSM(plat, plon + 360., VPoint.clat, ref_lon + 360., &easting, &northing);
                                                double epix = easting  * VPoint.view_scale_ppm;
                                                double npix = northing * VPoint.view_scale_ppm;

                                                double dx = epix * cos(VPoint.skew) + npix * sin(VPoint.skew);
                                                double dy = npix * cos(VPoint.skew) - epix * sin(VPoint.skew);
                                                p.x = (int)round((VPoint.pix_width  / 2) + dx);
                                                p.y = (int)round((VPoint.pix_height / 2) - dy);

                                                *pr = p;
                                                pr++;
                                        }
                                        dc.DrawLines(seg_cnt, ptp);
                                }
                        }
                }       // for x
        }       //for y
        platray = NULL;
        plonray = NULL;
        psegray = NULL;
        wvsrtv (_T("clean"), y, x, &platray, &plonray, &psegray);
}
//------------------------------------------------------------------------------
//      WVSChart Static "C" Helper routines, credits within
//------------------------------------------------------------------------------
/*  wvsrtv  Retrieve WVS data from compressed files.
                            DISTRIBUTION STATEMENT
    This source file is unclassified, distribution unlimited, public
    domain.  It is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
        The following functions are used to access the compressed World
    Vector Shoreline (WVS) files as they were stored on the National
    Geophysical Data Center (NGDC) Global Relief Data CD-ROM.  These files,
    and the compression method used on them, were designed and built by
    myself and James Hammack (then of NORDA).  The work was done in 1989.
    At that time the 350MB that was required to store the entire WVS data
    set was considered too large to deal with easily.  In addition, the data
    was sequential access, ASCII format which required that you read the
    entire data set to retrieve all of the data for any subset.  The data is
    stored in a direct access data file as unsigned character data.  There
    are no endian or floating point format issues with this data set.  It is
    organized by one-degree cells so that any one-degree cell is accessible
    with two reads.  The compression used is a simple delta coding method.
    The full resolution data set was compressed from 350MB to 25MB.  In
    addition to the full resolution data set, which was slightly better than
    1:250,000 scale, Jerry Landrum (NORDA) used the Douglas-Peuker algorithm
    to sub-sample the data to lower resolution data sets.  The resulting
    files were then compressed using our method to produce the following
    files:
        wvsfull.dat    -    full resolution     -    25,313,280 bytes
        wvs250k.dat    -    1:250,000           -    18,232,320 bytes
        wvs1.dat       -    1:1,000,000         -    4,896,768 bytes
        wvs3.dat       -    1:3,000,000         -    5,121,024 bytes
        wvs12.dat      -    1:12,000,000        -    2,933,760 bytes
        wvs43.dat      -    1:43,000,000        -    2,233,344 bytes
    Notice that the 1:3,000,000 scale data set is actually larger than the
    1:1,000,000 scale data set.  Go figure.  There isn't much use in using
    the 1:3,000,000 scale data set unless you just want less detailed
    shorelines.  The NGDC Global Relief Data CD-ROM also contains the CIA
    World Data Bank II coast, rivers, and political boundaries data files.
    These functions will not work with those files.  The coastlines were
    only 1:1,000,000 scale and the rivers and political boundaries don't
    match with the WVS data.  These functions can be modified to work with
    that data by including segment breaking based on data rank but it hardly
    seems worth the effort (which is why I didn't do it).
        For the masochistic I recommend reading A Portable Method For
    Compressing and Storing High-Resolution Geographic Data Sets, Jan C.
    Depner and James A. Hammack, Naval Oceanographic Office, 1989.  It
    describes the compression, storage, and retrieving methods in painful
    detail.  Every time I read it I get confused all over again.
        The following functions allow the user to retrieve the data from
    the data files one degree at a time.  This is the way I should have done
    it in the first place.  You must define an environment variable (WVS_DIR)
    that points to the directory containing the coastline files.  The
    coastline files must use the names defined above (as they were stored on
    the NGDC CD-ROM).  The functions will "fall through" to the next level
    data file if the requested one is not available.  That is, if you request
    full resolution and all you have is 1:12,000,000 it will switch to the
    lower resolution file.  After the last call to wvsrtv you should call it
    one more time with "clean" as the file name.  This will cause it to free
    memory and close any open files.
        To paraphrase my heroes M. Horwitz, J. Horwitz, and L. Fineberg,
    a-plotting we will go!
    Jan C. Depner
    Naval Oceanographic Office
    depnerj@navo.navy.mil
    February 17, 2002
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#define         PHYSIZ      3072
#define         FILE_BUFFER_LEN 2048
#define         SIGN_OF(x)  ((x)<0.0 ? -1 : 1)
//RWL make paths either WIN or Unix
#ifdef WIN32
#define DIR_PATH_CHAR "\\"
#else
#define DIR_PATH_CHAR "/"
#endif
/***************************************************************************\
*       Function nxtrec                                                     *
*****************************************************************************
*                                                                           *
*       Reads next record in overflow chain.                                *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         Jan C. Depner                                                     *
*         Naval Oceanographic Office                                        *
*         Oceanographic Support Staff (Code N4O)                            *
*         Hydrographic Department                                           *
*         Stennis Space Center, MS                                          *
*         39522-5001                                                        *
*         depnerj@navo.navy.mil                                             *
*                                                                           *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       rindex    - cell index number for algorithm addressing, rindex =    *
*                   integer (lat degrees + 90) * 360 + integer lon          *
*                   degrees + 180 + 1 ; this is the logical record          *
*                   address of the first data segment in any cell.          *
*       bytbuf    - char array of 'PHYSIZ' length for i/o to the direct     *
*                   access file.                                            *
*       curpos    - current byte position within the physical record.       *
*       addr      - current physical record address in bytes from beginning *
*                   of the file.                                            *
*       lperp     - logical records per physical record.                    *
*       paddr     - previous physical record address.                       *
*       fp        - file pointer for the direct access file.                *
*       logrec    - length of a logical record in bytes.                    *
*                                                                           *
\***************************************************************************/
static void nxtrec (long *rindex, unsigned char *bytbuf, long *curpos,
                    long *addr, int lperp, long *paddr, FILE *fp, int logrec)
{
    int        stat;
    /*  Compute the index number for the next logical record in the chain.   */
    *rindex = bytbuf[*curpos + 1] * (long) 65536 + bytbuf[*curpos + 2] *
        (long) 256 + bytbuf[*curpos + 3];
    *addr = ((*rindex - 1) / lperp) * PHYSIZ;
    /*  If the physical record has changed since the last access, read a
        new physical record.  */
    if (*addr != *paddr)
    {
        stat = fseek (fp, *addr, 0);
        stat = fread (bytbuf, PHYSIZ, 1, fp);
    }
    /*  Set the previous physical address to the current one, and compute
        the current byte position for the new record.  */
    *paddr = *addr;
    *curpos = ((*rindex - 1) % lperp) * logrec;
}
/***************************************************************************\
*       Function movpos                                                     *
*****************************************************************************
*                                                                           *
*       Updates current position pointer and checks for end of record.      *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         Jan C. Depner                                                     *
*         Naval Oceanographic Office                                        *
*         Oceanographic Support Staff (Code N4O)                            *
*         Hydrographic Department                                           *
*         Stennis Space Center, MS                                          *
*         39522-5001                                                        *
*         depnerj@navo.navy.mil                                             *
*                                                                           *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       rindex    - cell index number for algorithm addressing, rindex =    *
*                   integer (lat degrees + 90) * 360 + integer lon          *
*                   degrees + 180 + 1 ; this is the logical record          *
*                   address of the first data segment in any cell.          *
*       bytbuf    - char array of 'PHYSIZ' length for i/o to the direct     *
*                   access file.                                            *
*       curpos    - current byte position within the physical record.       *
*       addr      - current physical record address in bytes from beginning *
*                   of the file.                                            *
*       lperp     - logical records per physical record.                    *
*       paddr     - previous physical record address.                       *
*       fp        - file pointer for the direct access file.                *
*       logrec    - length of a logical record in bytes.                    *
*       fulrec    - full record value (logrec-4).                           *
*                                                                           *
\***************************************************************************/
static void movpos (long *rindex, unsigned char *bytbuf, long *curpos,
                    long *addr, int lperp, long *paddr, FILE *fp, int logrec,
                    long fulrec)
{
    *curpos = *curpos + 2;
    /*  If we hit the end of the physical record, get the next one.  */
    if (*curpos % logrec == fulrec) nxtrec (rindex, bytbuf, curpos, addr,
        lperp, paddr, fp, logrec);
}
/***************************************************************************\
*       Function test_bit                                                   *
*****************************************************************************
*                                                                           *
*       Checks for bit set in an unsigned char.                             *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         Jan C. Depner                                                     *
*         Naval Oceanographic Office                                        *
*         Oceanographic Support Staff (Code N4O)                            *
*         Hydrographic Department                                           *
*         Stennis Space Center, MS                                          *
*         39522-5001                                                        *
*         depnerj@navo.navy.mil                                             *
*                                                                           *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       mask  - char array of bit masks                                     *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       byte      - should be self-explanatory                              *
*       bitpos    - bit position within byte                                *
*                                                                           *
\***************************************************************************/
static int test_bit (unsigned char byte, int bitpos)
{
    static unsigned char    mask[9] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20,
                            0x40, 0x80};
    return (byte & mask[bitpos]);
}
/***************************************************************************\
*       Function celchk                                                     *
*****************************************************************************
*                                                                           *
*       Checks for data in a given one-degree cell, reads bit map.          *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         Jan C. Depner                                                     *
*         Naval Oceanographic Office                                        *
*         Oceanographic Support Staff (Code N4O)                            *
*         Hydrographic Department                                           *
*         Stennis Space Center, MS                                          *
*         39522-5001                                                        *
*         depnerj@navo.navy.mil                                             *
*                                                                           *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       caddr  - current physical record address in bytes from beginning    *
*                of the file (cell map address).                            *
*       ndxpos - bit position within the 64800 bit cell map for the cell    *
*                pointed to by 'rindex'.                                    *
*       bytpos - byte position within the cell map of the 'rindex' cell.    *
*       bitpos - bit position within the 'bytpos' byte of the 'rindex' cell *
*                bit.                                                       *
*       chk    - logical value returned (true if there is data in the       *
*                'rindex' cell).                                            *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       rindex    - cell index number for algorithm addressing, rindex =    *
*                   integer (lat degrees + 90) * 360 + integer lon          *
*                   degrees + 180 + 1 ; this is the logical record          *
*                   address of the first data segment in any cell.          *
*       logrec    - length of a logical record in bytes.                    *
*       fp        - file pointer for the direct access file.                *
*       pcaddr    - previous physical record address (cell map address).    *
*                                                                           *
\***************************************************************************/
static int celchk (int rindex, int logrec, int offset, FILE *fp, long *pcaddr)
{
    static unsigned char    celbuf[PHYSIZ];
    static long             caddr;
    long                    ndxpos;
    int                     bytpos, bitpos, chk, stat;
    /*  Compute the physical address of the 'rindex' cell bit. */
    caddr = (((rindex + logrec * 8) - (offset + 1)) /
        (PHYSIZ * 8)) * PHYSIZ;
    /*  If this is the first access or the physical address has changed
        since the last access, read a new physical record.  */
    if (*pcaddr != caddr)
    {
        stat = fseek (fp, caddr, 0);
        stat = fread (celbuf, PHYSIZ, 1, fp);
    }
    /*  Set the previous address to the current one.  */
    *pcaddr = caddr;
    /*  Compute the 'rindex' position within the physical record.  */
    ndxpos = ((rindex + logrec * 8) - (offset + 1)) % (PHYSIZ * 8);
    /*  Compute the byte and bit positions.  */
    bytpos = ndxpos / 8;
    bitpos = 7 - ndxpos % 8;
    /*  Test the 'rindex' bit and return.  */
    chk = test_bit (celbuf[bytpos], bitpos);
    return (chk);
}
/***************************************************************************\
*       Function build_seg                                                  *
*****************************************************************************
*                                                                           *
*       Build the arrays containing the segments.                           *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         Jan C. Depner                                                     *
*         Naval Oceanographic Office                                        *
*         Oceanographic Support Staff (Code N4O)                            *
*         Hydrographic Department                                           *
*         Stennis Space Center, MS                                          *
*         39522-5001                                                        *
*         depnerj@navo.navy.mil                                             *
*                                                                           *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       dlat   - latitude of current point (degrees)                        *
*       dlon   - longitude of current point (degrees)                       *
*       cont   - segment continuation flag.                                 *
*       nsegs  - number of segments in this cell                            *
*       npts   - number of points in this segment                           *
*       total  - total points in this cell                                  *
*       latray - latitude array pointer                                     *
*       lonray - longitude array pointer                                    *
*       segray - segment count array pointer                                *
*       lnbias - longitude offset (if in 0-360 world)                       *
*                                                                           *
****************************************************************************/
static void build_seg (float dlat, float dlon, int *cont, int *nsegs,
                       int *npts, int *total, float **latray, float **lonray,
                       int **segray, int lnbias)
{
    /*  If this is beginning of a new segment, close the last segment.  */
    if (!*cont)
    {
        /*  Make sure there are at least two points in the arrays.  */
        if (*npts > 0)
        {
            if ((*segray = (int *) realloc (*segray, (*nsegs + 1) *
                sizeof (int))) == NULL)
            {
                perror (__FILE__);
                exit (-1);
            }
            (*segray)[*nsegs] = *npts + 1;
            (*nsegs)++;
        }
        else
        {
            /*  Back up a spot if we got a single point segment.  */
            if (*total >= 0) (*total)--;
        }
        *npts = -1;
    }
    /*  Reallocate memory and store point.  */
    (*npts)++;
    (*total)++;
    if ((*latray = (float *) realloc (*latray, ((*total) + 1) *
        sizeof (float))) == NULL || (*lonray = (float *) realloc (*lonray,
        ((*total) + 1) * sizeof (float))) == NULL)
    {
        perror (__FILE__);
        exit (-1);
    }
    (*latray)[*total] = dlat - 90.0;
    (*lonray)[*total] = dlon - 180.0 + lnbias;
    *cont = -1;
    return;
}
/***************************************************************************\
*      Function wvsrtv                                                      *
*****************************************************************************
*      Retrieves the WVS data from the direct access files.                 *
*                                                                           *
*                                                                           *
*                                                                           *
*       Author :                                                            *
*                                                                           *
*         Jan C. Depner                                                     *
*         Naval Oceanographic Office                                        *
*         Oceanographic Support Staff (Code N4O)                            *
*         Hydrographic Department                                           *
*         Stennis Space Center, MS                                          *
*         39522-5001                                                        *
*         depnerj@navo.navy.mil                                             *
*                                                                           *
*                                                                           *
*       Variable definitions :                                              *
*                                                                           *
*       PHYSIZ    - size of a physical record in the direct access file,    *
*                   in bytes.                                               *
*       bytbuf    - char array of 'PHYSIZ' length for i/o to the direct     *
*                   access file.                                            *
*       celbuf    - char array of 'PHYSIZ' length for i/o to the            *
*                   direct access file (used to retrieve cell map).         *
*       fp        - file pointer for the direct access file.                *
*       logrec    - length of a logical record in bytes.                    *
*       lperp     - logical records per physical record.                    *
*       offset    - number of logical records needed to store the bit       *
*                   cell map + 1 .                                          *
*       version   - wvsrtv software and data file version.                  *
*       rindex    - cell index number for algorithm addressing, rindex =    *
*                   integer (lat degrees + 90) * 360 + integer lon          *
*                   degrees + 180 + 1 ; this is the logical record          *
*                   address of the first data segment in any cell.          *
*       addr      - current physical record address in bytes from beginning *
*                   of the file.                                            *
*       paddr     - previous physical record address.                       *
*       pcaddr    - previous physical record address (cell map address).    *
*       stat      - seek and read status variable.                          *
*       curpos    - current byte position within the physical record.       *
*       fulrec    - full record value (logrec-4).                           *
*       latdd     - latitude of one-degree cell (degrees, SW corner).       *
*       londd     - longitude of one-degree cell (degrees, SW corner).      *
*       ioff      - divisor for delta records (1 - WDB, 10 - WVS).          *
*       i         - utility variable.                                       *
*       j         - utility variable.                                       *
*       col       - longitude index.                                        *
*       lat       - integer value of lat of one-degree cell (SW corner).    *
*       lon       - integer value of lon of one-degree cell (SW corner).    *
*       slatf     - integer value of southern latitude of file (degrees).   *
*       nlatf     - integer value of northern latitude of file (degrees).   *
*       wlonf     - integer value of western longitude of file (degrees).   *
*       elonf     - integer value of eastern longitude of file (degrees).   *
*       widef     - integer width of file in degrees.                       *
*       size      - number of cells in the input file (for subset file).    *
*       segcnt    - number of data points in the current segment.           *
*       cont      - segment continuation flag.                              *
*       cnt       - segment loop counter.                                   *
*       latsec    - delta latitude in seconds or tenths of seconds.         *
*       lonsec    - delta longitude in seconds or tenths of seconds.        *
*       latoff    - latitude offset (seconds or tenths of seconds).         *
*       lonoff    - longitude offset (seconds or tenths of seconds).        *
*       conbyt    - byte position of continuation pointer within physical   *
*                   record.                                                 *
*       eflag     - end of segment flag.                                    *
*       todeg     - conversion factor from deltas to degrees.               *
*       dummy     - dummy pointer for modf function.                        *
*                                                                           *
*       Arguments:                                                          *
*                                                                           *
*       file   - NGDC Global Relief Data file name (wvsfull.dat, etc) or    *
*                "clean" to clear up memory, close files, etc.              *
*       latd   - integer latitude degrees of one-degree cell (SW corner).   *
*       lond   - integer longitude degrees of one-degree cell (SW corner).  *
*       latray - array containing latitude points (returned)                *
*       lonray - array containing longitude points (returned)               *
*       segray - array containing counts for each segment (returned)        *
*                                                                           *
*       int    - number of segments, 0 on end, error, or no data            *
*                                                                           *
\***************************************************************************/
int wvsrtv (const wxString& sfile, int latd, int lond, float **latray, float **lonray,
int **segray)
{
    static FILE             *fp = NULL;
    static char             prev_file[FILE_BUFFER_LEN] = "";

//    static char             files[6][12] =
//                            {"wvsfull.dat", "wvs250k.dat", "wvs1.dat",
//                            "wvs3.dat", "wvs12.dat", "wvs43.dat"};
    static int              logrec, lperp, offset, version, slatf, nlatf,
                            wlonf, elonf, widef, ioff, first = 1, npts;
    static long             rindex, addr, paddr, curpos, fulrec, size, pcaddr;
    static float            todeg;
    unsigned char           bytbuf[PHYSIZ];
//    int                     i, j;
    int                     col, segcnt, stat, lat, lon, nsegs, total,
                            cont, cnt, latsec, lonsec, eflag, lnbias;
    long                    latoff, lonoff, conbyt, lats, lons;
//    char                    dirfil[512], dir[512], tmpfil[512];
    float                   dlat, dlon;

    char file[FILE_BUFFER_LEN];
    strncpy(file, sfile.mb_str(), FILE_BUFFER_LEN);
    file[FILE_BUFFER_LEN - 1] = 0;
    /*
#ifdef DEBUG
    fprintf (stderr, "%s %d\n", __FILE__, __LINE__);
#endif
    */
    /*  On the first call, set the arrays that will be malloc'ed to NULL.
        This is so that the user doesn't have to remember to do it (I
        always forget).  They will be freed and reset on subsequent
        "clean" calls.  */
    if (first)
    {
        *latray = (float *) NULL;
        *lonray = (float *) NULL;
        *segray = (int *) NULL;
        first = 0;
    }
    /*  If the word "clean" is passed in as the file name, clean up memory
        and close the open file.  */
    if (sfile.IsSameAs(_T("clean")))
    {
        if (*latray) free (*latray);
        if (*lonray) free (*lonray);
        if (*segray) free (*segray);
        *latray = (float *) NULL;
        *lonray = (float *) NULL;
        *segray = (int *) NULL;
        if (fp) fclose (fp);
        fp = (FILE *) NULL;
        strncpy (prev_file, file, FILE_BUFFER_LEN);
        return (0);
    }
    /*  Initialize variables, open file and read first record.  */
    nsegs = 0;
    npts = -1;
    total = -1;
    paddr = -1;
    pcaddr = -1;
    lats = 0;
    lons = 0;
    lnbias = 0;
//      DSR
//      Skip all this extra file logic.
//      We pass in the full path name of the file requested, return 0 if not found
    /*  Have we changed files?  */
    if (strncmp (file, prev_file, FILE_BUFFER_LEN))
    {
        strcpy (prev_file, file);
            fp = fopen (file, "rb");
                if(NULL == fp)
                        return 0;
        strncpy (prev_file, file, FILE_BUFFER_LEN);
        fp = fopen (file, "rb");
        if(NULL == fp)
          return 0;

#if 0           // Todo: dsr
        /*  Was there a file already opened?  */
        if (fp != NULL) fclose (fp);
        fp = NULL;
        /*  Use the environment variable WVS_DIR to get the         */
        /*  directory name.                                         */
        /*                                                          */
        /*  To set the variable in csh use :                        */
        /*                                                          */
        /*      setenv WVS_DIR /usr/wvswdb                          */
        /*                                                          */
        /*  To set the variable in sh, bash, or ksh use :           */
        /*                                                          */
        /*      WVS_DIR=/usr/wvswdb                                 */
        /*      export WVS_DIR                                      */
        /*  Don't handle the CIA WDB II files (they suck anyway).  */
        if (!strcmp (file, "coasts.dat") || !strcmp (file, "rivers.dat") ||
            !strcmp (file, "bounds.dat")) return (0);
        // DWF: revised to enable /etc/xtide.conf 2002-03-05
        // (This code will break if the paths get too long)
        {
          dir[0] = '\0';
          char *e = getenv ("WVS_DIR");
          if (e != NULL) {
            // DWF: dir has length 512, but so does dirfil, so leave room.
            assert (strlen (e) <= 499);
            strcpy (dir, e);
          }
#ifdef SUPER_ULTRA_VERBOSE_DEBUGGING
          else
            fprintf (stderr,
                "\n\nEnvironment variable WVS_DIR is not set\n\n");
#endif
          if (dir[0] == '\0') {
            FILE *configfile;
            Dstr buf;
            if ((configfile = fopen ("/etc/xtide.conf", "r"))) {
              buf.getline (configfile);
              buf.getline (configfile);
              fclose (configfile);
              if (buf.isNull())
                return 0;
              assert (buf.length() <= 499);
              strcpy (dir, buf.aschar());
            }
          }
        }
        /*  Determine the resolution of the file so that we can go to lower
            resolution if it is not available.  */
        strcpy (tmpfil, file);
        for (i = 0 ; i < 6 ; i++)
        {
            if (!strcmp (files[i], tmpfil))
            {
                // RWL Put in for Windoze
                sprintf (dirfil, "%s%s%s", dir, DIR_PATH_CHAR, tmpfil);
                fp = fopen (dirfil, "rb");
                /*  No lower resolution files were available, look for
                    higher.  */
                if (fp == NULL && i == 5)
                {
                    strcpy (tmpfil, file);
                    for (j = 5 ; j >= 0 ; j--)
                    {
                        if (!strcmp (files[j], tmpfil))
                        {
                            // RWL Put in for Windoze
                            sprintf (dirfil, "%s%s%s", dir, DIR_PATH_CHAR,
                                tmpfil);
                            fp = fopen (dirfil, "rb");
                            if (fp != NULL) break;
                            /*  No files found.  */
                            if (!j) return (0);
                            strcpy (tmpfil, files[j - 1]);
                        }
                    }
                }
                if (fp != NULL) break;
                strcpy (tmpfil, files[i + 1]);
            }
        }
#endif
        /*  Get the file info (we actually looked at sub-setting these).  */
        stat = fseek (fp, 0, 0);
        stat = fread (bytbuf, 3072, 1, fp);
        logrec = bytbuf[3];
        fulrec = logrec - 4;
        version = bytbuf[4];
        ioff = bytbuf[5];
        slatf = bytbuf[6] * 256 + bytbuf[7];
        nlatf = bytbuf[8] * 256 + bytbuf[9];
        wlonf = bytbuf[10] * 256 + bytbuf[11];
        elonf = bytbuf[12] * 256 + bytbuf[13];
        if (elonf < wlonf) elonf += 360;
        if (slatf + nlatf + wlonf + elonf == 0)
        {
            nlatf = 180;
            elonf = 360;
        }
        widef = elonf - wlonf;
        size = (nlatf - slatf) * (long) widef;
        todeg = 3600.0 * ioff;
        offset = (size - 1) / (logrec*8) + 2;
        lperp = PHYSIZ / logrec;
    }
    /*  Check for longitude entered in 0-360 world.  */
    if (lond > 180) lnbias = 360;
    /*  Compute integer values for retrieval and adjust if necessary. */
    lat = latd + 90;
    lon = lond;
    col = lon % 360;
    if (col < -180) col = col + 360;
    if (col >= 180) col = col - 360;
    col += 180;
    if (col < wlonf) col += 360;
    rindex = (lat - slatf) * (long) widef + (col - wlonf) + 1 + offset;
    /*  Check for cell outside of file area or no data.  */
    if (lat < slatf || lat >= nlatf || col < wlonf || col >= elonf ||
        !(celchk (rindex, logrec, offset, fp, &pcaddr))) return (0);
    /*  Compute physical record address, read record and save as previous
        address.  */
    eflag = 0;
    addr = ((rindex - 1) / lperp) * PHYSIZ;
    if (addr != paddr)
    {
        stat = fseek (fp, addr, 0);
        stat = fread (bytbuf, PHYSIZ, 1, fp);
    }
    paddr = addr;
    /*  Compute byte position within physical record.  */
    curpos = ((rindex - 1) % lperp) * logrec;
    /*  If not at end of segment, process the record.  */
    while (!eflag)
    {
        /*  Get first two bytes of header and break out count and
            continuation bit.  */
        segcnt = (bytbuf[curpos] % 128) * 4 + bytbuf[curpos + 1] / 64 + 1;
        cont = bytbuf[curpos] / 128;
        /*  If this is a continuation record get offsets from the second
            byte.  */
        if (cont)
        {
            latoff = ((bytbuf[curpos + 1] % 64) / 8) * (long) 65536;
            lonoff = (bytbuf[curpos + 1] % 8) * (long) 65536;
        }
        /*  If this is an initial record set the offsets to zero.  */
        else
        {
            latoff = 0;
            lonoff = 0;
        }
        /*  Update the current byte position and get a new record if
            necessary.  */
        movpos (&rindex, bytbuf, &curpos, &addr, lperp, &paddr, fp, logrec,
            fulrec);
        /*  Compute the rest of the latitude offset.  */
        latoff += bytbuf[curpos] * (long) 256 + bytbuf[curpos + 1];
        movpos (&rindex, bytbuf, &curpos, &addr, lperp, &paddr, fp, logrec,
            fulrec);
        /*  Compute the rest of the longitude offset.  */
        lonoff += bytbuf[curpos] * (long) 256 + bytbuf[curpos + 1];
        /*  If this is a continuation record, bias the lat and lon offsets
            and compute the position.  */
        if (cont)
        {
            latoff -= 262144;
            lonoff -= 262144;
            lats += latoff;
            lons += lonoff;
        }
        /*  Else, compute the position.             */
        else
        {
            lats = (int) (lat * todeg) + latoff;
            lons = (int) (col * todeg) + lonoff;
        }
        /*  Set the position.  */
        dlat = (float) lats / todeg;
        dlon = (float) lons / todeg;
        /*  Update the current byte position.       */
        curpos += 2;
        /*  Get the continuation pointer.           */
        conbyt = ((rindex-1) % lperp) * logrec + fulrec;
        /*  If there is no continuation pointer or the byte position is
            not at the position pointed to by the continuation pointer,
            process the segment data.  */
        if (bytbuf[conbyt] == 0 || (curpos + 1) % logrec <=
            bytbuf[conbyt])
        {
            /*  If at the end of the logical record, get the next record
                in the chain.  */
            if (curpos % logrec == fulrec && bytbuf[conbyt] == 0)
                nxtrec (&rindex, bytbuf, &curpos, &addr, lperp, &paddr,
                fp, logrec);
            build_seg (dlat, dlon, &cont, &nsegs, &npts, &total, latray,
                lonray, segray, lnbias);
            /*  If the end of the segment has been reached, set the end
                flag.  */
            if ((curpos + 1) % logrec == bytbuf[conbyt]) eflag = 1;
            /*  Process the segment.                */
            for (cnt = 2 ; cnt <= segcnt ; cnt++)
            {
                /*  Compute the position from the delta record.  */
                latsec = bytbuf[curpos] - 128;
                lats += latsec;
                dlat = (float) lats / todeg;
                lonsec = bytbuf[curpos + 1] - 128;
                lons += lonsec;
                dlon = (float) lons / todeg;
                build_seg (dlat, dlon, &cont, &nsegs, &npts, &total, latray,
                    lonray, segray, lnbias);
                curpos += 2;
                conbyt = ((rindex - 1) % lperp) * logrec + fulrec;
                /*  If the end of the segment has been reached, set the
                    end flag and break out of for loop.  */
                if ((curpos + 1) % logrec == bytbuf[conbyt])
                {
                    eflag = 1;
                    break;
                }
                else
                {
                    if (curpos % logrec == fulrec) nxtrec (&rindex,
                        bytbuf, &curpos, &addr, lperp, &paddr, fp, logrec);
                }
            }
        }
        /*  Break out of while loop if at the end of the segment.  */
        else
        {
            break;
        }                           /*  end if      */
    }                               /*  end while   */
    /*  Call the build_seg routine to flush the buffers.  */
    cont = 0;
    build_seg (999.0, 999.0, &cont, &nsegs, &npts, &total, latray, lonray,
        segray, lnbias);
#ifdef DEBUG
    fprintf (stderr, "%s %d\n", __FILE__, __LINE__);
#endif
    return (nsegs);
}
