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

#include "X3D_common.h"
#include "level/X3D_level_linetexture.h"
#include "X3D_screen.h"

void x3d_linetexture2d_render_2d(X3D_LineTexture2D* tex, X3D_Vex2D pos, X3D_Color color);

struct X3D_CameraObject;

void x3d_linetexture3d_render(X3D_LineTexture3D* tex, struct X3D_CameraObject* cam, X3D_Vex3D pos, X3D_Color color);

