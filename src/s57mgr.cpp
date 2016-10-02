/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  S57 Chart Manager
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 */

#include "dychart.h"

#include "s57mgr.h"

#include "s57chart.h"

#include "cpl_port.h"
#include "cpl_csv.h"

#include "gdal.h"


static wxString *pval;

WX_DECLARE_STRING_HASH_MAP( wxString, EnvHash );

static EnvHash env;


//----------------------------------------------------------------------------------
//      mygetenv
//
//      Replacement for posix getenv() which works for __WXMSW__
//      Todo Make this thing into a couple of string arrays to stop leakage
//----------------------------------------------------------------------------------

/*
extern "C" char *mygetenv(char *pvar)
{
        wxString key(pvar);
        wxString test_val = env[key];
        if(test_val.Len())
        {
                pval->Empty();
                pval->Append(wxString(test_val));
                return((char *)pval->mb_str());

        }
        else
        {
                wxString val;
                wxGetEnv(key, &val);
                env[key] = val;
                pval->Empty();
                pval->Append(wxString(val));
                return((char *)pval->mb_str());
        }

}
*/
/*
#include <stdlib.h>
extern char **environ;
{
    wxLogMessage(_T("In my getenv"));

    void *t = (void *)environ;
    return NULL;
}
*/


//----------------------------------------------------------------------------------
//      s57mgr Implementation
//----------------------------------------------------------------------------------

s57mgr::s57mgr(const wxString& csv_dir)
{

#ifdef __WXMSW__
    pval = new wxString;                    // initialize static string
#endif



    //  MS Windows Build Note:
    //  In a .dll GDAL build, the following _putenv() call DOES NOT properly
    //  set the environment accessible to GDAL/OGR.  So, S57 Reader options
    //  are not set AT ALL.  Defaults will apply.
    //  See the README file

#ifdef __WXMSW__
    wxString envs1("S57_CSV=");
    envs1.Append(csv_dir);
    _putenv( envs1.mb_str());
#else
    wxSetEnv( "S57_CSV", csv_dir.mb_str());
#endif


        //  Set some S57 OGR Options thru environment variables

        //  n.b. THERE IS A BUG in GDAL/OGR 1.2.0 wherein the sense of the flag UPDATES= is reversed.
        //  That is, anything other than UPDATES=APPLY selects update mode.
        //  Conversely, UPDATES=APPLY deselects updates.
        //  Fixed in GDAL 1.3.2, at least, maybe earlier??
        //  Detect by GDALVersion check

    wxString set1, set2;

    set1 ="LNAM_REFS=ON";
    set1.Append(",SPLIT_MULTIPOINT=OFF");
    set1.Append(",ADD_SOUNDG_DEPTH=OFF");
    set1.Append(",PRESERVE_EMPTY_NUMBERS=OFF");
    set1.Append(",RETURN_PRIMITIVES=OFF");
    set1.Append(",RETURN_LINKAGES=OFF");


    const char *version_string = GDALVersionInfo("VERSION_NUM");
    int ver_num = (int)CPLScanLong((char *)version_string, 4);

    if(ver_num < 1320)
        set2 = ",UPDATES=BUGBUG";               // updates ENABLED
    else
        set2 = ",UPDATES=APPLY";

    set1.Append(set2);

#ifdef __WXMSW__
    wxString envs2("OGR_S57_OPTIONS=");
    envs2.Append(set1);
    _putenv( envs2.mb_str());

#else
    wxSetEnv("OGR_S57_OPTIONS",set1.mb_str());
#endif

    pcsv_locn = new wxString(csv_dir);

//    CPLSetConfigOption( "CPL_DEBUG", "ON");
//    CPLSetConfigOption( "CPL_LOG", "c:\\LOG");

    RegisterOGRS57();


}

s57mgr::~s57mgr()
{

      delete pcsv_locn;
      delete pval;

      //    Close and release any csv file access elements,
      //    Particularly the s57objectclasses.csv used for s57 object query support
      CSVDeaccess( NULL );

}






