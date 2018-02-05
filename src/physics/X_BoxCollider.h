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

#include "geo/X_Vec3.h"
#include "level/X_BspLevel.h"
#include "memory/X_Link.h"

typedef enum X_BoxColliderFlags
{
    X_BOXCOLLIDER_APPLY_GRAVITY = 1,
    X_BOXCOLLIDER_APPLY_FRICTION = 2,
    X_BOXCOLLIDER_ON_GROUND = 4
} X_BoxColliderFlags;

typedef struct X_BoxCollider
{
    int flags;
    X_BoundBox boundBox;
    int levelCollisionHull;
    X_Vec3 position;
    X_Vec3 velocity;
    X_Vec3* gravity;
    x_fp16x16 bounceCoefficient;
    x_fp16x16 frictionCoefficient;
    x_fp16x16 maxSpeed;
    
    X_Link objectsOnModel;
} X_BoxCollider;

void x_boxcollider_init(X_BoxCollider* collider, X_BoundBox* boundBox, X_BoxColliderFlags flags);
void x_boxcollider_update(X_BoxCollider* collider, X_BspLevel* level);

static inline bool x_boxcollider_is_on_ground(X_BoxCollider* collider)
{
    return collider->flags & X_BOXCOLLIDER_ON_GROUND;
}

//static inline void x_boxcollider_apply_velocity(X_BoxCollider* collider, X_Vec3_fp16x16*)

