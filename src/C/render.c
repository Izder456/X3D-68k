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

#include <alloca.h>

#include "X3D_common.h"
#include "X3D_prism.h"
#include "X3D_camera.h"
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "X3D_clip.h"

static void draw_clip_line(int16 x1, int16 y1, int16 x2, int16 y2, X3D_Color color, X3D_RasterRegion* region) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  X3D_Vex2D v1 = { x1, y1 };
  X3D_Vex2D v2 = { x2, y2 };
  
  if(x3d_rasterregion_clip_line(region, &renderman->stack, &v1, &v2)) {
    x3d_screen_draw_line(v1.x, v1.y, v2.x, v2.y, color);
  }
}

void x3d_prism3d_render(X3D_Prism3D* prism, X3D_CameraObject* object, X3D_Color color) {
#if 0
  X3D_Vex2D v[prism->base_v * 2];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(object, &cam_pos);
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    X3D_Vex3D translate = {
      prism->v[i].x - cam_pos.x,
      prism->v[i].y - cam_pos.y,
      prism->v[i].z - cam_pos.z
    };
    
    X3D_Vex3D rot;
    x3d_vex3d_int16_rotate(&rot, &translate, &object->base.mat);
    x3d_vex3d_int16_project(v + i, &rot);
  }
  
  for(i = 0; i < prism->base_v; ++i) {
    uint16 next = (i != prism->base_v - 1 ? i + 1 : 0);
    
    // BASE_A
    draw_clip_line(
      v[i].x,
      v[i].y,
      v[next].x,
      v[next].y,
      color
    );
    
    // BASE_B
    draw_clip_line(
      v[i + prism->base_v].x,
      v[i + prism->base_v].y,
      v[next + prism->base_v].x,
      v[next + prism->base_v].y,
      color
    );
    
    // Connecting lines
    draw_clip_line(
      v[i].x,
      v[i].y,
      v[i + prism->base_v].x,
      v[i + prism->base_v].y,
      color
    );
  }
#endif
}

void x3d_polygon3d_render_wireframe_no_clip(X3D_Polygon3D* poly, X3D_CameraObject* object, X3D_Color color) {
  X3D_Vex2D v[poly->total_v];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(object, &cam_pos);
  
  for(i = 0; i < poly->total_v; ++i) {
    X3D_Vex3D translate = {
      poly->v[i].x - cam_pos.x,
      poly->v[i].y - cam_pos.y,
      poly->v[i].z - cam_pos.z
    };
    
    X3D_Vex3D rot;
    x3d_vex3d_int16_rotate(&rot, &translate, &object->base.mat);
    x3d_vex3d_int16_project(v + i, &rot);
  }
  
  for(i = 0; i < poly->total_v; ++i) {
    uint16 next = (i != poly->total_v - 1 ? i + 1 : 0);
    
    // BASE_A
    x3d_screen_draw_line(
      v[i].x,
      v[i].y,
      v[next].x,
      v[next].y,
      color
    );
  }
}

void x3d_rasterregion_fill(X3D_RasterRegion* region, X3D_Color color) {
  int16 i;
  
  for(i = region->y_range.min; i < region->y_range.max; ++i) {
    uint16 index = i - region->y_range.min;
    x3d_screen_draw_line(region->x_left[index], i, region->x_right[index], i, color);
  }
}

void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region) {
  // Load the segment into the cache
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(id);

  uint16 step = x3d_enginestate_get_step();
  
  if(seg->last_engine_step == step)
    return;
  
  seg->last_engine_step = step;
  
  X3D_Prism3D* prism = &seg->prism;
  X3D_Vex2D v[prism->base_v * 2];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  for(i = 0; i < prism->base_v * 2; ++i) {
    X3D_Vex3D translate = {
      prism->v[i].x - cam_pos.x,
      prism->v[i].y - cam_pos.y,
      prism->v[i].z - cam_pos.z
    };
    
    X3D_Vex3D rot;
    x3d_vex3d_int16_rotate(&rot, &translate, &cam->base.mat);
    x3d_vex3d_int16_project(v + i, &rot);
  }
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    uint16 a, b;
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    draw_clip_line(v[a].x, v[a].y, v[b].x, v[b].y, color, region);
  }
  
  //x3d_prism3d_render(&seg->prism, cam, color);
  
  
  X3D_UncompressedSegmentFace* face = x3d_uncompressedsegment_get_faces(seg);
  
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  void* stack_save = x3d_stack_save(&renderman->stack);
  
  /// @todo It's a waste to calculate this for every edge if we don't need it,
  /// so add a bitmask to the cache to determine which faces actually need
  /// raster edges generated.
  
  uint16 total_e = seg->prism.base_v * 3;
  X3D_RasterEdge edges[total_e];
  
  for(i = 0; i < total_e; ++i) {
    uint16 a, b;
    
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    x3d_rasteredge_generate(&renderman->stack, edges + i, v[a], v[b], region->y_range);
  }

  // Render the connecting segments
  for(i = 0; i < x3d_prism3d_total_f(seg->prism.base_v); ++i) {
    if(face[i].portal_seg_face != X3D_FACE_NONE) {
      // Only render the segment if we're not on the opposite side of the wall
      // with the portal
      int16 dist = x3d_plane_dist(&face[i].plane, &cam_pos);
      
      //printf("Dist: %d\n", dist);
      
      if(dist > 0) {
        void* stack_ptr = x3d_stack_save(&renderman->stack);
        uint16 edge_index[prism->base_v];
        X3D_RasterRegion new_region;

        
        uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, i, edge_index);
        
        if(x3d_rasterregion_construct_from_edges(&new_region, &renderman->stack, edges, edge_index, face_e)) {
          if(x3d_rasterregion_intersect(region, &new_region)) {
            uint16 seg_id = x3d_segfaceid_seg(face[i].portal_seg_face);
           // x3d_rasterregion_fill(&new_region, 0xFFFF);
            x3d_segment_render(seg_id, cam, color * 8, &new_region);
          }
        }
        
        
        
        x3d_stack_restore(&renderman->stack, stack_ptr);
      }
    }
  }
  
  x3d_stack_restore(&renderman->stack, stack_save);
}


void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);
  
  x3d_segment_render(0, cam, color, &x3d_rendermanager_get()->region);
}