//----------------------------------------------------------------------------------
// Get First Chart M_COVR Object
//              n.b. Caller owns the data source and the feature on success
//----------------------------------------------------------------------------------
bool s57mgr::GetChartFirstM_COVR(char *pFullPath, OGRDataSource **pDS, OGRFeature **pFeature,
                                  OGRLayer **pLayer, int &catcov)
{

    OGRDataSource *poDS = OGRSFDriverRegistrar::Open( pFullPath );

    *pDS = poDS;                                    // give to caller

    if( poDS == NULL )
    {
        *pFeature = NULL;
        return false;
    }

    OGRLayer *pLay = poDS->GetLayerByName("M_COVR");
    *pLayer = pLay;                         // Give to caller
    pLay->ResetReading();
    OGRFeature *objectDef = pLay->GetNextFeature();
    *pFeature = objectDef;                  // Give to caller

    if(objectDef)
    {
    //  Fetch the CATCOV attribute
        for( int iField = 0; iField < objectDef->GetFieldCount(); iField++ )
        {
            if( objectDef->IsFieldSet( iField ) )
            {
                OGRFieldDefn *poFDefn = objectDef->GetDefnRef()->GetFieldDefn(iField);
                if(!strcmp(poFDefn->GetNameRef(), "CATCOV"))
                    catcov = objectDef->GetFieldAsInteger( iField );
            }
        }
        return true;
    }

    else
    {
        delete poDS;
        *pDS = NULL;
        return false;
    }

}

//----------------------------------------------------------------------------------
// GetNext Chart M_COVR Object
//              n.b. Caller still owns the data source and the feature on success
//----------------------------------------------------------------------------------
bool s57mgr::GetChartNextM_COVR(OGRDataSource *pDS, OGRLayer *pLayer, OGRFeature *pLastFeature,
                              OGRFeature **pFeature, int &catcov)
{


    if( pDS == NULL )
        return false;

    catcov = -1;


    int fid = pLastFeature->GetFID();

    OGRFeature *objectDef = pLayer->GetFeature(fid + 1);
    *pFeature = objectDef;                  // Give to caller

    if(objectDef)
    {
        for( int iField = 0; iField < objectDef->GetFieldCount(); iField++ )
        {
            if( objectDef->IsFieldSet( iField ) )
            {
                OGRFieldDefn *poFDefn = objectDef->GetDefnRef()->GetFieldDefn(iField);
                if(!strcmp(poFDefn->GetNameRef(), "CATCOV"))
                    catcov = objectDef->GetFieldAsInteger( iField );
            }
        }
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------------
// Get Chart Extents
//----------------------------------------------------------------------------------
bool s57mgr::GetChartExtent(char *pFullPath, Extent *pext)
{
 //   Fix this  find extents of which?? layer??
/*
        OGRS57DataSource *poDS = new OGRS57DataSource;
    poDS->Open(pFullPath, TRUE);

    if( poDS == NULL )
       return false;

    OGREnvelope Env;
    S57Reader   *poReader = poDS->GetModule(0);
    poReader->GetExtent(&Env, true);

    pext->NLAT = Env.MaxY;
    pext->ELON = Env.MaxX;
    pext->SLAT = Env.MinY;
    pext->WLON = Env.MinX;

    delete poDS;
*/
    return true;

}


//----------------------------------------------------------------------------------
// Get Chart Scale
//----------------------------------------------------------------------------------
int s57mgr::GetChartScale(char *pFullPath)
{

    DDFModule   *poModule;
    DDFRecord   *poRecord;
    int scale;

    poModule = new DDFModule();

    if( !poModule->Open(pFullPath) )
    {
        delete poModule;
        return 0;
    }

    poRecord = poModule->ReadRecord();
    if( poRecord == NULL )
    {
         poModule->Close();
         delete poModule;
         return 0;
    }

    scale = 1;
    for( ; poRecord != NULL; poRecord = poModule->ReadRecord() )
    {
        if( poRecord->FindField( "DSPM" ) != NULL )
        {
            scale = poRecord->GetIntSubfield("DSPM",0,"CSCL",0);
                        break;
        }
    }

    poModule->Close();
    delete poModule;

    return scale;

}


