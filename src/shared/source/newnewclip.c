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
#include "X3D_segment.h"
#include "X3D_vector.h"
#include "X3D_render.h"
#include "X3D_trig.h"
#include "X3D_clip.h"
#include "X3D_frustum.h"
#include "X3D_engine.h"
#include "X3D_memory.h"
#include "X3D_newclip.h"

typedef struct X3D_RasterEdge {
  uint8 flags;
  
  int16 min_y;

  int16 max_y;
  int16 max_x;

  int16* x_data;
  int16 min_x;
} X3D_RasterEdge;

enum {
  EDGE_HORIZONTAL = 1,
  EDGE_INVISIBLE = 2
};


typedef struct X3D_RenderStack {
  uint8* ptr;
  uint8* base;
  uint8* end;
} X3D_RenderStack;

#define SWAP(_a, _b) { typeof(_a) _temp; _temp = _a; _a = _b; _b = _temp; };


void* renderstack_alloc(X3D_RenderStack* stack, uint16 size) {
  stack->ptr -= size + (size & 1);
  
  if(stack->ptr < stack->base) {
    x3d_error("Render stack overflow (need %u)", size);
  }
  
  return stack->ptr;
}

void renderstack_init(X3D_RenderStack* stack, uint16 size) {
  stack->base = malloc(size);
  stack->end = stack->base + size;
  stack->ptr = stack->end;
}

void renderstack_cleanup(X3D_RenderStack* stack) {
  free(stack->base);
}

int32 vertical_slope(Vex2D v1, Vex2D v2);

void draw_edge(X3D_RasterEdge* edge) {
  if(edge->flags & EDGE_INVISIBLE)
    return;
  
  
  if(edge->flags & EDGE_HORIZONTAL) {
    FastDrawHLine(LCD_MEM, edge->min_x, edge->max_x, edge->min_y, A_XOR);
  }
  else {
    int16 y = edge->min_y;
    
    while(y <= edge->max_y) {
      DrawPix(edge->x_data[y - edge->min_y], y, A_XOR);
      ++y;
    }
  }
}

void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y) {
  if(a.y > b.y) {
    SWAP(a, b);
  }
  
  if(b.y < min_y || a.y > max_y) {
    edge->flags = EDGE_INVISIBLE;
  }
  else if(a.y == b.y) {
    // Case 1: edge is horizontal
    edge->flags = EDGE_HORIZONTAL;
    
    if(a.x < b.x) {
      edge->min_x = a.x;
      edge->max_x = b.x;
    }
    else {
      edge->min_x = b.x;
      edge->max_x = a.x;
    }
    
    edge->min_y = a.y;
    edge->max_y = a.y;
  }
  else {
    fp16x16 x;
    int16 y;
    fp16x16 slope = vertical_slope(a, b);
    
    if(a.y >= min_y) {
      x = ((fp16x16)a.x) << 16;
      y = a.y;
    }
    else {
      fp8x8 new_slope = slope >> 8;
      
      x = (((int32)a.x << 8) + ((int32)(min_y - a.y) * new_slope)) << 8;
      y = min_y;
    }
    
    int16 end_y = min(max_y, b.y);
    
    edge->min_y = y;
    
    edge->flags = 0;
    
    edge->x_data = renderstack_alloc(stack, (end_y - y + 1) * 2);
    
    while(y <= end_y) {
      edge->x_data[y - edge->min_y] = x >> 16;
      x += slope;
      ++y;
    }
    
    edge->max_y = end_y;
  }
}

typedef struct X3D_RasterRegion {
  int16 min_y;
  int16 max_y;
  
  int16* x_left;
  int16* x_right;
} X3D_RasterRegion;

uint16 find_top_edge(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e) {
  int16 min_index = 0;
  uint16 i;
  
  for(i = 1; i < total_e; ++i) {
    uint16 index = edge_index[i];
    X3D_RasterEdge* edge = raster_edge + index;
    X3D_RasterEdge* min_edge = raster_edge + min_index;
    
    if(edge->min_y < min_edge->min_y) {
      min_index = i;
      printf("Switch to %d\n", i);
    }
    else if(edge->min_y == min_edge->min_y) {
      // Only update the edge if it's not horizontal and it's more left than the current one
      if((edge->flags & EDGE_HORIZONTAL) || (!(min_edge->flags & EDGE_HORIZONTAL) && edge->x_data[1] < min_edge->x_data[1])) {
        min_index = i;
      }
    }
  }
  
  return min_index;
}

