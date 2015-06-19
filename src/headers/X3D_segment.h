// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"

//=============================================================================
// Forward declarations
//=============================================================================
struct X3D_RenderContext;
//=============================================================================
// Structures
//=============================================================================

/// @todo document
typedef struct X3D_Segment {
  uint16 base_v;
  uint16 face_offset;
  X3D_Vex3D_int16 v[0];
} X3D_Segment;

typedef struct X3D_SegmentFace {
  uint16 connect_id;
} X3D_SegmentFace;


//=============================================================================
// Function declarations
//=============================================================================
void x3d_prism_construct(X3D_Prism* s, uint16 steps, uint16 r, int16 h, X3D_Vex3D_angle256 rot_angle);
void x3d_prism_render(X3D_Prism* prism, struct X3D_RenderContext* context);

//=============================================================================
// Static inline functions
//=============================================================================

/// @todo document
static inline X3D_Vex3D_int16* x3d_segment_get_v(X3D_Segment* s) {
  return s->v;
}

/// @todo document
static inline X3D_SegmentFace* x3d_segment_get_face(X3D_Segment* s) {
  return (X3D_SegmentFace *)((uint8 *)s + s->face_offset);
}

/// @todo document
static inline uint16 x3d_segment_total_v(X3D_Segment* s) {
  return s->base_v << 1;
}

