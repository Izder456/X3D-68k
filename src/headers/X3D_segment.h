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
#include "X3D_prism.h"
#include "memory/X3D_varsizeallocator.h"
#include "X3D_prism.h"
#include "X3D_plane.h"

///< Represents a NULL segment, that is, "no segment"
#define X3D_SEGMENT_NONE 0xFFFF

///< Indicates that a segment has nothing attached to particular face
#define X3D_FACE_NONE 0xFFFF

///< Size of the segment cache
#define X3D_SEGMENT_CACHE_SIZE 32

///< Indicates an invalid location in the segment cache to represnt NULL
#define X3D_SEGMENT_CACHE_NONE 255

///////////////////////////////////////////////////////////////////////////////
/// The unique ID of a segment face. The lower 4 bits are the face numner
/// and the top 12 bits are the segment ID.
///////////////////////////////////////////////////////////////////////////////
typedef uint16 X3D_SegFaceID;


///////////////////////////////////////////////////////////////////////////////
/// Segment flags.
///////////////////////////////////////////////////////////////////////////////
typedef enum {
  X3D_SEGMENT_IN_CACHE = (1 << 15),     ///< Segment has already been loaded
                                        ///  into the cache
  X3D_SEGMENT_UNCOMPRESSED = (1 << 14)  ///< The segment is uncompressed
} X3D_SegmentFlags;


///////////////////////////////////////////////////////////////////////////////
/// Face of an uncompressed segment.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_UncompressedSegmentFace {
  X3D_SegFaceID portal_seg_face;  ///< Face ID that the portal on the face is
                                  ///  connected to
  X3D_Plane plane;                ///< Plane equation of the face
} X3D_UncompressedSegmentFace;


///////////////////////////////////////////////////////////////////////////////
/// The "base" struct of a segment. All of the derived segments must have this
///   struct as the first member of the struct.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentBase {
  uint16 flags;       ///< Segment flags (see @ref X3D_SegmentFlags).
  uint16 id;          ///< ID of the segment
  uint16 base_v;      ///< Number of vertices in one of the prism bases
} X3D_SegmentBase;


///////////////////////////////////////////////////////////////////////////////
/// A segment that is fully decompressed, complete with calculated plane
///   equations for the walls.
///
/// @note This is a two-part variable-sized data structure.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_UncompressedSegment {
  X3D_SegmentBase base;       ///< "Base" segment struct
  uint16 face_offset;         ///< Offset from the beginning of the struct of
                              ///  the face data.
  X3D_Prism3D prism;          ///< Prism data
} X3D_UncompressedSegment;


///////////////////////////////////////////////////////////////////////////////
/// A cache entry for an uncompressed segment.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentCacheEntry {
  uint8 prev;                   ///< The previous most recently used segment
  uint8 next;                   ///< The next most recently used segment
  X3D_UncompressedSegment seg;  ///< The uncompressed segment
} X3D_SegmentCacheEntry;


///////////////////////////////////////////////////////////////////////////////
/// A cache for storing recently accessed segments to improve performance and
///   reduce memory usage. Most segments are stored in a compressed format,
///   and only the ones that are currently being used need to be decompressed.
///   Whenever a new segment is needed, the least-recently used segment is
///   removed from the cache and replaced by the new segment.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentCache {
  X3D_SegmentCacheEntry entry[X3D_SEGMENT_CACHE_SIZE];
  uint16 lru_head;  ///< Index of the least recently used (LRU) cache entry
  uint16 lru_tail;  ///< Most-recently used cache entry
} X3D_SegmentCache;


///////////////////////////////////////////////////////////////////////////////
/// Manages all the segments for the engine.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentManager {
  X3D_SegmentCache cache;       ///< Uncompressed segments currently loaded into
                                ///  the cache.
  X3D_VarSizeAllocator alloc;   ///< Variable sized allocator for allocating
                                ///  segments.
} X3D_SegmentManager;



///////////////////////////////////////////////////////////////////////////////
/// Calculates the offset from the beginning of a segment of where the face
///   data is stored.
///
/// @param base_v - the number of vertices in the base
///
/// @return The offset, in bytes.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_uncompressedsegment_face_offset(uint16 base_v) {
  return sizeof(X3D_UncompressedSegment) + 2 * base_v * sizeof(X3D_Vex3D);
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the size needed to store an uncompressed segment with the
///   given number of vertices in one of the prism's bases.
///
/// @param base_v - number of vertices in the prism base
///
/// @return Size in bytes.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_uncompressedsegment_size(uint16 base_v) {
  return x3d_uncompressedsegment_face_offset(base_v) +
    x3d_prism3d_total_f(base_v) * sizeof(X3D_UncompressedSegmentFace);
}

///////////////////////////////////////////////////////////////////////////////
/// Returns a pointer to the face data for a segment. This is not directly
///   accessible because the data structure is variably sized, based on the
///   number of vertices in the prism base and the number of faces.
///
/// @param seg  - uncompressed segment
///
/// @return The address of the face data.
///////////////////////////////////////////////////////////////////////////////
static inline X3D_UncompressedSegmentFace* x3d_uncompressedsegment_get_faces(X3D_UncompressedSegment* seg) {
  return ((void *)seg) + seg->face_offset;
}

X3D_INTERNAL void x3d_segmentmanager_init(uint16 max_segments, uint16 seg_pool_size);
X3D_SegmentBase* x3d_segmentmanager_add(uint16 size);
X3D_INTERNAL X3D_SegmentBase* x3d_segmentmanager_get_internal(uint16 id);
X3D_UncompressedSegment* x3d_segmentmanager_load(uint16 id);

