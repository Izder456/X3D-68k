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

typedef struct X3D_InitSettings {
  int16 screen_w;         ///< Width of the screen
  int16 screen_h;         ///< Height of the screen
  int16 screen_scale;     ///< Screen scaling factor
  _Bool fullscreen;       ///< Fullscreen
} X3D_InitSettings;


void x3d_init(X3D_InitSettings* settings);


X3D_PLATFORM
X3D_INTERNAL
void x3d_platform_init(X3D_InitSettings* settings);