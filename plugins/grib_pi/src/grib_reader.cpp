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

/**
 * \file
 *
 * \implements \ref grib_reader.h
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "grib_reader.h"
#include "grib_v1_record.h"
#include "grib_v2_record.h"
#include <cassert>

//-------------------------------------------------------------------------------
GribReader::GribReader() {
  ok = false;
  dewpointDataStatus = NO_DATA_IN_FILE;
}
//-------------------------------------------------------------------------------
GribReader::GribReader(const wxString fname) {
  ok = false;
  dewpointDataStatus = NO_DATA_IN_FILE;
  if (fname != "") {
    OpenFile(fname);
  } else {
    CleanAllVectors();
  }
}
//-------------------------------------------------------------------------------
GribReader::~GribReader() {
  CleanAllVectors();
  if (file != nullptr) {
    zu_close(file);
    file = nullptr;
  }
}

//-------------------------------------------------------------------------------
void GribReader::CleanAllVectors() {
  std::map<std::string, std::vector<GribRecord *> *>::iterator it;
  for (it = mapGribRecords.begin(); it != mapGribRecords.end(); it++) {
    std::vector<GribRecord *> *ls = (*it).second;
    CleanVector(*ls);
    delete ls;
  }
  mapGribRecords.clear();
}
//-------------------------------------------------------------------------------
void GribReader::CleanVector(std::vector<GribRecord *> &ls) {
  std::vector<GribRecord *>::iterator it;
  for (it = ls.begin(); it != ls.end(); it++) {
    delete *it;
    *it = nullptr;
  }
  ls.clear();
}

//---------------------------------------------------------------------------------
void GribReader::storeRecordInMap(GribRecord *rec) {
#if 0
    fprintf(stderr,
        "GribReader: STORE record type: data_type=%d level_type=%d level_value=%d idCenter==%d && idModel==%d && idGrid==%d\n",
            rec->GetDataType(), rec->GetLevelType(), rec->GetLevelValue(),
            rec->GetIdCenter(), rec->GetIdModel(), rec->GetIdGrid()
        );
#endif
  std::map<std::string, std::vector<GribRecord *> *>::iterator it;
  it = mapGribRecords.find(rec->GetKey());
  if (it == mapGribRecords.end()) {
    mapGribRecords[rec->GetKey()] = new std::vector<GribRecord *>;
    assert(mapGribRecords[rec->GetKey()]);
  }
  mapGribRecords[rec->GetKey()]->push_back(rec);
}

//---------------------------------------------------------------------------------
static bool RecordIsWind(GribRecord *rec) {
  return rec->GetDataType() == GRB_WIND_VX ||
         rec->GetDataType() == GRB_WIND_VY ||
         rec->GetDataType() == GRB_WIND_DIR ||
         rec->GetDataType() == GRB_WIND_SPEED;
}

//---------------------------------------------------------------------------------
static bool RecordIsGust(GribRecord *rec) {
  return rec->GetDataType() == GRB_WIND_GUST_VX ||
         rec->GetDataType() == GRB_WIND_GUST_VY ||
         rec->GetDataType() == GRB_WIND_GUST;
}

static bool RecordIsCurrent(GribRecord *rec) {
  return rec->GetDataType() == GRB_UOGRD || rec->GetDataType() == GRB_VOGRD ||
         rec->GetDataType() == GRB_CUR_DIR ||
         rec->GetDataType() == GRB_CUR_SPEED;
}

void GribReader::ReadAllGribRecords() {
  //--------------------------------------------------------
  // Lecture de l'ensemble des GribRecord du fichier
  // et stockage dans les listes appropriées.
  //--------------------------------------------------------
  GribRecord *rec = nullptr;
  GribRecord *prevDataSet = nullptr;
  int id = 0;
  time_t firstdate = -1;
  bool b_EOF;
  bool is_v2 = false;

  do {
    id++;
    // use the previously seen record type first
    // a miss with compressed file is really slow as
    // seek may mean re reading and decompressing the
    // file from the start

    if (is_v2 == false) {
      rec = new GribV1Record(file, id);
      if (rec->IsOk() == false) {
        delete rec;
        rec = new GribV2Record(file, id);
        is_v2 = rec->IsOk();
      }
    } else {
      GribV2Record *rec2 = dynamic_cast<GribV2Record *>(rec);
      if (rec2 && rec2->hasMoreDataSet()) {
        rec = rec2->GribV2NextDataSet(file, id);
        delete prevDataSet;
      } else {
        rec = new GribV2Record(file, id);
      }

      is_v2 = rec->IsOk();
      if (rec->IsOk() == false) {
        delete rec;
        rec = new GribV1Record(file, id);
      }
    }
    prevDataSet = nullptr;
    if (rec->IsOk() == false) {
      delete rec;
      break;
    }
    b_EOF = rec->IsEof();

    if (!rec->IsDataKnown()) {
      GribV2Record *rec2 = dynamic_cast<GribV2Record *>(rec);
      if (rec2 == nullptr || !rec2->hasMoreDataSet()) {
        delete rec;
        rec = nullptr;
      } else {  // must delete it in the next iteration
        prevDataSet = rec;
      }
      continue;
    }
    ok = true;  // au moins 1 record ok

    if (firstdate == -1) firstdate = rec->GetRecordCurrentDate();

    if ((rec->GetDataType() == GRB_PRESSURE && rec->GetLevelValue() == 0 &&
         (rec->GetLevelType() == LV_MSL ||
          rec->GetLevelType() == LV_GND_SURF)) ||
        (RecordIsWind(rec) && rec->GetLevelType() == LV_ABOV_GND &&
         rec->GetLevelValue() == 10) ||
        (RecordIsWind(rec) &&
         rec->GetLevelType() == LV_ISOBARIC  // wind at x hpa
         && (rec->GetLevelValue() == 850 || rec->GetLevelValue() == 700 ||
             rec->GetLevelValue() == 500 || rec->GetLevelValue() == 300)))
      storeRecordInMap(rec);

    else if ((RecordIsGust(rec) && rec->GetLevelType() == LV_GND_SURF &&
              rec->GetLevelValue() == 0))
      storeRecordInMap(rec);

    else if (RecordIsWind(rec) && rec->GetLevelType() == LV_GND_SURF)
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_TEMP  // Air temperature at 2m
             && rec->GetLevelType() == LV_ABOV_GND && rec->GetLevelValue() == 2)
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_TEMP  // Air temperature at x hpa
             && rec->GetLevelType() == LV_ISOBARIC &&
             (rec->GetLevelValue() == 850 || rec->GetLevelValue() == 700 ||
              rec->GetLevelValue() == 500 || rec->GetLevelValue() == 300))
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_PRECIP_TOT  // total rainfall
             && rec->GetLevelType() == LV_GND_SURF && rec->GetLevelValue() == 0)
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_PRECIP_RATE &&
             rec->GetLevelType() == LV_GND_SURF && rec->GetLevelValue() == 0)
      storeRecordInMap(rec);

    else if ((rec->GetDataType() == GRB_CLOUD_TOT  // cloud cover
              || rec->GetDataType() == GRB_COMP_REFL) &&
             rec->GetLevelType() == LV_ATMOS_ALL && rec->GetLevelValue() == 0)
      storeRecordInMap(rec);
    else if (rec->GetDataType() == GRB_HTSGW)  // Significant Wave Height
      storeRecordInMap(rec);

    else if (rec->GetDataType() ==
             GRB_PER)  // Combined Wind Waves and Swell period
      storeRecordInMap(rec);

    else if (rec->GetDataType() ==
             GRB_DIR)  // Combined Wind Waves and Swell Direction
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_WVHGT)  // Wind Wave Height
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_WVPER)  // Wind Waves period
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_WVDIR)  // Wind Waves Direction
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_CRAIN)  // Catagorical Rain  1/0
      storeRecordInMap(rec);

    else if ((rec->GetDataType() == GRB_WTMP) &&
             (rec->GetLevelType() == LV_GND_SURF) &&
             (rec->GetLevelValue() == 0))
      storeRecordInMap(rec);  // rtofs Water Temp + translated gfs Water Temp

    else if (RecordIsCurrent(rec))  // rtofs model sea current current
      storeRecordInMap(rec);

    else if (rec->GetDataType() == GRB_CAPE &&
             rec->GetLevelType() == LV_GND_SURF &&
             rec->GetLevelValue() == 0)  // Potential energy
      storeRecordInMap(rec);

    else if ((rec->GetDataType() == GRB_GEOPOT_HGT &&
              rec->GetLevelType() ==
                  LV_ISOBARIC)  // geopotentiel geight at x hpa
             && (rec->GetLevelValue() == 850 || rec->GetLevelValue() == 700 ||
                 rec->GetLevelValue() == 500 || rec->GetLevelValue() == 300))
      storeRecordInMap(rec);

    else if ((rec->GetDataType() == GRB_HUMID_REL &&
              rec->GetLevelType() == LV_ISOBARIC)  // relative humidity at x hpa
             && (rec->GetLevelValue() == 850 || rec->GetLevelValue() == 700 ||
                 rec->GetLevelValue() == 500 || rec->GetLevelValue() == 300))
      storeRecordInMap(rec);

    else {
      GribV2Record *rec2 = dynamic_cast<GribV2Record *>(rec);
#if 0
            fprintf(stderr,
                       "GribReader: unknown record type: data_type=%d level_type=%d level_value=%d idCenter==%d && idModel==%d && idGrid==%d\n",
                       rec->GetDataType(), rec->GetLevelType(), rec->GetLevelValue(),
                       rec->GetIdCenter(), rec->GetIdModel(), rec->GetIdGrid()
                );
#endif
      if (rec2 == nullptr || !rec2->hasMoreDataSet()) {
        delete rec;
        rec = nullptr;
      } else {
        prevDataSet = rec;
      }
    }
  } while (!b_EOF);
  delete prevDataSet;
}

//---------------------------------------------------------------------------------
void GribReader::CopyFirstCumulativeRecord(int dataType, int levelType,
                                           int levelValue) {
  time_t dateref = GetRefDate();
  GribRecord *rec = GetGribRecord(dataType, levelType, levelValue, dateref);
  if (rec == nullptr) {
    rec = GetFirstGribRecord(dataType, levelType, levelValue);
    if (rec != nullptr) {
      GribRecord *r2 = new GribRecord(*rec);
      r2->SetRecordCurrentDate(dateref);  // 1er enregistrement factice
      storeRecordInMap(r2);
    }
  }
}
/*
//---------------------------------------------------------------------------------
void  GribReader::removeFirstCumulativeRecord (int data_type,int level_type,int
level_value)
{
    time_t dateref = getRefDate();
    GribRecord *rec = getFirstGribRecord(data_type, level_type, level_value);

    if (rec!=nullptr  &&  rec->GetRecordCurrentDate() == dateref)
    {
        std::vector<GribRecord *> *liste = getListOfGribRecords(data_type,
level_type, level_value); if (liste != nullptr) { std::vector<GribRecord
*>::iterator it; for (it=liste->begin(); it!=liste->end() && (*it)!=rec; it++)
            {
            }
            if ((*it) == rec) {
                liste->erase(it);
            }
        }
    }
}
*/
void GribReader::CopyMissingWaveRecords(int dataType, int levelType,
                                        int levelValue) {
  std::set<time_t> setdates = GetListDates();
  std::set<time_t>::iterator itd, itd2;
  for (itd = setdates.begin(); itd != setdates.end(); itd++) {
    time_t date = *itd;
    GribRecord *rec = GetGribRecord(dataType, levelType, levelValue, date);
    if (rec == nullptr) {
      itd2 = itd;
      itd2++;  // next date
      if (itd2 != setdates.end()) {
        time_t date2 = *itd2;
        GribRecord *rec2 =
            GetGribRecord(dataType, levelType, levelValue, date2);
        if (rec2 && rec2->IsOk()) {
          // create a copied record from date2
          GribRecord *r2 = new GribRecord(*rec2);
          r2->SetRecordCurrentDate(date);
          storeRecordInMap(r2);
        }
      }
    }
  }
}

