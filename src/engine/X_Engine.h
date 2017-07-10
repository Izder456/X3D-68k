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

#include "X_EngineContext.h"

X_EngineContext* x_engine_init(int screenW, int screenH, const char* programPath);
void x_engine_cleanup(void);

static inline _Bool x_engine_level_is_loaded(const X_EngineContext* context)
{
    return x_bsplevel_file_is_loaded(&context->currentLevel);
}

static inline X_BspLevel* x_engine_get_current_level(X_EngineContext* context)
{
    return &context->currentLevel;
}

static inline void x_enginecontext_begin_frame(X_EngineContext* context)
{
    ++context->frameCount;
}