uint16 find_bottom_edge(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e) {
  int16 max_index = 0;
  uint16 i;
  
  for(i = 1; i < total_e; ++i) {
    uint16 index = edge_index[i];
    X3D_RasterEdge* edge = raster_edge + index;
    X3D_RasterEdge* max_edge = raster_edge + max_index;
    
    if(edge->max_y > max_edge->max_y) {
      max_index = i;
    }
    else if(edge->max_y == max_edge->max_y) {
      // Only update the edge if it's not horizontal and it's more left than the current one
      if((edge->flags & EDGE_HORIZONTAL) || (!(max_edge->flags & EDGE_HORIZONTAL) && edge->x_data[1] < max_edge->x_data[1])) {
        max_index = i;
      }
    }
  }
  
  return max_index;
}

uint16 next_edge(uint16 edge, uint16 total_e, int16 dir) {
  if(dir < 0) {
    return (edge == 0 ? total_e - 1 : edge - 1);
  }
  else {
    return (edge == total_e - 1 ? 0 : edge + 1);
  }
}

int16* populate_edge(X3D_RasterEdge* edge, int16* dest, _Bool last_edge, _Bool left) {
  
  
  if(edge->flags & EDGE_HORIZONTAL) {
    printf("Horizontal\n");
    if(last_edge) {
      *dest = left ? edge->min_x : edge->max_x;
      
      
      return dest + 1;
    }
    else {
      return dest;
    }
  }
  else {
    int16 count = edge->max_y - edge->min_y + (last_edge ? 1 : 0);
    int16* data = edge->x_data;
    
    while(count-- > 0) {
      *dest = *data;
      
      if(left) {
      }
      else {
        //*dest = 200;
      }
      
      
      ++dest;
      ++data;
      
      
      
    }
    
    return dest;
  }
}

#define EDGE_INDEX() (edge_index[*edge])
#define NEXT_EDGE() (edge_index[edge[1]])

#define EDGE() raster_edge[EDGE_INDEX()]

int16 populate_polyline(X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, uint16 start_edge,
                        int16 dir, uint16 end_edge, int16* dest, _Bool left, int16* start_y) {
  
  int16* start = dest;
  uint16 edge_list[20];
  uint16 total_list = 0;
  
  int16 end;
  
  uint16 index = start_edge;
  
  if(!left && start_edge == end_edge) {
    edge_list[total_list++] = index;
    index = next_edge(index, total_e, dir);
  }
  else {
    //printf("Case b\n");
    while(index != end_edge) {
      edge_list[total_list++] = index;
      index = next_edge(index, total_e, dir);
    }
    //printf("Done\n");
  }
  
  if(left) {
    edge_list[total_list++] = index;
    index = next_edge(index, total_e, dir);
  }
  
  //printf("List of edges:\n");
  
  
  //printf("Dir: %d\n", dir);
  
#if 0
         /\
     0  /  \  2  
       /___ \
          1
  
#endif 
  
  
  uint16 t;
  for(t = 0; t < total_list; ++t) {
    //printf("%d ", edge_list[t]);
  }
  
  //printf("\n");
  //ngetchx();
  
  
  uint16* edge = edge_list;
  uint16* edge_end = edge_list + total_list;
  
#if 1
  //printf("enter A\n");
  while(edge < edge_end && (EDGE().flags & EDGE_INVISIBLE)) {
    //printf("Skip %d\n", (int16)(edge - edge_list));
    ++edge;
  }
  //printf("Exit A\n");
  
  if(edge == edge_end) {
    //printf("return\n");
    return 0;
  }
#endif
  
  *start_y = EDGE().min_y;
  
  //printf("Start: %d\n", *start_y);
  //ngetchx();
  
  _Bool last = FALSE;
    
  do {
    last = edge + 1 == edge_end || (EDGE().flags & EDGE_INVISIBLE);
    //printf("last: %d\n", (int16)last);
    dest = populate_edge(&EDGE(), dest, last, left);
    //printf("Size: %d\n", (int16)(dest - start));
    
    //printf("Edge: %d\n", EDGE_INDEX());
    
    ++edge;
  } while(edge < edge_end && !last);
  
  return dest - start;
}

_Bool clip_span(int16 portal_left, int16 portal_right, int16* span_left, int16* span_right) {
  *span_left = max(portal_left, *span_left);
  *span_right = min(portal_right, *span_right);
  
  return *span_left <= *span_right;
}

#define CLIP() clip_span(*portal_left, *portal_right, region_left, region_right)