void GribReader::ComputeAccumulationRecords(int dataType, int levelType,
                                            int levelValue) {
  std::set<time_t> setdates = GetListDates();
  std::set<time_t>::reverse_iterator rit;
  GribRecord *prev = 0;
  int p1 = 0, p2 = 0;

  if (setdates.empty()) return;

  // XXX only work if P2 -P1 === time
  for (rit = setdates.rbegin(); rit != setdates.rend(); ++rit) {
    time_t date = *rit;
    GribRecord *rec = GetGribRecord(dataType, levelType, levelValue, date);
    if (rec && rec->IsOk()) {
      // XXX double check reference date and timerange
      if (prev != 0) {
        if (prev->GetPeriodP1() == rec->GetPeriodP1()) {
          // printf("substract %d %d %d\n", prev->getPeriodP1(),
          // prev->GetPeriodP2(), prev->GetPeriodSec());
          if (rec->GetTimeRange() == 4) {
            // accumulation
            // prev = prev -rec
            prev->Substract(*rec);
            p1 = rec->GetPeriodP2();
          } else if (rec->GetTimeRange() == 3) {
            // average
            // prev = (prev*d2 - rec*d1) / (double) (d2 - d1);
            prev->Average(*rec);
            p1 = rec->GetPeriodP2();
          }
        }
        // convert to mm/h
        if (p2 > p1 && rec->GetTimeRange() == 4) {
          prev->multiplyAllData(1.0 / (p2 - p1));
        }
        p2 = p1 = 0;
      }
      prev = rec;
      p1 = prev->GetPeriodP1();
      p2 = prev->GetPeriodP2();
    }
  }
  if (prev != 0 && p2 > p1 && prev->GetTimeRange() == 4) {
    // the last one
    prev->multiplyAllData(1.0 / (p2 - p1));
  }
}

