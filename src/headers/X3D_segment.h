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
#include "memory/X3D_handle.h"

///< Represents a NULL segment, that is, "no segment"
#define X3D_SEGMENT_NONE 0xFFFF

///< Indicates that a segment has nothing attached to particular face
#define X3D_FACE_NONE 0xFFFF

#define X3D_DOOR_CLOSED 0
#define X3D_DOOR_OPEN   0x7FFF

///////////////////////////////////////////////////////////////////////////////
/// The unique ID of a segment face. The lower 4 bits are the face number
/// and the top 12 bits are the segment ID.
///////////////////////////////////////////////////////////////////////////////
typedef uint16 X3D_SegFaceID;

///////////////////////////////////////////////////////////////////////////////
/// Creates an X3D_SegFaceID from the segment ID and face ID.
///
/// @param seg  - segment ID
/// @param face - face ID
///
/// @return A new X3D_SegFaceID that references the face in seg.
///////////////////////////////////////////////////////////////////////////////
static inline X3D_SegFaceID x3d_segfaceid_create(uint16 seg, uint16 face) {
  return (seg << 4) | face;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the face number from an X3D_SegFaceID.
///
/// @param id - SegFaceID
///
/// @return The face number (0 to base_v + 2, where base_v is the number of
///   vertices in one of the segment's prism bases).
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_segfaceid_face(X3D_SegFaceID id) {
  return id & ((1 << 4) - 1);
}

///////////////////////////////////////////////////////////////////////////////
/// Gets the segment ID from an X3D_SegFaceID
///
/// @param id - SegFaceID
///
/// @return The segment ID.
///////////////////////////////////////////////////////////////////////////////
static inline uint16 x3d_segfaceid_seg(X3D_SegFaceID id) {
  return id >> 4;
}


///////////////////////////////////////////////////////////////////////////////
/// Segment flags.
///////////////////////////////////////////////////////////////////////////////
typedef enum {
  X3D_SEGMENT_DOOR = 1    ///< The segment is a door
} X3D_SegmentFlags;

typedef struct X3D_SegmentFaceAttachement {
  uint16 type;
  uint16 flags;
  void* data;
  struct X3D_SegmentFaceAttachement* next;
} X3D_SegmentFaceAttachement;

enum {
  X3D_ATTACH_WALL_PORTAL
};

enum {
  X3D_DOOR_STOPPED = 0,
  X3D_DOOR_OPENING = 1,
  X3D_DOOR_CLOSING = 2
};

///////////////////////////////////////////////////////////////////////////////
/// Face of an uncompressed segment.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentFace {
  X3D_SegFaceID portal_seg_face;  ///< Face ID that the portal on the face is
                                  ///  connected to
  X3D_Plane plane;                ///< Plane equation of the face
  X3D_SegmentFaceAttachement* attach;
  X3D_Handle texture;
} X3D_SegmentFace;


///////////////////////////////////////////////////////////////////////////////
/// The "base" struct of a segment. All of the derived segments must have this
///   struct as the first member of the struct.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentBase {
  uint16 flags;       ///< Segment flags (see @ref X3D_SegmentFlags).
  uint16 id;          ///< ID of the segment
  uint16 base_v;      ///< Number of vertices in one of the prism bases
} X3D_SegmentBase;

#define X3D_MAX_OBJECTS_IN_SEG 10

typedef struct X3D_SegmentObjectList {
  X3D_Handle objects[X3D_MAX_OBJECTS_IN_SEG];
} X3D_SegmentObjectList;

typedef struct X3D_SegmentDoorData {
  fp0x16 door_open;
  int16 max_open;
  int16 mode;
  fp0x16 open_speed;
} X3D_SegmentDoorData;

///////////////////////////////////////////////////////////////////////////////
/// A segment that is fully decompressed, complete with calculated plane
///   equations for the walls.
///
/// @note This is a two-part variable-sized data structure.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Segment {
  X3D_SegmentBase base;       ///< "Base" segment struct
  uint16 face_offset;         ///< Offset from the beginning of the struct of
                              ///  the face data.
  uint16 last_engine_step;    ///< Last step the segment was rendered
  X3D_SegmentObjectList object_list; ///< List of objects currently in the segment
  
  union {
    X3D_SegmentDoorData* door_data;
  };
  
  X3D_Prism3D prism;          ///< Prism data (MUST BE LAST MEMBER)
} X3D_Segment;




///////////////////////////////////////////////////////////////////////////////
/// Manages all the segments for the engine.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_SegmentManager {
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
  return sizeof(X3D_Segment) + 2 * base_v * sizeof(X3D_Vex3D);
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
    x3d_prism3d_total_f(base_v) * sizeof(X3D_SegmentFace);
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
static inline X3D_SegmentFace* x3d_uncompressedsegment_get_faces(X3D_Segment* seg) {
  return ((void *)seg) + seg->face_offset;
}

static inline _Bool x3d_segment_is_door(X3D_Segment* seg) {
  return seg->base.flags & X3D_SEGMENT_DOOR;
}

X3D_INTERNAL void x3d_segmentmanager_init(uint16 max_segments, uint16 seg_pool_size);
void x3d_segmentmanager_cleanup(void);

X3D_SegmentBase* x3d_segmentmanager_add(uint16 size);
X3D_Segment* x3d_segmentmanager_get_internal(uint16 id);
X3D_Segment* x3d_segmentmanager_load(uint16 id);

void x3d_uncompressedsegment_add_object(uint16 seg_id, X3D_Handle object);
void x3d_segment_point_normal(X3D_Segment* seg, uint16 point, X3D_Vex3D* dest, X3D_Vex3D_int16* face_normal, angle256 angle);
void x3d_segment_reset(X3D_Segment* s);

void x3d_segment_face_attach(uint16 seg_id, uint16 face, uint16 attach_type, void* attach_data, uint16 flags);

