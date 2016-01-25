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

#include "X3D_common.h"
#include "X3D_vector.h"
#include "X3D_screen.h"
#include "X3D_clip.h"
#include "X3D_plane.h"

enum {
  X3D_PORTAL_REMOTE = 1,
  X3D_PORTAL_DRAW_OUTLINE = 2,
  X3D_PORTAL_FILL = 3
};

typedef struct X3D_Portal {
  uint16 flags;
  uint16 total_v;
  X3D_Vex3D* v;
  
  X3D_Color outline_color;
  X3D_Color fill_color;
  
  X3D_RasterRegion* region;
  X3D_Plane* plane;
} X3D_Portal;


