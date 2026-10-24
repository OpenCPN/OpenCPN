/* GPL-2.0-or-later. Cyclic coordinates for chart-safety queries only. */
#ifndef OCPN_CHART_SAFETY_LONGITUDE_H
#define OCPN_CHART_SAFETY_LONGITUDE_H
#include <cmath>

namespace ocpn::chart_safety {
// Keep ordinary inputs bit-for-bit unchanged; only aliases need reduction.
inline double CanonicalLongitude(double longitude) {
  if (longitude >= -180.0 && longitude < 180.0) return longitude;
  double value = std::fmod(longitude, 360.0);
  if (value < -180.0) value += 360.0;
  if (value >= 180.0) value -= 360.0;
  return value;
}
// Integer wrapping avoids rounding a neighbouring tile/cell onto the seam.
inline long CanonicalLongitudeIndex(long index, long half_turn) {
  long value = index % (2 * half_turn);
  if (value < -half_turn) value += 2 * half_turn;
  if (value >= half_turn) value -= 2 * half_turn;
  return value;
}
inline void UnwrapSafetySegment(double& start, double& end) {
  start = CanonicalLongitude(start);
  end = CanonicalLongitude(end);
  if (end - start > 180.0) end -= 360.0;
  if (end - start < -180.0) end += 360.0;
}
}  // namespace ocpn::chart_safety
#endif
