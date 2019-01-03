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

#include "X_GameObject.h"
#include "render/X_Viewport.h"
#include "physics/X_BoxCollider.h"
#include "memory/X_BitSet.hpp"
#include "level/PotentiallyVisibleSet.hpp"

struct X_EngineContext;
struct X_RenderContext;
struct X_Screen;
struct X_BspLeaf;

enum CameraobjectFlags
{
    CAMERA_OVERRIDE_PVS = 1
};


typedef struct X_CameraObject
{
    void updateFrustum();

    void overrideBspLeaf(int leafId, BspLevel* level);
    
    void updateView();

    X_GameObject base;
    EnumBitSet<CameraobjectFlags> flags;
    Vec3fp position;
    
    Viewport viewport;
    Mat4x4 viewMatrix;
    x_fp16x16 angleX;
    x_fp16x16 angleY;
    struct X_CameraObject* nextInCameraList;
    struct X_BspLeaf* currentLeaf;
    struct X_BspLeaf* lastLeaf;
    DecompressedLeafVisibleSet pvsForCurrentLeaf;
    
    void (*screenResizeCallback)(struct X_CameraObject* cam, struct X_Screen* screen, x_fp16x16 fov);
} X_CameraObject;

X_CameraObject* x_cameraobject_new(struct X_EngineContext* context);
void x_cameraobject_render(X_CameraObject* cam, struct X_RenderContext* renderContext);

static inline Vec3fp x_cameraobject_get_position(X_CameraObject* cam)
{
    return cam->position;
}


