/******************************************************************************
* filterobj.h
*
* Project:  Many
* Purpose:  Class that implements single order inifinite-impulse-response filter
* Author:   Transmitterdan
***************************************************************************
*   Copyright (C) 2016                                                    *
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
*/

/**************************************************************************
 * How to use:                                                            *
 *                                                                        *
 * To create a filter object declare on instance of filterobj. There are  *
 * 2 optional parameters to the constructor. The first (Fc) determines    *
 * the filter cutoff frequency. A value of 0.5 is almost no filtering     *
 * and smaller values decrease the cutoff frequency. If you think of the  *
 * filter as being "fast" or "slow" then 0.5 is fastest and smaller values*
 * are "slower". If you truly want no filtering set the Fc paramter to    *
 * 0.0. This will tell the filter to just return each input unfiltered.   *
 * The second parameter (tp) selects whether the underlying filtered      *
 * values represent a linear value (such as speed) or a circular angle    *
 * such as direction. The angle can be either in radians or degrees.      *
 * These values can be changed on the "fly" with the setFC() and setType()*
 * methods.                                                               *
 * The main method is filter() which accepts a new unfiltered value and   *
 * returns a fitered value. To obtain the most recent filter output use   *
 * the get() method. Lesser used methods are getType (returns tp) and     *
 * getFC() (returns FC). The reset() method resets the filter to zero or  *
 * to some other arbitrary value provided as the first argument.          *
 **************************************************************************
 */
#if ! defined( FILTEROBJ_CLASS_HEADER )
#define FILTEROBJ_CLASS_HEADER

// Define filter types
enum
{
    FILTEROBJ_TYPE_LINEAR = 1 << 0,
    FILTEROBJ_TYPE_DEG = 1 << 1,
    FILTEROBJ_TYPE_RAD = 1 << 2
};

class filterobj
{
public:

    filterobj(double fc = 0.5, int tp = FILTEROBJ_TYPE_LINEAR);
    ~filterobj(){};
    double filter(double data); // Return filtered data given new data point
    void reset(double a = 0.0); // Clear filter
    void setFC(double fc = 0.1);// Set cutoff frequency
    void setType(int tp);       // Set type of filter (linear or angle type)
    double getFC(void) const;         // Return cutoff frequency
    int getType(void) const;          // Return type of filter
    double get(void) const;           // Return the current filtered data

protected:

    void unwrapDeg(double deg);
    void unwrapRad(double rad);

private:

    double a0;
    double b1;
    double accum;
    double oldDeg;
    double oldRad;
    int wraps;
    int type;
};

// Operators
inline bool operator==(const filterobj& lhs, const filterobj& rhs){ return (lhs.get() == rhs.get()); }
inline bool operator!=(const filterobj& lhs, const filterobj& rhs){ return !(rhs == lhs); }

inline bool operator<(const filterobj& lhs, const filterobj& rhs){ return (lhs.get() < rhs.get()); }
inline bool operator>(const filterobj& lhs, const filterobj& rhs){ return rhs < lhs; }
inline bool operator<=(const filterobj& lhs, const filterobj& rhs){ return !(lhs > rhs); }
inline bool operator>=(const filterobj& lhs, const filterobj& rhs){ return !(lhs < rhs); }

#endif