void rasterize_rasterregion(X3D_RasterRegion* region) {
  int16 y = region->min_y;

  //printf("Miny: %d, maxy: %d\n", region->min_y, region->max_y);
  
  while(y <= region->max_y) {
    FastDrawHLine(LCD_MEM, region->x_left[y - region->min_y], region->x_right[y - region->min_y], y, A_XOR);
    ++y;
  }
}

_Bool intersect_rasterregion(X3D_RasterRegion* portal, X3D_RasterRegion* region) {
  int16* portal_left = portal->x_left + region->min_y - portal->min_y;
  int16* portal_right = portal->x_right + region->min_y - portal->min_y;
  
  int16* region_left = region->x_left;
  int16* region_right = region->x_right;
  
  //printf("left: %d, right: %d\n", *portal_left, *portal_right);
  //printf("rleft: %d, rright: %d\n", *region_left, *region_right);
  
  int16 y = region->min_y;
  
  // Skip fully clipped spans
  while(y <= region->max_y && !CLIP()) {
    ++y;
    ++portal_left;
    ++portal_right;
    ++region_left;
    ++region_right;
  }
  
  // If all the spans are fully clipped, it's invisible!
  if(y > region->max_y) {
    printf("All invisible!\n");
    return FALSE;
  }
  
  // We need to actually adjust x_left and x_right to point to the first visible span
  region->x_left = region_left;
  region->x_right = region_right;
  
  region->min_y = y;
  
  while(y <= region->max_y && CLIP()) {
    ++y;
    ++portal_left;
    ++portal_right;
    ++region_left;
    ++region_right;
  }
  
  region->max_y = y - 1;
  
  return TRUE;
}

_Bool construct_rasterregion(X3D_RenderStack* stack, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest, int16 min_y, int16 max_y) {
  uint16 top_index = find_top_edge(raster_edge, edge_index, total_e);
  
  X3D_RasterEdge* top = raster_edge + top_index;
  
  //printf("Top index: %d\n", top_index);
  
  uint16 bottom_index = find_bottom_edge(raster_edge, edge_index, total_e);
  
  X3D_RasterEdge* bottom = raster_edge + bottom_index;
  
  //printf("bottom: %d\n", bottom_index);
  
  //printf("Bottom index: %d\n", bottom_index);
  
  int16 left_dir;
  
  X3D_RasterEdge* next_left = raster_edge + next_edge(top_index, total_e, -1);
  X3D_RasterEdge* next_right = raster_edge + next_edge(top_index, total_e, 1);
  
  int16 estimated_height = min(max_y, bottom->max_y) - max(min_y, top->min_y) + 1;
  
  int16* left = renderstack_alloc(stack, estimated_height * 2);
  int16* right = renderstack_alloc(stack, estimated_height * 2);
  
  int16 start_right;
  
  if(top->flags & EDGE_HORIZONTAL) {
    //printf("Horizontal!\n");
    //ngetchx();
    start_right = top_index;
    
    left_dir = (next_left->x_data[1] < next_right->x_data[1] ? -1 : 1);
  }
  else {
    //printf("Xnext left: %d\n", next_edge(top_index, total_e, -1));
    
    if(next_right->min_y == top->min_y) {
      left_dir = -1;
    }
    else {
      left_dir = 1;
    }
    
    //printf("top: %d\n", top_index);
    //printf("leftdir: %d\n", left_dir);
    //printf("end: %d\n", bottom_index);
    
    start_right = next_edge(top_index, total_e, -left_dir);
    
  }
  
  
  int16 start_y;
  
  int16 total_left = populate_polyline(raster_edge, edge_index, total_e, top_index, left_dir, bottom_index, left, TRUE, &start_y);
  
  int16 total_right = populate_polyline(raster_edge, edge_index, total_e, start_right, -left_dir, bottom_index, right, FALSE, &start_y);
  
  if(total_left != total_right) {
    x3d_error("Polgon polyline mismatch (left: %d, right: %d)", total_left, total_right);
  }
  
  dest->x_left = left;
  dest->x_right = right;
  
  dest->min_y = start_y;
  dest->max_y = start_y + total_left - 1;
  
  //printf("start: %d, end %d\n", dest->min_y, dest->max_y);
  //ngetchx();
  
  return dest->max_y >= dest->min_y;
  
#if 0
  int16 i;
  
  for(i = 0; i < total_left; ++i) {
    //DrawPix(left[i], i + top->min_y, A_NORMAL);
    //DrawPix(right[i], i + top->min_y, A_NORMAL);
    
    FastDrawHLine(LCD_MEM, left[i], right[i], i + start_y, A_NORMAL);
  }
#endif
  
}


