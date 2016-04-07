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

typedef struct X3D_ScanVertex {
  
} X3D_PolygonVertex;

typedef struct X3D_ScanPolygon {
  uint16 total_v;
  X3D_ScanVertex* v;
} X3D_ScanPolygon;



typedef struct X3D_ScanLine {
  int16 left;
  int16 right;
} X3D_ScanLine;

typedef struct X3D_ScanRegion {
  X3D_ScanLine* scans;
  X3D_BoundingRect rect;  
} X3D_ScanRegion;

