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

#include "Palette.hpp"
#include "math/FixedPoint.hpp"
#include "Texture.hpp"

#define X_ZBUF_FURTHEST_VALUE 0

class Screen;
struct EngineContext;
struct Camera;

struct Font;

struct ScreenEventHandlers
{
    void* userData;         // Pointer to user-defined data passed to screen updates
    void (*displayFrame)(struct Screen* screen, void* userData);
    void (*restartVideo)(struct EngineContext* context, void* userData);
    void (*cleanupVideo)(struct EngineContext* context, void* userData);
    bool (*isValidResolution)(int w, int h);
};



class Screen
{
public:
    Screen(int w, int h, ScreenEventHandlers& handlers, const X_Palette* palette_);
    
    void setPalette(const X_Palette* palette)
    {
        this->palette = palette;
    }
    
    int getW() const
    {
        return canvas.getW();
    }
    
    int getH() const
    {
        return canvas.getH();
    }
    
    void clearZBuf()
    {
        memset(zbuf, X_ZBUF_FURTHEST_VALUE, calculateZBufSize());
    }
    
    Vec2 getCenter()
    {
        return Vec2(getW() / 2, getH() / 2);
    }
    
    void restartVideo(int newW, int newH, fp newFov);
    
    ~Screen()
    {
        x_free(zbuf);
    }
    
    Texture canvas;
    x_fp0x16* zbuf;
    
    const X_Palette* palette;
    ScreenEventHandlers handlers;
    
private:
    int calculateZBufSize() const
    {
        return canvas.totalTexels() * sizeof(short);
    }
};

