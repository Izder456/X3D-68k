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
#include "X3D_prism.h"
#include "X3D_object.h"
#include "X3D_camera.h"
#include "X3D_screen.h"

void x3d_prism3d_render(X3D_Prism3D* prism, X3D_CameraObject* object, X3D_Color color);
void x3d_polygon3d_render_wireframe_no_clip(X3D_Polygon3D* poly, X3D_CameraObject* object, X3D_Color color);
void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color);
