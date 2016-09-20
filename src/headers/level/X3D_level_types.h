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

typedef uint16 X3D_LEVEL_VERTEX;
typedef uint16 X3D_LEVEL_SEG;
typedef uint16 X3D_LEVEL_VERTEX_RUN;
typedef uint16 X3D_LEVEL_SEG_FACE_RUN;
typedef uint16 X3D_LEVEL_WALL_SEG;

typedef struct X3D_LevelSegment {
  uint16                 id;
  uint16                 flags;
  uint16                 base_v;
  X3D_LEVEL_VERTEX_RUN   v;
  X3D_LEVEL_SEG_FACE_RUN faces;
} X3D_LevelSegment;

typedef struct X3D_LevelSegFace {
  uint16 connect_face;
  X3D_LEVEL_WALL_SEG wall_seg_start;
  X3D_Plane plane;
} X3D_LevelSegFace;

typedef struct X3D_LevelSegInterface {
  
} X3D_LevelSegInterface;

typedef struct X3D_LevelSegFaceArray {
  uint16            total;
  X3D_LevelSegFace* faces;
} X3D_LevelSegFaceArray;

typedef struct X3D_LevelSegArray {
  uint16        total;
  X3D_LevelSegment* segs;
} X3D_LevelSegArray;

typedef struct X3D_LevelVertexArray {
  uint16     total;
  X3D_Vex3D* v;
} X3D_LevelVertexArray;

typedef struct X3D_LevelVertexRunArray {
  uint16            total;
  X3D_LEVEL_VERTEX* v;
} X3D_LevelVertexRunArray;

typedef struct X3D_LevelWallSegArray {
    uint16              total;
    X3D_LEVEL_WALL_SEG* wall_segs;
} X3D_LevelWallSegArray;

typedef struct X3D_Level {
  X3D_LevelSegArray       segs;
  X3D_LevelVertexArray    v;
  X3D_LevelVertexRunArray runs;
  X3D_LevelSegFaceArray   faces;
  X3D_LevelWallSegArray   wall_segs;
} X3D_Level;

