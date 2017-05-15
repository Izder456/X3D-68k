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

#include "X_Texture.h"
#include "memory/X_alloc.h"

#define X_ZBUF_FURTHEST_VALUE 0

////////////////////////////////////////////////////////////////////////////////
/// A z-buffered canvas for drawing.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_Canvas
{
    X_Texture tex;      ///< Internal texture everything gets drawn to
    short* zbuf;        ///< Z-buffer
} X_Canvas;

////////////////////////////////////////////////////////////////////////////////
/// Returns the width of a canvas.
////////////////////////////////////////////////////////////////////////////////
static inline int x_canvas_w(const X_Canvas* canvas)
{
    return x_texture_w(&canvas->tex);
}

////////////////////////////////////////////////////////////////////////////////
/// Returns the height of a canvas.
////////////////////////////////////////////////////////////////////////////////
static inline int x_canvas_h(const X_Canvas* canvas)
{
    return x_texture_h(&canvas->tex);
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the total numbers of texels in a canvas.
////////////////////////////////////////////////////////////////////////////////
static inline int x_canvas_total_texels(const X_Canvas* canvas)
{
    return x_canvas_w(canvas) * x_canvas_h(canvas);
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the size of the zbuffer (in bytes).
////////////////////////////////////////////////////////////////////////////////
static inline size_t x_canvas_zbuf_size(const X_Canvas* canvas)
{
    return x_canvas_total_texels(canvas) * sizeof(short);
}

////////////////////////////////////////////////////////////////////////////////
/// Initializes the canvas with the given dimensions.
///
/// @note This allocates memory for the internal texture and zbuffer. Make sure
///     to call @ref x_canvas_cleanup() when done to free these up!
/// @note The z-buffer and the canvas texels are not initialized by default. Use
///     @ref x_canvas_zbuf_clear() and @ref x_canvas_fill() to do so.
////////////////////////////////////////////////////////////////////////////////
static inline void x_canvas_init(X_Canvas* canvas, int w, int h)
{
    x_texture_init(&canvas->tex, w, h);
    canvas->zbuf = x_malloc(x_canvas_zbuf_size(canvas));
}

////////////////////////////////////////////////////////////////////////////////
/// Releases the resources for a canvas. Make sure to call this when you are
///     done with a canvas!
////////////////////////////////////////////////////////////////////////////////
static inline void x_canvas_cleanup(X_Canvas* canvas)
{
    x_texture_cleanup(&canvas->tex);
    x_free(canvas->zbuf);
    canvas->zbuf = NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Clears the z-buffer, which means setting each depth value to the furthest
///     away value. That way, any new draws will replace those values.
////////////////////////////////////////////////////////////////////////////////
static inline void x_canvas_zbuf_clear(X_Canvas* canvas)
{
    memset(canvas->tex.texels, X_ZBUF_FURTHEST_VALUE, x_canvas_zbuf_size(canvas));
}

////////////////////////////////////////////////////////////////////////////////
/// Fills a canvas with a solid color.
///
/// @note This function ignores the z-buffer.
////////////////////////////////////////////////////////////////////////////////
static inline void x_canvas_fill(X_Canvas* canvas, X_Color clear_color)
{
    if(sizeof(X_Color) == 1)
    {
        memset(canvas->tex.texels, x_texture_total_texels(&canvas->tex), clear_color);
    }
    else
    {
        for(int i = 0; i < x_texture_total_texels(&canvas->tex); ++i)
            canvas->tex.texels[i] = clear_color;
    }
}


void x_canvas_draw_line(X_Texture* tex, X_Vec2 start, X_Vec2 end, X_Color color);

