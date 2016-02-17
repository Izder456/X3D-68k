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
#include "X3D_portal.h"

void x3d_portal_render(X3D_Portal* portal) {
  //if(x3d_portal_fill(portal))
  //  x3d_rasterregion_fill(portal->region, x3d_portal_fill_color(portal));

  if(x3d_portal_outline(portal)) {
    /// @todo Implement portal outlining
  }
}
