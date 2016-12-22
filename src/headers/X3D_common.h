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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#if defined(__pc__)
#include <alloca.h>
#elif defined(__68k__)
#include <alloc.h>

#ifdef __x3d_build__

#define _GENERIC_ARCHIVE
#endif
#endif


#define X3D_WORD_ALIGN 8

#define X3D_SWAP(_a, _b) { __typeof__(_a) _temp = (_a); (_a) = (_b); (_b) = _temp; }

#define X3D_MIN(_a, _b) ({ __typeof__(_a) _aa = _a; __typeof__(_b) _bb = _b; _aa < _bb ? _aa : _bb; })
#define X3D_MAX(_a, _b) ({ __typeof__(_a) _aa = _a; __typeof__(_b) _bb = _b; _aa > _bb ? _aa : _bb; })

#define X3D_SIGNOF(_v) ((_v) < 0 ? -1 : ((_v) > 0 ? 1 : 0))

#define X3D_UNITS_PER_FOOT 32
#define X3D_TEXELS_PER_FOOT 16

#define X3D_TEXEL_SCALE ((X3D_TEXELS_PER_FOOT * 256) / X3D_UNITS_PER_FOOT)

#include "X3D_int.h"
#include "X3D_interface.h"
#include "X3D_log.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "geo/X3D_point.h"

typedef struct X3D_Pair {
  int16 val[2];
} X3D_Pair;


static inline int16 x3d_int16_add_wrap(int16 val, int16 add, int16 max) {
  return val + add < max ? val + add : val + add - max;
}

static inline int16 x3d_int16_increment_wrap(int16 val, int16 max) {
    return x3d_int16_add_wrap(val, 1, max);
}

static inline int16 x3d_int16_sub_wrap(int16 val, int16 sub, int16 max) {
  return val - sub >= 0 ? val - sub : val - sub + max;
}

static inline int16 x3d_units_to_texels(int16 units) {    
    return (units * X3D_TEXEL_SCALE) / 256;
}

static inline int16 x3d_texels_to_units(int16 texels) {
    return (texels * 256) / X3D_TEXEL_SCALE;
}

static inline void x3d_strncpy(char* dest, const char* src, size_t buffer_size) {
    strncpy(dest, src, buffer_size);
    dest[buffer_size - 1] = '\0';
}

