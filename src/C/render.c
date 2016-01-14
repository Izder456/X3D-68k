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
#include "X3D_trig.h"


static void draw_clip_line(int16 x1, int16 y1, int16 x2, int16 y2, X3D_Color color, X3D_RasterRegion* region) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  X3D_Vex2D v1 = { x1, y1 };
  X3D_Vex2D v2 = { x2, y2 };
  
  if(x3d_rasterregion_clip_line(region, &renderman->stack, &v1, &v2)) {
    x3d_screen_draw_line(v1.x, v1.y, v2.x, v2.y, color);
  }
}


///////////////////////////////////////////////////////////////////////////////
/// Transforms a list of points so they are relative to a camera and
///   projected onto the screen.
///
/// @param cam      - camera
/// @param v        - list of points to transform
/// @param total_v  - total numeber of points
/// @param dest3d   - where to write 3D points (optional, NULL if not used)
/// @param dest2d   - where to write projected 2D points
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_camera_transform_points(X3D_CameraObject* cam, X3D_Vex3D* v,
        uint16 total_v, X3D_Vex3D* dest3d, X3D_Vex2D* dest2d) {
  
  X3D_Vex3D cam_pos;                // Position of the camera
  x3d_object_pos(cam, &cam_pos);
  
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    // Translate the point so it's relative to the camera
    X3D_Vex3D translate = {
      v[i].x - cam_pos.x,
      v[i].y - cam_pos.y,
      v[i].z - cam_pos.z
    };
    
    // Rotate the point around the origin
    X3D_Vex3D temp_rot;
    x3d_vex3d_int16_rotate(&temp_rot, &translate, &cam->base.mat);
    
    temp_rot.x += cam->shift.x;
    temp_rot.y += cam->shift.y;
    temp_rot.z += cam->shift.z;
    
    // Project it onto the screen
    x3d_vex3d_int16_project(dest2d + i, &temp_rot);
    
    // Store the rotated 3D point, if requested
    if(dest3d) {
      *dest3d = temp_rot;
      ++dest3d;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
/// Fills a raster region.
///
/// @param region - region
/// @param color  - color to the fill the region with
///
/// @return Nothing.
/// @note No clipping is performed (it is assumed to already be clipped).
///////////////////////////////////////////////////////////////////////////////
void x3d_rasterregion_fill(X3D_RasterRegion* region, X3D_Color color) {
  int16 i;
  
  for(i = region->y_range.min; i < region->y_range.max; ++i) {
    uint16 index = i - region->y_range.min;
    x3d_screen_draw_line(region->x_left[index], i, region->x_right[index], i, color);
  }
}

#define MAX_PORTAL_POINTS 10

typedef struct X3D_WallPortal {
  uint16 id;
  X3D_SegFaceID face;
  X3D_Vex3D center;
  uint16 portal_id;
  X3D_Color color;
  X3D_Mat3x3 mat;
  _Bool fill;
  
  struct {
    X3D_Polygon3D portal_poly;
    X3D_Vex3D v[MAX_PORTAL_POINTS];
  };
} X3D_WallPortal;

#define X3D_MAX_WALL_PORTALS 32

// These are part of the temporary wall portal implementation
static X3D_WallPortal wall_portals[32];     // Table of all wall portals

void x3d_wallportals_init() {
  uint16 i;
  
  for(i = 0; i < X3D_MAX_WALL_PORTALS; ++i) {
    wall_portals[i].face = X3D_FACE_NONE;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Adds a new wall portal to a wall.
///
/// @param face       - wall to add portal to
/// @param center     - center of 3D portal on wall {0, 0, 0} means center of
///                       the wall (unused for now).
/// @param portal_id  - wall portal ID this portal is connected to
/// @param poly       - 2D portal shape
/// 
/// @return The ID of the portal.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_wallportal_add(X3D_SegFaceID face, X3D_Vex3D c, uint16 portal_id, X3D_Polygon2D* poly, X3D_Color color) {
  static X3D_WallPortal* wall_portal_ptr = wall_portals;     // Location of next free wall portal
  
  X3D_WallPortal* portal = wall_portal_ptr++;
  
  portal->id = portal - wall_portals;
  portal->face = face;
  //portal->center = center;
  portal->portal_id = portal_id;
  portal->color = color;
  
  uint16 seg_id = x3d_segfaceid_seg(face);
  uint16 face_id = x3d_segfaceid_face(face);
  
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(seg_id);
  
  x3d_assert(seg_id == seg->base.id);
  
  /// @todo Add support for BASE_A and BASE_B face portals
  //x3d_assert(face_id >= 2);
  
  uint16 top_left_v = face_id - 2;
  uint16 bottom_right_v = ((top_left_v + 1) % seg->prism.base_v) + seg->prism.base_v;
  
  // Project the 2D polygon onto the wall
  x3d_polygon2d_to_polygon3d(
    poly,
    &portal->portal_poly,
    &x3d_uncompressedsegment_get_faces(seg)[face_id].plane,
    seg->prism.v + top_left_v,
    seg->prism.v + bottom_right_v,
    &portal->mat
  );
  
  // Move the portal polygon to the center of the wall
  X3D_Polygon3D* wall = alloca(1000);//x3d_polygon3d_size(seg->prism.base_v * 2));
  x3d_prism3d_get_face(&seg->prism, face_id, wall);
  
  // Calculate the center of the wall
  X3D_Vex3D_int16 center = { 0, 0, 0 };
  uint16 i;
  
  for(i = 0; i < wall->total_v; ++i) {
    center.x += wall->v[i].x;
    center.y += wall->v[i].y;
    center.z += wall->v[i].z;
  }
  
  center.x /= wall->total_v;
  center.y /= wall->total_v;
  center.z /= wall->total_v;
  
  printf("Center: %d, %d, %d\n", center.x, center.y, center.z);
  
  portal->center = (X3D_Vex3D) { center.x, center.y, center.z };
  
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    portal->portal_poly.v[i].x += center.x;
    portal->portal_poly.v[i].y += center.y;
    portal->portal_poly.v[i].z += center.z;
  }
  
  return portal->id;
}


///////////////////////////////////////////////////////////////////////////////
/// Gets a list of portals attached to a face.
///
/// @param face - face ID
/// @param dest - where to write portal ID's
///
/// @return The number of portals attached to the given face.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_wall_get_wall_portals(X3D_SegFaceID face, uint16* dest) {
  uint16 total = 0;
  uint16 i;
  
  for(i = 0; i < X3D_MAX_WALL_PORTALS; ++i) {
    if(wall_portals[i].face == face) {
      dest[total++] = i;
    }
  }
  
  return total;
}

void x3d_wallportal_connect(uint16 portal_from, uint16 portal_to) {
  wall_portals[portal_from].portal_id = portal_to;
}

void x3d_wallportal_render(uint16 wall_portal_id, X3D_CameraObject* cam, X3D_RasterRegion* region) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  void* stack_ptr = x3d_stack_save(&renderman->stack);
  
  X3D_WallPortal* portal = wall_portals + wall_portal_id;
  
  X3D_Vex2D v2d[portal->portal_poly.total_v];
  X3D_Vex3D v3d[portal->portal_poly.total_v];
  
  x3d_camera_transform_points(cam, portal->portal_poly.v, portal->portal_poly.total_v,
    v3d, v2d);
  
  uint16 i;
  
  X3D_RasterEdge edges[portal->portal_poly.total_v];
  
  // Construct the portal's raster region, which has to be clipped against the
  // parent raster region
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    uint16 a, b;
    
    a = i;
    b = i + 1 < portal->portal_poly.total_v ? i + 1 : 0;
    
    x3d_rasteredge_generate(&renderman->stack, edges + i, v2d[a], v2d[b], region->y_range);
  }
  
  X3D_RasterRegion clipped_region;
  uint16 edge_index[portal->portal_poly.total_v];
  
  for(i = 0; i < portal->portal_poly.total_v; ++i)
    edge_index[i] = i;
  
  x3d_enginestate_next_step();
  
  X3D_CameraObject new_cam = *cam;
  
  int16 dist = -1000;
  
  
  if(portal->portal_id != 0xFFFF && x3d_rasterregion_construct_from_edges(&clipped_region, &renderman->stack, edges, edge_index, portal->portal_poly.total_v)) {
    X3D_WallPortal* other_side = wall_portals + portal->portal_id;
    
    
    X3D_Mat3x3 other_side_transpose = other_side->mat;
    
    x3d_mat3x3_transpose(&other_side_transpose);
    
    X3D_Mat3x3 temp;
    
    X3D_Mat3x3 portal_mat = portal->mat;
    
    x3d_mat3x3_mul(&temp, &portal_mat, &other_side_transpose);
    
    //new_cam.base.mat = temp;
    
    X3D_Mat3x3 temp2;
    X3D_Mat3x3 rot;
    
    X3D_Vex3D_angle256 angle = { 0, ANG_180, 0 };
    x3d_mat3x3_construct(&rot, &angle);
    
    x3d_mat3x3_mul(&temp2, &rot, &temp);
    
    
    x3d_mat3x3_mul(&new_cam.base.mat, &temp2, &cam->base.mat);
    
    
    X3D_Vex3D cam_pos;
    x3d_object_pos(cam, &cam_pos);
    
    X3D_Vex3D center, c = x3d_vex3d_sub(&other_side->center, &cam_pos);
    
    X3D_Mat3x3 transpose = new_cam.base.mat;
    //x3d_mat3x3_transpose(&transpose);
    
    x3d_vex3d_int16_rotate(&center, &c, &transpose);
    
    
    X3D_Vex3D pcenter, pc = x3d_vex3d_sub(&portal->center, &cam_pos);
    
    x3d_vex3d_int16_rotate(&pcenter, &pc, &cam->base.mat);
    
    new_cam.shift.x = pcenter.x - center.x;
    new_cam.shift.y = pcenter.y - center.y;
    new_cam.shift.z = pcenter.z - center.z;
    
    new_cam.pseduo_pos = other_side->center;
    
    //printf("===================\n");
    
    //new_cam.base.mat.data[2] = -new_cam.base.mat.data[2];
    //new_cam.base.mat.data[5] = -new_cam.base.mat.data[5];
    //new_cam.base.mat.data[8] = -new_cam.base.mat.data[8];
    
    //new_cam.base.mat.data[2] = -new_cam.base.mat.data[2];
    //new_cam.base.mat.data[5] = -new_cam.base.mat.data[5];
    //new_cam.base.mat.data[8] = -new_cam.base.mat.data[8];
    
    //x3d_mat3x3_transpose(&new_cam.base.mat);
    
    //x3d_vex3d_int16_rotate(&new_diff, &diff, &transpose);
    
    //printf("Diff: %d, %d, %d\n", diff.x, diff.y, diff.z);
    //printf("New Diff: %d, %d, %d\n", new_diff.x, new_diff.y, new_diff.z);
    
    
    
#if 0
    
    new_cam.base.base.pos.x = ((int32)other_side->center.x + new_diff.x) * 256;
    new_cam.base.base.pos.y = ((int32)other_side->center.y + new_diff.y) * 256;
    new_cam.base.base.pos.z = ((int32)other_side->center.z + new_diff.z) * 256;
    
    new_cam.base.base.pos.x = ((int32)other_side->center.x - 100) * 256;
    new_cam.base.base.pos.y = ((int32)other_side->center.y) * 256;
    new_cam.base.base.pos.z = ((int32)other_side->center.z + 100) * 256;
    
#endif

    
    if(x3d_rasterregion_intersect(region, &clipped_region)) {
      uint16 seg_id = x3d_segfaceid_seg(other_side->face);
      
      x3d_segment_render(seg_id, &new_cam, 31, region, x3d_enginestate_get_step());
    }
  }
  
  // Draw the portal outline
  for(i = 0; i < portal->portal_poly.total_v; ++i) {
    uint16 a, b;
    X3D_Vex2D va, vb;
    
    a = i;
    b = i + 1 < portal->portal_poly.total_v ? i + 1 : 0;
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v2d + a, v2d + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      draw_clip_line(va.x, va.y, vb.x, vb.y, portal->color, region);
    }
  }
  
  x3d_stack_restore(&renderman->stack, stack_ptr);
  
  //printf("Render %d\n", wall_portal_id);
}

