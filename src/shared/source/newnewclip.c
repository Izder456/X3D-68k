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
#include "X3D_newnewclip.h"

#include "extgraph.h"

#define ASSERT(_s) {if(!(_s)) x3d_error("Assert failed! %s (%s %d)", #_s, __FUNCTION__, __LINE__);}
#define ASSERT_RANGE(_a, _min, _max) ASSERT(_a >= _min && _a <= _max);

#define ASSERT_EQUAL_INT16(_a, _b) { if((_a) != (_b)) x3d_error("Assert failed! %s == %s (%d, %d)", #_a, #_b, (int16)(_a), (int16)(_b)); }

#ifdef NDEBUG
#undef NDEBUG
#endif

#if 0
#define ENTER() printf("enter %d\n", __LINE__);
#define EXIT() printf("Exit\n");

#else

#define ENTER() ;
#define EXIT() ;

#endif



void* renderstack_save(X3D_RenderStack* stack) {
  return stack->ptr;
}

void renderstack_restore(X3D_RenderStack* stack, void* ptr) {
  stack->ptr = ptr;
}

#define SWAP(_a, _b) { typeof(_a) _temp; _temp = _a; _a = _b; _b = _temp; };

#define ADDRESS(_a) {if((int32)_a & 1) x3d_error("Address: %s\n", #_a);}


void* renderstack_alloc(X3D_RenderStack* stack, uint16 size) {
  
  ADDRESS(stack);
  
  stack->ptr -= size + (size & 1);
  
  ADDRESS((uint8* )stack );
  ADDRESS(stack->ptr);
  ADDRESS((uint8* )stack->base);
  
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

void intersect_line_with_horizontal(fp16x16 slope, Vex2D* start, int16 y) {
  ASSERT((slope >> 16) < 128);    // To prevent overflow when converting to fp8x8 for the slope
  
  int16 dy = y - start->y;
  int16 slope_8x8 = slope >> 8;
  
  start->x = start->x + (((int32)dy * slope) >> 8);
  start->y = y;
}

_Bool clip_rasteredge(X3D_RasterEdge* edge, Vex2D* a, Vex2D* b, fp16x16* slope, X3D_Range region_y_range) {
  edge->flags = 0;
  edge->x_data = NULL;
  
  // Swap points if out of order vertically
  if(a->y > b->y) {
    SWAP(*a, *b);
    edge->flags |= EDGE_DIRECTION_UP;
  }
  
  edge->y_range = get_range(a->y, b->y);
  
  if(edge->y_range.min == edge->y_range.max) {
    edge->flags |= EDGE_HORIZONTAL;
  }
  
  if(!range_overlap(edge->y_range, region_y_range)) {
    edge->flags |= EDGE_INVISIBLE;
    
    return FALSE;
  }
  
  if(!(edge->flags & EDGE_HORIZONTAL)) {         // Only clip visible and non-horizontal edges
    *slope = vertical_slope(*a, *b);
    
    if(a->y < region_y_range.min) {
      intersect_line_with_horizontal(*slope, a, region_y_range.min);
    }
    
    // Clamp the max y
    edge->y_range.max = min(edge->y_range.max, region_y_range.max);
  }
  
  return TRUE;
}

#define EDGE_VALUE(_edge, _y) ((_edge)->x_data[_y - (_edge)->y_range.min])
 
void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, X3D_Range region_y_range) {
  fp16x16 slope;
  
  if(clip_rasteredge(edge, &a, &b, &slope, region_y_range)) {     // Only generate the edge if it's (potentially) visible
    fp16x16 x = ((int32)a.x) << 16;
    int16 y = a.y;
    int16 height = b.y - a.y + 1;
    
    ASSERT(region_y_range.min <= region_y_range.max);
    ASSERT(in_range(SCREEN_Y_RANGE, height - 1));
    ASSERT(in_range(region_y_range, a.y));
    ASSERT(in_range(region_y_range, b.y));
    
    // Allocate space for the values
    edge->x_data = renderstack_alloc(stack, height * 2);
    
    while(y <= b.y) {
      EDGE_VALUE(edge, y) = x >> 16;
      x += slope;
      ++y;
    }
    
    b.x = (x - slope) >> 16;
  }
  
  edge->x_range = get_range(a.x, b.x);
}

void test_newnew_clip() {
  X3D_RenderStack stack;
  renderstack_init(&stack, 2048);
  FontSetSys(F_4x6);
  
  {
    void* stack_ptr = renderstack_save(&stack);
    X3D_Range y_range = SCREEN_Y_RANGE;
    X3D_RasterEdge edge;
    
    Vex2D a = { -20000, -30000 };
    Vex2D b = { 20000, -30000 };
    
    generate_rasteredge(&stack, &edge, a, b, y_range);
    
    printf("Case 0\n");
    ASSERT_EQUAL_INT16((edge.flags & EDGE_INVISIBLE) != 0, TRUE);
    ASSERT(edge.flags & EDGE_HORIZONTAL);
    ASSERT(edge.x_range.min == -20000);
    ASSERT(edge.x_range.max == 20000);
    ASSERT(edge.y_range.min == -30000);
    ASSERT(edge.y_range.max == 30000);
    ASSERT(edge.x_data == NULL);
    
    renderstack_restore(&stack, stack_ptr);
  }
}


