//---------------------------------------------------------------------------------
void GribReader::CopyFirstCumulativeRecord() {
  CopyFirstCumulativeRecord(GRB_CLOUD_TOT, LV_ATMOS_ALL, 0);
  CopyFirstCumulativeRecord(GRB_PRECIP_TOT, LV_GND_SURF, 0);
}
/*
//---------------------------------------------------------------------------------
void  GribReader::removeFirstCumulativeRecord()
{
    removeFirstCumulativeRecord(GRB_TMIN, LV_ABOV_GND, 2);
    removeFirstCumulativeRecord(GRB_TMAX, LV_ABOV_GND, 2);
    removeFirstCumulativeRecord(GRB_CLOUD_TOT,   LV_ATMOS_ALL, 0);
    removeFirstCumulativeRecord(GRB_PRECIP_TOT,  LV_GND_SURF, 0);
    removeFirstCumulativeRecord(GRB_PRECIP_RATE, LV_GND_SURF, 0);
    removeFirstCumulativeRecord(GRB_SNOW_CATEG,  LV_GND_SURF, 0);
    removeFirstCumulativeRecord(GRB_FRZRAIN_CATEG, LV_GND_SURF, 0);
}
*/
void GribReader::CopyMissingWaveRecords() {
  CopyMissingWaveRecords(GRB_HTSGW, LV_GND_SURF, 0);
  CopyMissingWaveRecords(GRB_WVDIR, LV_GND_SURF, 0);
  CopyMissingWaveRecords(GRB_WVPER, LV_GND_SURF, 0);
  CopyMissingWaveRecords(GRB_DIR, LV_GND_SURF, 0);
  CopyMissingWaveRecords(GRB_PER, LV_GND_SURF, 0);
}