void x3d_segment_render_face_portal(X3D_SegFaceID id, X3D_CameraObject* cam,
        X3D_Color color, X3D_RasterRegion* parent_region, X3D_Polygon3D* portal) {
  X3D_Vex2D v[portal->total_v];
  X3D_Vex3D v3d[portal->total_v];
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  uint16 i;
  for(i = 0; i < portal->total_v; ++i) {
    X3D_Vex3D translate = {
      portal->v[i].x - cam_pos.x,
      portal->v[i].y - cam_pos.y,
      portal->v[i].z - cam_pos.z
    };
    
    x3d_vex3d_int16_rotate(v3d + i, &translate, &cam->base.mat);
    x3d_vex3d_int16_project(v + i, v3d + i);
  }
  
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  void* stack_save = x3d_stack_save(&renderman->stack);
  
  
  uint16 total_e = portal->total_v;
  X3D_RasterEdge edges[total_e];
  
  for(i = 0; i < total_e; ++i) {
    uint16 a, b;
    
    a = i;
    b = i + 1 < total_e ? i + 1 : 0;
    
    x3d_rasteredge_generate(&renderman->stack, edges + i, v[a], v[b], parent_region->y_range);
  }
  
  for(i = 0; i < portal->total_v; ++i) {
    uint16 a, b;
    X3D_Vex2D va, vb;
    
    a = i;
    b = i + 1 < total_e ? i + 1 : 0;
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v + a, v + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      draw_clip_line(va.x, va.y, vb.x, vb.y, color, parent_region);
    }
  }
  
  X3D_CameraObject new_cam = *cam;
  
  //new_cam.base.base.pos.x += 1000L << 8;
  new_cam.base.base.pos.z += 20L << 8;
  
  void* stack_ptr = x3d_stack_save(&renderman->stack);
  uint16 edge_index[portal->total_v];
  X3D_RasterRegion new_region;
  
  for(i = 0; i < portal->total_v; ++i)
    edge_index[i] = i;
  
  if(x3d_rasterregion_construct_from_edges(&new_region, &renderman->stack, edges, edge_index, portal->total_v)) {
    if(x3d_rasterregion_intersect(parent_region, &new_region)) {
      
#if 0
      if(id == 0)
        x3d_rasterregion_fill(&new_region, 0xFFFF);
      else
        x3d_rasterregion_fill(&new_region, 31);
#endif
      
      
      
      
      //x3d_segment_render(id, &new_cam, color * 8, &new_region);
    }
  }
  
  x3d_stack_restore(&renderman->stack, stack_save);
}


