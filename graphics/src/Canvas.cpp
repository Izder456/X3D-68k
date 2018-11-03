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

#include "Canvas.hpp"

namespace X3D
{
    void Canvas::drawLine(Vec2 start, Vec2 end, Color color)
    {
        clamp(start);
        clamp(end);

        Vec2i startPos = start.toVec2<int>();
        Vec2i endPos = end.toVec2<int>();

        int dx = endPos.x - startPos.x;
        int dy = endPos.y - startPos.y;

        int sx = startPos.x < endPos.x ? 1 : -1;
        int sy = startPos.y < endPos.y ? 1 : -1;

        int err = (dx > dy ? dx : -dy) / 2;
        
        Vec2i pos = startPos;

        do
        {
            texture->setTexel(pos, color);

            if(pos == endPos)
            {
                break;
            }

            int oldError = err;
            if(oldError > -dx)
            {
                err -= dy;
                pos.x += sx;
            }

            if(oldError < dy)
            {
                err += dx;
                pos.y += sy;
            }
        } while(true);
    }

    void Canvas::fill(Color color)
    {
        memset(texture->getTexels(), color, texture->totalTexels());
    }

    void Canvas::drawPalette(Palette* palette, int x, int y, int colorSize)
    {
        for(int i = 0; i < 16; ++i)
        {
            for(int j = 0; j < 16; ++j)
            {
                int y = i * colorSize;
                int x = j * colorSize;
                Color color = i * 16 + j;
                
                for(int k = 0; k < 8; ++k)
                {
                    for(int d = 0; d < 8; ++d)
                    {
                        texture->setTexel(x + d, y + k, color);
                    }
                }
            }
        }
    }
}