//---------------------------------------------------------------------------------
void GribReader::ReadGribFileContent() {
  fileSize = zu_filesize(file);
  ReadAllGribRecords();
  CreateListDates();
  //    hoursBetweenRecords = computeHoursBeetweenGribRecords();
  // XXX should it be done after reading all files, rather than per file?
  if (GetNumberOfGribRecords(GRB_WIND_GUST, LV_GND_SURF, 0) == 0) {
    for (auto date : SetAllDates) {
      GribRecord *recX = GetGribRecord(GRB_WIND_GUST_VX, LV_GND_SURF, 0, date);
      if (recX == nullptr) continue;

      GribRecord *recY = GetGribRecord(GRB_WIND_GUST_VY, LV_GND_SURF, 0, date);
      if (recY == nullptr) continue;
      GribRecord *rec = GribRecord::MagnitudeRecord(*recX, *recY);
      rec->SetDataType(GRB_WIND_GUST);
      storeRecordInMap(rec);
    }
  }
  //-----------------------------------------------------
  // Are dewpoint data in file ?
  // If no, compute it with Magnus-Tetens formula, if possible.
  //-----------------------------------------------------
  dewpointDataStatus = DATA_IN_FILE;
  if (GetNumberOfGribRecords(GRB_DEWPOINT, LV_ABOV_GND, 2) != 0) return;

  dewpointDataStatus = NO_DATA_IN_FILE;
  if (GetNumberOfGribRecords(GRB_HUMID_REL, LV_ABOV_GND, 2) == 0 ||
      GetNumberOfGribRecords(GRB_TEMP, LV_ABOV_GND, 2) == 0)
    return;

  dewpointDataStatus = COMPUTED_DATA;
  for (auto iter : SetAllDates) {
    time_t date = iter;
    GribRecord *recModel = GetGribRecord(GRB_TEMP, LV_ABOV_GND, 2, date);
    if (recModel == nullptr) continue;

    // Crée un GribRecord avec les dewpoints calculés
    GribRecord *recDewpoint = new GribRecord(*recModel);
    recDewpoint->SetDataType(GRB_DEWPOINT);
    for (zuint i = 0; i < (zuint)recModel->GetNi(); i++) {
      for (zuint j = 0; j < (zuint)recModel->GetNj(); j++) {
        double x, y;
        recModel->getXY(i, j, &x, &y);
        double dp = ComputeDewPoint(x, y, date);
        recDewpoint->SetValue(i, j, dp);
      }
    }
    storeRecordInMap(recDewpoint);
  }
}