void x3d_segment_render(uint16 id, X3D_CameraObject* cam, X3D_Color color, X3D_RasterRegion* region, uint16 step) {
  // Load the segment into the cache
  X3D_UncompressedSegment* seg = x3d_segmentmanager_load(id);
  
  
  if(id == 0) {
    color = x3d_rgb_to_color(0, 0, 255);
  }
  else if (id == 1) {
    color = x3d_rgb_to_color(255, 0, 255);
  }
  else {
    color = 31;
  }
  
  //printf("Step: %d, seg: %d\n", step, seg->last_engine_step);
  
  //if(seg->last_engine_step != step)
  //  return;
  
  seg->last_engine_step = step;
  
  X3D_Prism3D* prism = &seg->prism;
  X3D_Vex2D v[prism->base_v * 2];
  X3D_Vex3D v3d[prism->base_v * 2];
  uint16 i;
  
  X3D_Vex3D cam_pos;
  x3d_object_pos(cam, &cam_pos);
  
  x3d_camera_transform_points(cam, prism->v, prism->base_v * 2, v3d, v);
  
  
  
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
    uint16 portals[32];
    
    uint16 total_p = x3d_wall_get_wall_portals(x3d_segfaceid_create(id, i), portals);
    
    //if(id == 6 && total_p == 0) continue;
    
    if(face[i].portal_seg_face != X3D_FACE_NONE || total_p != 0) {// || (id == 0 && i == 3)) {
      // Only render the segment if we're not on the opposite side of the wall
      // with the portal
      int16 dist = x3d_plane_dist(&face[i].plane, &cam->pseduo_pos);
      
      //printf("Dist: %d\n", dist);
      
      if(dist > 0) {
        if(total_p > 0) {
          uint16 i;
          
          for(i = 0; i < total_p; ++i)
            x3d_wallportal_render(portals[i], cam, region);
        }
        else {
          void* stack_ptr = x3d_stack_save(&renderman->stack);
          uint16 edge_index[prism->base_v];
          X3D_RasterRegion new_region;

          
          uint16 face_e = x3d_prism_face_edge_indexes(prism->base_v, i, edge_index);
          
          if(x3d_rasterregion_construct_from_edges(&new_region, &renderman->stack, edges, edge_index, face_e)) {
            if(x3d_rasterregion_intersect(region, &new_region)) {
              uint16 seg_id = x3d_segfaceid_seg(face[i].portal_seg_face);
              
  #if 0
              if(id == 0)
                x3d_rasterregion_fill(&new_region, 0xFFFF);
              else
                x3d_rasterregion_fill(&new_region, 31);
  #endif
              
              x3d_segment_render(seg_id, cam, color * 8, &new_region, step);
            }
          }
          
          
          
          x3d_stack_restore(&renderman->stack, stack_ptr);
        }
      }
    }
  }
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    uint16 a, b;
    X3D_Vex2D va, vb;
    x3d_prism_get_edge_index(prism->base_v, i, &a, &b);
    
    if(x3d_clip_line_to_near_plane(v3d + a, v3d + b, v + a, v + b, &va, &vb, 10) != EDGE_INVISIBLE) {
      draw_clip_line(va.x, va.y, vb.x, vb.y, color, region);
    }
  }
  
  x3d_stack_restore(&renderman->stack, stack_save);
}


void x3d_render(X3D_CameraObject* cam) {
  X3D_Color color = 31;//x3d_rgb_to_color(0, 0, 255);
  
  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);
  
  x3d_segment_render(0, cam, color, &x3d_rendermanager_get()->region, x3d_enginestate_get_step());
  
  int16 cx = x3d_screenmanager_get()->w / 2;
  int16 cy = x3d_screenmanager_get()->h / 2;
  
  
  x3d_screen_draw_pix(cx, cy - 1, 0xFFFF);
  x3d_screen_draw_pix(cx, cy + 1, 0xFFFF);
  x3d_screen_draw_pix(cx - 1, cy, 0xFFFF);
  x3d_screen_draw_pix(cx + 1, cy, 0xFFFF);
  
  
}

