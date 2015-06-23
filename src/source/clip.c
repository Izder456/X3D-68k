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

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"
#include "X3D_clip.h"
#include "X3D_render.h"
#include "X3D_frustum.h"

typedef struct X3D_ParamLine2D {
  X3D_Vex2D_int16 start;
  X3D_Vex2D_int16 normal;
  int16 d;
} X3D_ParamLine2D;

typedef struct X3D_Prism2D {
  uint32 draw_edges;        ///< Bitfield of which edges should be drawn
  uint16 base_v;            ///< Number of vertices in each base
  X3D_Vex2D_int16 v[0];     ///< Vertices (variable number)
} X3D_Prism2D;


// Parameterizes the line between two points
void x3d_param_line2d(X3D_ParamLine2D* line, X3D_Vex2D_int16* a, X3D_Vex2D_int16* b) {
  int16 dx = b->x - a->x;
  int16 dy = b->y - a->y;
  
  // Calculate the normal for the line
  line->normal = (X3D_Vex2D_int16){ -dy, dx };
  
  // Calculate the distance to the origin
  /// @todo add overflow checking
  line->d = (line->normal.x * a->x + line->normal.y * a->y);
}

typedef struct X3D_ClipRegion {
  uint16 total_pl;
  X3D_ParamLine2D pl[0];
} X3D_ClipRegion;

void x3d_prism2d_clip(X3D_Prism2D* prism, X3D_ClipRegion* clip) {
  // Check each point against each bounding line
  uint16 i, d;
  int16 dist[clip->total_pl][prism->base_v * 2];
  
  for(i = 0; i < clip->total_pl; ++i) {
    for(d = 0; d < prism->base_v * 2; ++d) {
      dist[i][d] = clip->pl[i].normal.x * prism->v[i].x + clip->pl[i].normal.y * prism->v[i].y - clip->pl[i].d;
    }
  }
}

/// @todo document
void x3d_get_fail_planes(X3D_VertexClip* vc, X3D_Vex3D_int16* v, X3D_Frustum* f) {
  uint16 i;

  vc->total_fp = 0;

  for(i = 0; i < f->total_p; ++i) {
    int16 dot = x3d_vex3d_fp0x16_dot(v, &f->p[i].normal);

    // If the point is outside the plane, add the plane to the list
    if(dot < f->p[i].d) {
      vc->fp[vc->total_fp].dot = dot;
      vc->fp[vc->total_fp++].plane_d = f->p[i].d;
    }
  }
}

/// @todo document
void x3d_edge_clip(X3D_Edge* e, X3D_VertexClip* a, X3D_VertexClip* b, X3D_Frustum* f) {
  uint16 i, d;
  X3D_VertexClip* arr[2] = { a, b };
  X3D_VertexClip* v;
  X3D_VertexClip* v_other;

  for(i = 0; i < 2; ++i) {
    v = arr[i];
    v_other = arr[i ^ 1];

    for(d = 0; d < v->total_fp; ++d) {
      //fp8x8 t = div_int16_by_int16_as_fp8x8(v->fp[i].dot - v->fp[i].plane_d, v_other->fp[i]. );
    }
  }
}

void test_clip(X3D_RenderContext* context) {
  X3D_Frustum* frustum = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 10);
  X3D_VertexClip* clip_a = malloc(sizeof(X3D_VertexClip) + sizeof(X3D_FailPlane) * 10);

  X3D_Vex3D_int16 v_a = { 100, 0, 20 };

  x3d_frustum_from_rendercontext(frustum, context);
  x3d_get_fail_planes(clip_a, &v_a, frustum);


  printf("Fail planes:\n");

  uint16 i;
  for(i = 0; i < clip_a->total_fp; ++i) {
    //printf("%u\n", clip_a->fp[i].plane);
  }

  free(frustum);
}

