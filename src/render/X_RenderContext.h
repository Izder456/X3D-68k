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

#include "X_Viewport.h"
#include "object/X_CameraObject.h"
#include "math/X_Mat4x4.h"
#include "X_Screen.h"
#include "level/X_BspLevel.h"

struct X_EngineContext;
struct X_Renderer;

typedef struct X_RenderContext
{
    struct X_Renderer* renderer;
    X_CameraObject* cam;
    X_Screen* screen;
    X_Texture* canvas;
    x_fp0x16* zbuf;
    X_Frustum* viewFrustum;
    X_Mat4x4* viewMatrix;
    struct X_EngineContext* engineContext;
    X_BspLevel* level;
    int currentFrame;
    Vec3 camPos;
} X_RenderContext;

