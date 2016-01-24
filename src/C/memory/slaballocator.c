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
#include "X3D_assert.h"
#include "memory/X3D_stack.h"
#include "memory/X3D_slaballocator.h"

/// @todo Document file.

static uint16 x3d_slab_id_from_size(X3D_SlabAllocator* alloc, uint16 size) {
  return size >> 4;
}

void* x3d_slaballocator_alloc(X3D_SlabAllocator* alloc, uint16 size) {
  uint16 slab_id = x3d_slab_id_from_size(alloc, size);
  void* block;
  
  x3d_assert(slab_id < X3D_TOTAL_SLABS);
  
  // Check if a block with the size ID is available
  if(alloc->slabs[slab_id].head) {
    block = alloc->slabs[slab_id].head;
    
    // Remove the block from the freelist
    alloc->slabs[slab_id].head = alloc->slabs[slab_id].head->next;
    
    x3d_log(X3D_INFO, "Grabbed existing block\n");
  }
  else {
    // No block is available, so create a new one. We need an extra 2 bytes
    // to record the size ID of the block.
    block = x3d_stack_alloc(&alloc->stack, alloc->slabs[slab_id].size + 2);
    
    x3d_log(X3D_INFO, "Created new block\n");
  }
  
  // Size ID comes 2 bytes before the block that is returned to the user
  *((uint16 *)block) = slab_id;
    
  return block + 2;
}

void x3d_slaballocator_free(X3D_SlabAllocator* alloc, void* mem) {
  X3D_SlabBlock* block = mem - 2;
  
  // Slab ID comes 2 bytes before mem
  uint16 slab_id = *((uint16 *)block);
  
  block->next = NULL;
  
  // Add the block back to the slab's freelist
  if(alloc->slabs[slab_id].head) {
    alloc->slabs[slab_id].tail->next = block;
    alloc->slabs[slab_id].tail = block;
  }
  else {
    alloc->slabs[slab_id].head = block;
    alloc->slabs[slab_id].tail = block;
  }
}

void x3d_slaballocator_reset(X3D_SlabAllocator* alloc) {
  x3d_stack_reset(&alloc->stack);
  
  uint16 i;
  for(i = 0; i < X3D_TOTAL_SLABS; ++i) {
    alloc->slabs[i].head = NULL;
    alloc->slabs[i].tail = NULL;
    alloc->slabs[i].size = (i + 1) * 16;
  }
}

void x3d_slaballocator_init(X3D_SlabAllocator* alloc, size_t mem_size) {
  void* mem = malloc(mem_size);
  
  x3d_stack_init(&alloc->stack, mem, mem_size);
  x3d_slaballocator_reset(alloc);
}

