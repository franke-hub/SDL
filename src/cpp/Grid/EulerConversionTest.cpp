//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EulerConversionTest.cpp
//
// Purpose-
//       Quick and dirty Euler conversion test.
//
// Last change date-
//       2020/10/04
//
//----------------------------------------------------------------------------
#define GLM_ENABLE_EXPERIMENTAL     // Wasn't needed before but needed now
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <math.h>

#define PI M_PI
#define RADTODEG(x) ( (x) * 180.0 / PI )
#define DEGTORAD(x) ( (x) * PI / 180.0 )

using namespace std;

static float ROUND3(float v)
{
//  long l = v * 10000.0;
//  if( l >= 0 )
//    l += 5;
//  else
//    l -= 5;
//  l /= 10;
//  return float(l) / 1000.0;

    return round(v*1000.0)/1000.0;
}

static inline std::ostream&
    operator<<(
      std::ostream&    s,
      const glm::quat  v)
{   return s << "<" << v.x << "," << v.y << "," << v.z << "," << v.w << ">";
}

static inline std::ostream&
    operator<<(
      std::ostream&    s,
      const glm::tvec3<float> v)
{   return s << "<" << v.x << "," << v.y << "," << v.z  << ">";
}

static inline void say(glm::quat v) {
//  cout << "QUAT: ";
    cout << "<" << v.x << "," << v.y << "," << v.z << "," << v.w << ">";
}

static inline void say(glm::tvec3<float> v) {
//  cout << "VEC3: ";
    cout << "<" << v.x << "," << v.y << "," << v.z << ">";
}

static inline void say(glm::tvec4<float> v) {
//  cout << "VEC4: ";
    cout << "<" << v.x << "," << v.y << "," << v.z << "," << v.w << ">";
}

int         main( void )
{
    float RotX = 90.f;
    float RotY = 180.f;
    float RotZ = -270.f;

    if ( RotX || RotY || RotZ )
    {
        std::cout << "Init: x= " << RotX << ", y= " << RotY << ", z= " << RotZ << "\n";
        glm::quat Q(glm::tvec3<float>(DEGTORAD( RotX ),
                                      DEGTORAD( RotY ),
                                      DEGTORAD( RotZ )));
// cout << "QUAT: "; say(Q); cout << "\n";
cout << "QUAT: " << Q << "\n";

        glm::tvec3<float> E = glm::eulerAngles(Q);
// cout << " EUL: "; say(E); cout << "\n";
cout << " EUL: " << E << "\n";

        RotX = RADTODEG(E.x);
        RotY = RADTODEG(E.y);
        RotZ = RADTODEG(E.z);

        RotX = ROUND3(RotX);
        RotY = ROUND3(RotY);
        RotZ = ROUND3(RotZ);

        std::cout << "Final: x= " << RotX << ", y= " << RotY << ", z= " << RotZ << "\n";
    }
    return (0);
}
