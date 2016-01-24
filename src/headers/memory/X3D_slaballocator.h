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

#pragma once

#include "X3D_common.h"
#include "X3D_assert.h"
#include "memory/X3D_stack.h"

//
typedef struct X3D_SlabBlock {
  struct X3D_SlabBlock* next;
} X3D_SlabBlock;

// A freelist with all the blocks of the same size
typedef struct X3D_Slab {
  uint16 size;
  X3D_SlabBlock* head;
  X3D_SlabBlock* tail;
} X3D_Slab;

// Max number of distinct slab sizes in a slab allocator
#define X3D_TOTAL_SLABS 32

///////////////////////////////////////////////////////////////////////////////
/// An allocator suitable for several small allocations (< 512 bytes). Blocks
///   are created on demand from a memory pool and then recycled when they are
///   freed. Note that one a block has been created from the pool, it can
///   never change its size.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SlabAllocator {
  X3D_Stack stack;
  X3D_Slab slabs[X3D_TOTAL_SLABS];
} X3D_SlabAllocator;

void* x3d_slaballocator_alloc(X3D_SlabAllocator* alloc, uint16 size);
void x3d_slaballocator_free(X3D_SlabAllocator* alloc, void* mem);
void x3d_slaballocator_reset(X3D_SlabAllocator* alloc);
void x3d_slaballocator_init(X3D_SlabAllocator* alloc, size_t mem_size);

