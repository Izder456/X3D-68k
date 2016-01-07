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
#include "X3D_segment.h"
#include "X3D_enginestate.h"
#include "memory/X3D_varsizeallocator.h"
#include "X3D_prism.h"

///////////////////////////////////////////////////////////////////////////////
/// Initializes the segment manager.
///
/// @param max_segments   - maximum number of segments
/// @param seg_pool_size  - memory to set aside for allocating segments
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL void x3d_segmentmanager_init(uint16 max_segments, uint16 seg_pool_size) {
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  
  x3d_log(X3D_INFO, "Segment manager init");
  
  x3d_varsizeallocator_init(&seg_manager->alloc, max_segments, seg_pool_size);
}

///////////////////////////////////////////////////////////////////////////////
/// Creates a new segment.
///////////////////////////////////////////////////////////////////////////////
X3D_SegmentBase* x3d_segmentmanager_add(uint16 size) {
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  uint16 id;
  X3D_SegmentBase* seg = x3d_varsizeallocator_alloc(&seg_manager->alloc, size, &id);
  
  seg->id = id;
  seg->flags = 0;
  
  return seg;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets a pointer to the interal representation of a segment.
///
/// @param id - ID of the segment to get
///
/// @return The addess of the interal segment representation.
///////////////////////////////////////////////////////////////////////////////
X3D_INTERNAL X3D_SegmentBase* x3d_segmentmanager_get_internal(uint16 id) {
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  
  return x3d_varsizeallocator_get(&seg_manager->alloc, id);
}


///////////////////////////////////////////////////////////////////////////////
/// Decompresses a segment and loads it into cache, if it's not already there.
///
/// @param id - id of the segment to load
///
/// @return The address of the uncompressed segment.
/// @note This address is only valid while the segment remains in cache!
/// @todo Implemented some sort of locking mechinism?
/// @todo Implement LRU strategy for deciding which block should be replaced.
///////////////////////////////////////////////////////////////////////////////
X3D_UncompressedSegment* x3d_segmentmanager_load(uint16 id) {
  X3D_SegmentBase* seg = x3d_segmentmanager_get_internal(id);
  X3D_SegmentManager* seg_manager = x3d_segmentmanager_get();
  
  // Currently only support uncompressed segments
  x3d_assert(seg->flags & X3D_SEGMENT_UNCOMPRESSED);
  
  // The segment is already loaded into the cache
  if(seg->flags & X3D_SEGMENT_IN_CACHE) {
    uint16 cache_entry = seg->flags & (X3D_SEGMENT_CACHE_SIZE - 1);
    
    X3D_SegmentCacheEntry* entry = &seg_manager->cache.entry[cache_entry];
    
    /// @todo Move the segment back to the tail of the queue (for the LRU
    ///   strategy).
    return &entry->seg;
  }
  
  uint16 i;
  for(i = 0; i < X3D_SEGMENT_CACHE_SIZE; ++i) {
    if(seg_manager->cache.entry[i].seg.base.id == X3D_SEGMENT_NONE) {
      seg_manager->cache.entry[i].seg = *((X3D_UncompressedSegment *)seg);
      return &seg_manager->cache.entry[i].seg;
    }
  }
  
  x3d_assert(!"Out of segments!");
  return NULL;
}











