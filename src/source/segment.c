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
#include "X3D_segment.h"
#include "X3D_trig.h"
#include "X3D_render.h"
#include "X3D_matrix.h"
#include "X3D_geo.h"

/**
* Constructs a prism with regular polygons as the base.
*
* @param s          - pointer to the dest prism
* @param steps      - number of "steps" i.e. points on the polygon base
* @param r          - radius of the base
* @param h          - height of the rism
* @param rot_angle  - angles of rotation around its origin
*
* @return nothing
* @note @ref X3D_Prism is a variable-sized data structure. Make sure s is
*     at least sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * steps * 2 bytes big!
*/
void x3d_prism_construct(X3D_Prism* s, uint16 steps, uint16 r, int16 h, X3D_Vex3D_angle256 rot_angle) {
  ufp8x8 angle = 0;
  ufp8x8 angle_step = 65536L / steps;
  uint16 i;

  s->base_v = steps;

  // Construct the first base (a regular polygon)
  for(i = 0; i < steps; ++i) {
    s->v[i].x = mul_fp0x16_by_int16_as_int16(x3d_cosfp(uint16_upper(angle)), r);
    s->v[i].z = mul_fp0x16_by_int16_as_int16(x3d_sinfp(uint16_upper(angle)), r);
    s->v[i].y = -h / 2;

    angle += angle_step;
  }

  // Construct the second base perpendicular to the first
  for(i = 0; i < steps; ++i) {
    s->v[i + steps].x = s->v[i].x;
    s->v[i + steps].z = s->v[i].z;
    s->v[i + steps].y = h / 2;
  }

  // Rotate the prism around its center
  X3D_Mat3x3_fp0x16 mat;
  x3d_mat3x3_fp0x16_construct(&mat, &rot_angle);

  for(i = 0; i < steps * 2; ++i) {
    X3D_Vex3D_int16 rot;

    x3d_vex3d_int16_rotate(&rot, &s->v[i], &mat);
    s->v[i] = rot;
    s->v[i].z += 200;
  }

  s->draw_edges = 0xFFFFFFFF;
}

/**
* Renders a prism in wireframe.
*
* @param prism - pointer to the prism
* @param context - render context to render to
*
* @return nothing
* @note No clipping is performed, nor is the position of the camera taken into
*     account when rendering.
* @todo Add clipping and take into position of the camera.
*/
void x3d_prism_render(X3D_Prism* prism, X3D_RenderContext* context) {
  uint16 i, d;
  X3D_Vex2D_int16 screen[prism->base_v * 2];

  // Project all of the points on the screen
  for(i = 0; i < prism->base_v * 2; ++i) {
    x3d_vex3d_int16_project(&screen[i], &prism->v[i], context);
  }

  // Draw the prism bases
  /// @todo Rewrite to not use modulus

  uint32 edges = prism->draw_edges;

  for(i = 0; i < 2; ++i) {
    uint16 start = prism->base_v * i;

    for(d = 0; d < prism->base_v; ++d) {
      if(edges & 1) {
        uint16 v = start + d;
        uint16 next = start + ((start + d + 1) % prism->base_v);

        x3d_draw_line_black(context, screen[v], screen[next]);
      }

      edges >>= 1;
    }
  }

  // Draw the connecting lines between the bases
  for(i = 0; i < prism->base_v; ++i) {
    if(edges & 1) {
      x3d_draw_line_black(context, screen[i], screen[i + prism->base_v]);
    }

    edges >>= 1;
  }
}

