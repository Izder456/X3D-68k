// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <cmath>

#include "Trig.hpp"
#include "FixedPoint.hpp"

fp x_sin(fp angle)
{
    static const int sintab[] =
    {                                                                                                                                                                                                             
         0    ,  1608 ,  3215 ,  4821 ,  6423 ,  8022 ,  9616 ,  11204,                                                                                                                                                              
         12785,  14359,  15923,  17479,  19024,  20557,  22078,  23586,                                                                                                                                                              
         25079,  26557,  28020,  29465,  30893,  32302,  33692,  35061,                                                                                                                                                              
         36409,  37736,  39039,  40319,  41575,  42806,  44011,  45189,                                                                                                                                                              
         46340,  47464,  48558,  49624,  50660,  51665,  52639,  53581,                                                                                                                                                              
         54491,  55368,  56212,  57022,  57797,  58538,  59243,  59913,                                                                                                                                                              
         60547,  61144,  61705,  62228,  62714,  63162,  63571,  63943,                                                                                                                                                              
         64276,  64571,  64826,  65043,  65220,  65358,  65457,  65516,                                                                                                                                                              
         65536,  65516,  65457,  65358,  65220,  65043,  64826,  64571,                                                                                                                                                              
         64276,  63943,  63571,  63162,  62714,  62228,  61705,  61144,                                                                                                                                                              
         60547,  59913,  59243,  58538,  57797,  57022,  56212,  55368,                                                                                                                                                              
         54491,  53581,  52639,  51665,  50660,  49624,  48558,  47464,                                                                                                                                                              
         46340,  45189,  44011,  42806,  41575,  40319,  39039,  37736,                                                                                                                                                              
         36409,  35061,  33692,  32302,  30893,  29465,  28020,  26557,                                                                                                                                                              
         25079,  23586,  22078,  20557,  19024,  17479,  15923,  14359,                                                                                                                                                              
         12785,  11204,  9616 ,  8022 ,  6423 ,  4821 ,  3215 ,  1608 ,                                                                                                                                                              
         0    , -1608 , -3215 , -4821 , -6423 , -8022 , -9616 , -11204,                                                                                                                                                              
        -12785, -14359, -15923, -17479, -19024, -20557, -22078, -23586,                                                                                                                                                              
        -25079, -26557, -28020, -29465, -30893, -32302, -33692, -35061,                                                                                                                                                              
        -36409, -37736, -39039, -40319, -41575, -42806, -44011, -45189,                                                                                                                                                              
        -46340, -47464, -48558, -49624, -50660, -51665, -52639, -53581,                                                                                                                                                              
        -54491, -55368, -56212, -57022, -57797, -58538, -59243, -59913,                                                                                                                                                              
        -60547, -61144, -61705, -62228, -62714, -63162, -63571, -63943,                                                                                                                                                              
        -64276, -64571, -64826, -65043, -65220, -65358, -65457, -65516,                                                                                                                                                              
        -65536, -65516, -65457, -65358, -65220, -65043, -64826, -64571,                                                                                                                                                              
        -64276, -63943, -63571, -63162, -62714, -62228, -61705, -61144,                                                                                                                                                              
        -60547, -59913, -59243, -58538, -57797, -57022, -56212, -55368,                                                                                                                                                              
        -54491, -53581, -52639, -51665, -50660, -49624, -48558, -47464,                                                                                                                                                              
        -46340, -45189, -44011, -42806, -41575, -40319, -39039, -37736,                                                                                                                                                              
        -36409, -35061, -33692, -32302, -30893, -29465, -28020, -26557,                                                                                                                                                              
        -25079, -23586, -22078, -20557, -19024, -17479, -15923, -14359,                                                                                                                                                              
        -12785, -11204, -9616 , -8022 , -6423 , -4821 , -3215 , -1608                                                                                                                                                                
    };
    
    fp frac = angle.frac();
    int angleIndex = angle.toInt() & 0xFF;
    int nextAngleIndex = (angleIndex + 1) & 0xFF;
    
    fp a(sintab[angleIndex]);
    fp b(sintab[nextAngleIndex]);
    
    return fp::lerp(a, b, frac).toFp16x16();
}

// TODO: replace with lookup tables
fp x_atan2(fp y, fp x)
{
    return radiansToAngle(fp::fromFloat(atan2(y.toFloat(), x.toFloat())));
}

fp x_acos(fp val)
{
    return radiansToAngle(fp::fromFloat(acos(val.toFloat())));
}

fp x_asin(fp val)
{
    return radiansToAngle(fp::fromFloat(asin(val.toFloat())));
}


