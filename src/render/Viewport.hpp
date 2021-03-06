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

#pragma once

#include "geo/Vec2.hpp"
#include "math/Angle.hpp"
#include "geo/Frustum.hpp"

class Viewport
{
public:
    void init(Vec2 screenPos, int w, int h, fp fieldOfView);
    void updateFrustum(const Vec3fp& camPos, const Vec3fp& forward, const Vec3fp& right, const Vec3fp& up);
    void project(const Vec3fp& src, Vec2_fp16x16& dest);
    void clamp(Vec2& v);
    void clampfp(Vec2_fp16x16& v);

    void projectBisect(const Vec3fp& src, Vec2_fp16x16& dest);

    int closestMipLevelForZ(fp z)
    {
        for(int i = 0; i < 3; ++i)
        {
            if(z < mipDistances[i])
                return i;
        }
        
        return 3;
    }

    Vec2 screenPos;
    int w;
    int h;
    int distToNearPlane;
    fp fieldOfView;
    X_Frustum viewFrustum;
    FrustumPlane viewFrustumPlanes[6];
    fp mipDistances[3];
    
private:
    void initMipDistances();
};