//---------------------------------------------------
int GribReader::GetDewpointDataStatus(int /*level_type*/, int /*level_value*/) {
  return dewpointDataStatus;
}

//---------------------------------------------------
int GribReader::GetTotalNumberOfGribRecords() {
  int nb = 0;
  std::map<std::string, std::vector<GribRecord *> *>::iterator it;
  for (it = mapGribRecords.begin(); it != mapGribRecords.end(); it++) {
    nb += (*it).second->size();
  }
  return nb;
}

//---------------------------------------------------
std::vector<GribRecord *> *GribReader::GetFirstNonEmptyList() {
  std::vector<GribRecord *> *ls = nullptr;
  std::map<std::string, std::vector<GribRecord *> *>::iterator it;
  for (it = mapGribRecords.begin(); ls == nullptr && it != mapGribRecords.end();
       it++) {
    if ((*it).second->size() > 0) ls = (*it).second;
  }
  return ls;
}

//---------------------------------------------------
int GribReader::GetNumberOfGribRecords(int dataType, int levelType,
                                       int levelValue) {
  std::vector<GribRecord *> *liste =
      getListOfGribRecords(dataType, levelType, levelValue);
  if (liste != nullptr)
    return liste->size();
  else
    return 0;
}

//---------------------------------------------------------------------
std::vector<GribRecord *> *GribReader::getListOfGribRecords(int dataType,
                                                            int levelType,
                                                            int levelValue) {
  std::string key = GribRecord::MakeKey(dataType, levelType, levelValue);
  if (mapGribRecords.find(key) != mapGribRecords.end())
    return mapGribRecords[key];
  else
    return nullptr;
}
//---------------------------------------------------------------------------
double GribReader::GetTimeInterpolatedValue(int dataType, int levelType,
                                            int levelValue, double px,
                                            double py, time_t date) {
  GribRecord *before, *after;
  FindGribsAroundDate(dataType, levelType, levelValue, date, &before, &after);
  return Get2GribsInterpolatedValueByDate(px, py, date, before, after);
}

