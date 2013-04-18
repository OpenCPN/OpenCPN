/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

/*************************
Reader for a GRIB file

*************************/

#ifndef GRIBREADER_H
#define GRIBREADER_H

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers


#include <iostream>
#include <cmath>
#include <vector>
#include <set>
#include <map>

#include "GribRecord.h"
#include "zuFile.h"

//===============================================================
class GribReader
{
    public:
        GribReader();
        GribReader(const wxString fname);
        ~GribReader();

        void  openFile(const wxString fname);
        bool  isOk()                 {return ok;}
        long  getFileSize()          {return fileSize;}
        wxString getFileName()    {return fileName;}

        int          getNumberOfGribRecords(int dataType,int levelType,int levelValue);
        int          getTotalNumberOfGribRecords();

        GribRecord * getGribRecord(int dataType,int levelType,int levelValue, time_t date);

        GribRecord * getFirstGribRecord();
        GribRecord * getFirstGribRecord(int dataType,int levelType,int levelValue);

      std::vector<GribRecord *> * getListOfGribRecords(int dataType,int levelType,int levelValue);

//      double		 getHoursBeetweenGribRecords()  {return hoursBetweenRecords;}
      std::set<time_t>  getListDates()   {return setAllDates;}
      int        getNumberOfDates()      {return setAllDates.size();}
      time_t     getRefDate()            {return setAllDates.size()>0 ?
                                                       *setAllDates.begin() : 0;}

        // Valeur pour un point et une date quelconques
      double  getTimeInterpolatedValue   (int dataType,int levelType,int levelValue, double px, double py, time_t date);

        // Crée un GribRecord interpolé
        GribRecord * getTimeInterpolatedGribRecord (int dataType,int levelType,int levelValue, time_t date);

      double computeDewPoint(double lon, double lat, time_t date);

      int	   getDewpointDataStatus(int levelType,int levelValue);

        // Rectangle de la zone couverte par les données
      bool getZoneExtension (double *x0,double *y0, double *x1,double *y1);

      enum GribFileDataStatus {DATA_IN_FILE, NO_DATA_IN_FILE, COMPUTED_DATA};

      void  copyFirstCumulativeRecord   ();
      //void  removeFirstCumulativeRecord ();
      void  copyMissingWaveRecords ();
      void  copyFirstCumulativeRecord   (int dataType,int levelType,int levelValue);
      //void  removeFirstCumulativeRecord (int dataType,int levelType,int levelValue);
      void  copyMissingWaveRecords (int dataType,int levelType,int levelValue);

      std::map < std::string, std::vector<GribRecord *>* > * getGribMap(){ return  &mapGribRecords; }              //dsr

    private:
        bool      ok;
        wxString  fileName;
        ZUFILE    *file;
        long      fileSize;
//        double    hoursBetweenRecords;
        int       dewpointDataStatus;

        std::map < std::string, std::vector<GribRecord *>* >  mapGribRecords;

        void storeRecordInMap(GribRecord *rec);

        void   readGribFileContent();
        void   readAllGribRecords();
        void   createListDates();
        double computeHoursBeetweenGribRecords();
        std::set<time_t> setAllDates;

        void clean_vector(std::vector<GribRecord *> &ls);
        void clean_all_vectors();
        std::vector<GribRecord *> * getFirstNonEmptyList();

      // Interpolation between 2 GribRecord
        double      get2GribsInterpolatedValueByDate (
                                    double px, double py, time_t date,
                                    GribRecord *before, GribRecord *after);

      // Détermine les GribRecord qui encadrent une date
        void findGribsAroundDate (int dataType,int levelType,int levelValue, time_t date,
                  GribRecord **before, GribRecord **after);
};


#endif
