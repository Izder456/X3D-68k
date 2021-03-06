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

#include "geo/Vec3.hpp"
#include "level/BspLevel.hpp"
#include "memory/OldLink.hpp"
#include "memory/BitSet.hpp"

struct X_RayTracer;
struct Entity;

typedef enum X_BoxColliderFlags
{
    X_BOXCOLLIDER_APPLY_GRAVITY = 1,
    X_BOXCOLLIDER_APPLY_FRICTION = 2,
    X_BOXCOLLIDER_ON_GROUND = 4,
    BOXCOLLIDER_IN_PORTAL = 8
} X_BoxColliderFlags;

enum BoxColliderCollisionType
{
    BOXCOLLIDER_COLLISION_NONE,
    BOXCOLLIDER_COLLISION_PORTAL
};

struct BoxColliderCollisionInfo
{
    BoxColliderCollisionType type;

    union
    {
        Portal* hitPortal;
    };
};

struct X_BoxCollider;

void x_boxcollider_init(X_BoxCollider* collider, BoundBox* boundBox, Flags<X_BoxColliderFlags> flags);

class EntityBuilder;

struct X_BoxCollider
{
    X_BoxCollider()
        : standingOnEntity(nullptr)
    {
        static BoundBox box;
        x_boxcollider_init(this, &box, X_BOXCOLLIDER_APPLY_GRAVITY);
    }
    
    bool traceRay(X_RayTracer& tracer);

    Flags<X_BoxColliderFlags> flags;

    BoundBox boundBox;
    int levelCollisionHull;
    Vec3fp velocity;
    Vec3fp* gravity;
    fp bounceCoefficient;
    fp frictionCoefficient;
    fp maxSpeed;
    BoxColliderCollisionInfo collisionInfo;

    Entity* standingOnEntity;
    
    Portal* currentPortal;
    
    OldLink objectsOnModel;  // TODO: this should be deleted
};

static inline bool x_boxcollider_is_on_ground(X_BoxCollider* collider)
{
    return collider->flags.hasFlag(X_BOXCOLLIDER_ON_GROUND);
}

//static inline void x_boxcollider_apply_velocity(X_BoxCollider* collider, X_Vec3_fp16x16*)

