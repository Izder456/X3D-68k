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

#include <X3D/X3D.h>

enum class PlayerKeys
{
    forward = 1,
    backward = 2,
    strafeLeft = 4,
    strafeRight = 8,
    jump = 16,
    lookUp = 32,
    lookDown = 64,
    lookLeft = 128,
    lookRight = 256
};

using PlayerKeyFlags = EnumBitSet<PlayerKeys>;

class Player
{
public:
    Player()
    {
        // FIXME: need actual bound box
        static BoundBox box;
        x_boxcollider_init(&collider, &box, X_BOXCOLLIDER_APPLY_GRAVITY);
    }
    
    X_CameraObject& getCamera()
    {
        return camera;
    }
    
    X_BoxCollider collider;
    X_CameraObject camera;
    
    fp angleX;
    fp angleY;
    
private:
};