//------------------------------------------------------------------
void GribReader::FindGribsAroundDate(int dataType, int levelType,
                                     int levelValue, time_t date,
                                     GribRecord **before, GribRecord **after) {
  // Cherche les GribRecord qui encadrent la date
  std::vector<GribRecord *> *ls =
      getListOfGribRecords(dataType, levelType, levelValue);
  *before = nullptr;
  *after = nullptr;
  zuint nb = ls->size();
  for (zuint i = 0; i < nb && *before == nullptr && *after == nullptr; i++) {
    GribRecord *rec = (*ls)[i];
    if (rec->GetRecordCurrentDate() == date) {
      *before = rec;
      *after = rec;
    } else if (rec->GetRecordCurrentDate() < date) {
      *before = rec;
    } else if (rec->GetRecordCurrentDate() > date && *before != nullptr) {
      *after = rec;
    }
  }
}

//------------------------------------------------------------------
double GribReader::Get2GribsInterpolatedValueByDate(double px, double py,
                                                    time_t date,
                                                    GribRecord *before,
                                                    GribRecord *after) {
  double val = GRIB_NOTDEF;
  if (before != nullptr && after != nullptr) {
    if (before == after) {
      val = before->GetInterpolatedValue(px, py);
    } else {
      time_t t1 = before->GetRecordCurrentDate();
      time_t t2 = after->GetRecordCurrentDate();
      if (t1 == t2) {
        val = before->GetInterpolatedValue(px, py);
      } else {
        double v1 = before->GetInterpolatedValue(px, py);
        double v2 = after->GetInterpolatedValue(px, py);
        if (v1 != GRIB_NOTDEF && v2 != GRIB_NOTDEF) {
          double k = fabs((double)(date - t1) / (t2 - t1));
          val = (1.0 - k) * v1 + k * v2;
        }
      }
    }
  }
  return val;
}

//---------------------------------------------------
// Premier GribRecord trouvé (pour récupérer la grille)
GribRecord *GribReader::GetFirstGribRecord() {
  std::vector<GribRecord *> *ls = GetFirstNonEmptyList();
  if (ls != nullptr) {
    return ls->at(0);
  } else {
    return nullptr;
  }
}
//---------------------------------------------------
// Premier GribRecord (par date) pour un type donné
GribRecord *GribReader::GetFirstGribRecord(int dataType, int levelType,
                                           int levelValue) {
  std::set<time_t>::iterator it;
  GribRecord *rec = nullptr;
  for (it = SetAllDates.begin(); rec == nullptr && it != SetAllDates.end();
       it++) {
    time_t date = *it;
    rec = GetGribRecord(dataType, levelType, levelValue, date);
  }
  return rec;
}
//---------------------------------------------------
// Délai en heures entre 2 records
// On suppose qu'il est fixe pour tout le fichier !!!
// NOT USED
double GribReader::ComputeHoursBeetweenGribRecords() {
  double res = 1;
  std::vector<GribRecord *> *ls = GetFirstNonEmptyList();
  if (ls != nullptr) {
    time_t t0 = (*ls)[0]->GetRecordCurrentDate();
    time_t t1 = (*ls)[1]->GetRecordCurrentDate();
    res = fabs((double)(t1 - t0)) / 3600.0;
    if (res < 1) res = 1;
  }
  return res;
}
//---------------------------------------------------
GribRecord *GribReader::GetGribRecord(int dataType, int levelType,
                                      int levelValue, time_t date) {
  std::vector<GribRecord *> *ls =
      getListOfGribRecords(dataType, levelType, levelValue);
  if (ls != nullptr) {
    // Cherche le premier enregistrement à la bonne date
    GribRecord *res = nullptr;
    zuint nb = ls->size();
    for (zuint i = 0; i < nb && res == nullptr; i++) {
      if ((*ls)[i]->GetRecordCurrentDate() == date) res = (*ls)[i];
    }
    return res;
  } else {
    return nullptr;
  }
}

