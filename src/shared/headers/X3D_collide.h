/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/

#include "X3D_fix.h"
#include "X3D_segment.h"
#include "X3D_vector.h"

#pragma once

_Bool x3d_point_in_segment(X3D_Segment* seg, Vex3D* p);
void x3d_attempt_move_object(X3D_Context* context, void* object, Vex3D_int32* dir);