_Bool construct_and_clip_rasterregion(X3D_RenderStack* stack, X3D_RasterRegion* portal, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest) {  
  return construct_rasterregion(stack, raster_edge, edge_index, total_e, dest, portal->min_y, portal->max_y)
  
    && intersect_rasterregion(portal, dest);
}

_Bool construct_rasterregion_from_points(X3D_RenderStack* stack, X3D_RasterRegion* dest, Vex2D* v, uint16 total_v) {
  X3D_RasterEdge edges[total_v];
  uint16 edge_index[total_v];
  uint16 i;
  
  for(i = 0; i < total_v; ++i) {
    int16 next = (i + 1) % total_v;
    generate_rasteredge(stack, edges + i, v[i], v[next], 0, LCD_HEIGHT - 1);
    
    //printf("es: %d, ee: %d\n", edges[i].min_y, edges[i].max_y);
    
    edge_index[i] = i;
  }
  
  return construct_rasterregion(stack, edges, edge_index, total_v, dest, 0, LCD_HEIGHT - 1);
}






void raster_tri(X3D_RasterEdge* edge_left, X3D_RasterEdge* edge_right) {
  int16 y = edge_left->min_y;
  
  while(y <= edge_left->max_y) {
    FastDrawHLine(LCD_MEM, edge_left->x_data[y - edge_left->min_y], edge_right->x_data[y - edge_left->min_y], y, A_NORMAL);
    ++y;
  }
}

void test_newnew_clip() {
  clrscr();
  
  X3D_RenderStack stack;
  renderstack_init(&stack, 2048);
  
  X3D_RasterEdge edge;
  
  X3D_RasterRegion screen_region;
  
  int16 SIZE = 50;
  
#if 0
  Vex2D screen_v[] = {
    { 0, 0},
    {LCD_WIDTH - 1, 0},
    {LCD_WIDTH - 1, LCD_HEIGHT - 1},
    {0, LCD_HEIGHT - 1}
  };
#else
  
  Vex2D screen_v[] = {
    { 120, 120 },
    { 10, 10 },
    { LCD_WIDTH - 10, 10 }
  };
  
  
#if 0
         /\
     0  /  \  2  
       /___ \
          1
  
#endif 
  
  
#endif
  
  construct_rasterregion_from_points(&stack, &screen_region, screen_v, 3);
  
  
  
  rasterize_rasterregion(&screen_region);
   
  ngetchx();
  
  
  
  
  
  
  Vex2D v[] = {
    { 200, 110 },
    { 228, 70 },
    { 90, 30 },
    { 20, 30 },
    { 120, 110 }
  };
  
  
  uint16 i;
  
  uint16 TOTAL = 5;
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    //DrawLine(v[i].x, v[i].y, v[next].x, v[next].y, A_NORMAL);
  }
  
  X3D_RasterEdge edges[20];
  
  //screen_region.min_y = 50;
  //screen_region.max_y = 78;
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    generate_rasteredge(&stack, edges + i, v[i], v[next], screen_region.min_y, screen_region.max_y);
    
    draw_edge(edges + i);
    ngetchx();
  }
  
  X3D_RasterRegion region;
  
  uint16 edge_index[] =  { 0, 1, 2, 3, 4 };
  
  clrscr();
  
  if(construct_and_clip_rasterregion(&stack, &screen_region, edges, edge_index, 5, &region)) {
    rasterize_rasterregion(&region);
  }
  
  //construct_rasterregion(edges, edge_index, 4, &region);
  

  ngetchx();
  
  return;
  
  clrscr();
  
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 50, A_NORMAL);
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 85, A_NORMAL);
  
  for(i = 0; i < TOTAL; ++i) {
    int16 next = (i + 1) % TOTAL;
    generate_rasteredge(&stack, edges + i, v[i], v[next], 50, 85);
    
    //draw_edge(edges);
    //ngetchx();
  }
  
  //construct_rasterregion(edges, edge_index, 5, &region);
  
  
  
  
  
#if 0
  
  generate_rasteredge(&stack, &edge, a, b, 40, 80);
  //draw_edge(&edge);
  
  X3D_RasterEdge edge2;
  generate_rasteredge(&stack, &edge2, a, c, 40, 80);
  
  raster_tri(&edge, &edge2);
  
  FastDrawHLine(LCD_MEM, 0, LCD_WIDTH - 1, 40, A_NORMAL);
#endif
  
  
  ngetchx();
}






