//-------------------------------------------------------
// Génère la liste des dates pour lesquelles des prévisions existent
void GribReader::CreateListDates() {  // Le set assure l'ordre et l'unicité des
                                      // dates
  SetAllDates.clear();
  std::map<std::string, std::vector<GribRecord *> *>::iterator it;
  for (it = mapGribRecords.begin(); it != mapGribRecords.end(); it++) {
    std::vector<GribRecord *> *ls = (*it).second;
    for (zuint i = 0; i < ls->size(); i++) {
      SetAllDates.insert(ls->at(i)->GetRecordCurrentDate());
    }
  }
}

//-------------------------------------------------------
double GribReader::ComputeDewPoint(double lon, double lat, time_t now) {
  double diewpoint = GRIB_NOTDEF;

  GribRecord *recTempDiew = GetGribRecord(GRB_DEWPOINT, LV_ABOV_GND, 2, now);
  if (recTempDiew != nullptr) {
    // GRIB file contains diew point data
    diewpoint = recTempDiew->GetInterpolatedValue(lon, lat);
  } else {
    // Compute diew point with Magnus-Tetens formula
    GribRecord *recTemp = GetGribRecord(GRB_TEMP, LV_ABOV_GND, 2, now);
    GribRecord *recHumid = GetGribRecord(GRB_HUMID_REL, LV_ABOV_GND, 2, now);
    if (recTemp && recHumid) {
      double temp = recTemp->GetInterpolatedValue(lon, lat);
      double humid = recHumid->GetInterpolatedValue(lon, lat);
      if (temp != GRIB_NOTDEF && humid != GRIB_NOTDEF) {
        double a = 17.27;
        double b = 237.7;
        double t = temp - 273.15;
        double rh = humid;
        // if ( t>0 && t<60 && rh>0.01)
        {
          double alpha = a * t / (b + t) + log(rh / 100.0);
          diewpoint = b * alpha / (a - alpha);
          diewpoint += 273.15;
        }
      }
    }
  }
  return diewpoint;
}

//-------------------------------------------------------------------------------
// Lecture complète d'un fichier GRIB
//-------------------------------------------------------------------------------
void GribReader::OpenFile(const wxString fname) {
  grib_debug("Open file: %s", (const char *)fname.mb_str());
  fileName = fname;
  ok = false;
  // clean_all_vectors();
  //--------------------------------------------------------
  // Open the file
  //--------------------------------------------------------
  file = zu_open((const char *)fname.mb_str(), "rb", ZU_COMPRESS_AUTO);
  if (file == nullptr) {
    erreur("Can't open file: %s", (const char *)fname.mb_str());
    return;
  }
  ReadGribFileContent();

  // Look for compressed files with alternate extensions
  if (!ok) {
    if (file != nullptr) zu_close(file);
    file = zu_open((const char *)fname.mb_str(), "rb", ZU_COMPRESS_BZIP);
    if (file != nullptr) ReadGribFileContent();
  }
  if (!ok) {
    if (file != nullptr) zu_close(file);
    file = zu_open((const char *)fname.mb_str(), "rb", ZU_COMPRESS_GZIP);
    if (file != nullptr) ReadGribFileContent();
  }
  if (!ok) {
    if (file != nullptr) zu_close(file);
    file = zu_open((const char *)fname.mb_str(), "rb", ZU_COMPRESS_NONE);
    if (file != nullptr) ReadGribFileContent();
  }
  if (file != nullptr) {
    zu_close(file);
    file = nullptr;
  }
}
