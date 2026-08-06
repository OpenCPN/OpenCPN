#include <wx/wxprec.h>

#include <cmath>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/math.h>

#include "iir_filter.h"

IirFilter::IirFilter(double fc, int tp) {
  wxASSERT(tp == IIRFILTER_TYPE_DEG || tp == IIRFILTER_TYPE_LINEAR ||
           tp == IIRFILTER_TYPE_RAD);
  SetFc(fc);
  type = tp;
  reset();
}

double IirFilter::Filter(double data) {
  if (!std::isnan(data) && !std::isnan(b1)) {
    if (std::isnan(accum)) accum = 0.0;
    switch (type) {
      case IIRFILTER_TYPE_LINEAR:
        accum = accum * b1 + a0 * data;
        break;

      case IIRFILTER_TYPE_DEG:
        UnwrapDeg(data);
        accum = accum * b1 + a0 * (oldDeg + 360.0 * wraps);
        break;

      case IIRFILTER_TYPE_RAD:
        UnwrapRad(data);
        accum = accum * b1 + a0 * (oldRad + 2.0 * M_PI * wraps);
        break;

      default:
        wxASSERT(false);
    }
  } else
    accum = data;
  return get();
}

void IirFilter::reset(double a) {
  accum = a;
  oldDeg = NAN;
  oldRad = NAN;
  wraps = 0;
}

void IirFilter::SetFc(double fc) {
  if (std::isnan(fc) || fc <= 0.0)
    a0 = b1 = NAN;  // NAN means no filtering will be done
  else {
    reset();
    b1 = exp(-2.0 * 3.1415926535897932384626433832795 * fc);
    a0 = 1 - b1;
  }
}

void IirFilter::SetType(int tp) {
  wxASSERT(tp == IIRFILTER_TYPE_DEG || tp == IIRFILTER_TYPE_LINEAR ||
           tp == IIRFILTER_TYPE_RAD);
  type = tp;
}

double IirFilter::GetFc() const {
  if (std::isnan(b1)) return 0.0;
  double fc = log(b1) / (-2.0 * 3.1415926535897932384626433832795);
  return fc;
}

int IirFilter::GetType() const { return type; }

double IirFilter::get() const {
  if (std::isnan(accum)) return accum;
  double res = accum;
  switch (type) {
    case IIRFILTER_TYPE_DEG:
      while (res < 0) res += 360.0;
      while (res > 360) res -= 360.0;
      break;

    case IIRFILTER_TYPE_RAD:
      while (res < 0) res += 2.0 * M_PI;
      while (res > 2.0 * M_PI) res -= 2.0 * M_PI;
      break;
  }
  return res;
}

void IirFilter::UnwrapDeg(double deg) {
  if (deg - oldDeg > 180) {
    wraps--;
  } else if (deg - oldDeg < -180) {
    wraps++;
  }
  oldDeg = deg;
}

void IirFilter::UnwrapRad(double rad) {
  if (rad - oldRad > M_PI) {
    wraps--;
  } else if (rad - oldRad < M_PI) {
    wraps++;
  }
  oldRad = rad;
}
