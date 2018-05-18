#ifndef _ANGLE_H_
#define _ANGLE_H_

#include <cassert>
#include <cmath>
#include <limits>
#include <string>

/* Angle is class for safe angles, headings, bearings and courses.

   + The class takes care of overflow and underflow of arithmetic.
   + Degrees and Radians are handled explicitely to avoid confusion.
   + To create angles use factory functions like
     TrueHeading::FromDegrees and MagneticHeading::FromRadians.
   + These constructors *assert* that the value is between [0,360) (and [0,2pi))
     which means that release builds will end up with a NaN Angle, while
     debug builds will abort making debugging easier.
   + Angles can have an empty state. This is implemented internally
     by using a NaN.
   + Currently use internally degrees as OpenCPN does by default,
     but can be changed to radians without any changes to users
     of the class.
 */
class Angle {
    double deg = std::numeric_limits<double>::quiet_NaN();

public:
  constexpr Angle() = default;

  static Angle FromRadians(double rad) {
      return Angle(checkRange(rad*radToDeg));
  }
  static Angle FromDegrees(double deg) {
      return Angle(checkRange(deg));
  }

  constexpr double radians() const {
      return deg * degToRad;
  }
  constexpr double degrees() const {
      return deg;
  }
  constexpr int decimal() const {
      return degrees() + 0.5;
  }
  double sin() const {
      return std::sin(radians());
  }
  double cos() const {
      return std::cos(radians());
  }
  double tan() const {
      return std::tan(radians());
  }

  Angle& operator+=(const Angle& rhs) {
      deg = normDown1(deg + rhs.degrees());
      return *this;
  }

  Angle& operator-=(const Angle& rhs) {
      deg = normUp1(deg- rhs.degrees());
      return *this;
  }

  // valid checks whether this is a valid angle or NaN.
  bool valid() const { return !std::isnan(deg); }

  // reset sets this angle to NaN.
  void reset() { deg = std::numeric_limits<double>::quiet_NaN(); }

  Angle operator+(const Angle& rhs) const {
      auto res = *this;
      res += rhs;
      return res;
  }

  Angle operator-(const Angle& rhs) const {
      auto res = *this;
      res -= rhs;
      return res;
  }

  std::string to_string() {
      char buf[4] = {'-','-','-',0};
      if(valid()) {
          auto d = decimal();
          buf[0] = d%10; d /= 10;
          buf[1] = d%10; d /= 10;
          buf[2] = d%10;
      }
      return {buf, 3};
  }

private:
    // Constructor with degrees private - use static creator functions.
    constexpr Angle(double d) : deg(d) {}

    // sigh, standard does not define pi, there are hacks, but..
    static constexpr double pi = 3.14159265358979323846264338327950288;
    static constexpr double degToRad = pi / 180.0;
    static constexpr double radToDeg = 180.0 / pi;

    // normDeg normalizes something that overflows in any manner. R->[0,360)
    //static constexpr double normDeg(double r) {
    //    return (r >= 0 && r < 360.) ? r : (r - (std::floor(r*(1/360.))*360.));
    //}

    // normDown1 normalizes something between [0,2*360) -> [0,360)
    static constexpr double normDown1(double d) {
        return (d < 360.) ? d : d - 360.;
    }
    // normUp1 handles (-360,360) -> [0,360)
    static constexpr double normUp1(double d) {
        return (d >= 0) ? d : d + 360.;
    }

    // checkRange asserts that the argument is in the range [0,360).
    // returns the argument or NaN.
    static double checkRange(double d) {
        // assert is disabled in release builds.
        // Debug builds abort here, release builds continue with a NaN.
        assert(d >= 0 && d < 360.);
        if(d < 0 || d >= 360.)
            return std::numeric_limits<double>::quiet_NaN();
        return d;
    }
};

using MagneticHeading = Angle;
using TrueHeading = Angle;

#endif // _ANGLE_H_
