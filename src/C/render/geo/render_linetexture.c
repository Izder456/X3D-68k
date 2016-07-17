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
#include "level/X3D_level_linetexture.h"
#include "X3D_screen.h"
#include "X3D_camera.h"
#include "geo/X3D_line.h"
#include "render/geo/X3D_render_line.h"

void x3d_linetexture2d_render_2d(X3D_LineTexture2D* tex, X3D_Vex2D pos, X3D_Color color) {
  X3D_Vex2D v[tex->total_v];
  
  uint16 i;
  for(i = 0; i < tex->total_v; ++i) {
    v[i].x = tex->v[i].x + pos.x;
    v[i].y = tex->v[i].y + pos.y;
  }
  
  for(i = 0; i < tex->total_e; ++i) {
    X3D_Vex2D* v0 = v + tex->edges[i].val[0];
    X3D_Vex2D* v1 = v + tex->edges[i].val[1];
    
    x3d_screen_draw_line(v0->x, v0->y, v1->x, v1->y, color);
  }
}


void x3d_linetexture3d_render(X3D_LineTexture3D* tex, X3D_CameraObject* cam, X3D_Vex3D pos, X3D_Color color) {
  X3D_Vex3D v[tex->total_v];
  X3D_Vex3D rotated_v[tex->total_v];
  
  uint16 i;
  for(i = 0; i < tex->total_v; ++i) {
    v[i] = x3d_vex3d_add(tex->v + i, &pos);
  }
  
  //x3d_camera_transform_points(cam, v, tex->total_v, rotated_v, NULL);
  
  for(i = 0; i < tex->total_e; ++i) {
    X3D_Ray3D ray;
    
    ray.v[0] = v[tex->edges[i].val[0]];
    ray.v[1] = v[tex->edges[i].val[1]];
    
    x3d_ray3d_render(&ray, cam, color);
  }
}

