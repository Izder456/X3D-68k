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
#include "X3D_screen.h"

uint8 quake_color_palette[256][3] = {
    {   0,   0,   0 },
    {  15,  15,  15 },
    {  31,  31,  31 },
    {  47,  47,  47 },
    {  63,  63,  63 },
    {  75,  75,  75 },
    {  91,  91,  91 },
    { 107, 107, 107 },
    { 123, 123, 123 },
    { 139, 139, 139 },
    { 155, 155, 155 },
    { 171, 171, 171 },
    { 187, 187, 187 },
    { 203, 203, 203 },
    { 219, 219, 219 },
    { 235, 235, 235 },
    {  15,  11,   7 },
    {  23,  15,  11 },
    {  31,  23,  11 },
    {  39,  27,  15 },
    {  47,  35,  19 },
    {  55,  43,  23 },
    {  63,  47,  23 },
    {  75,  55,  27 },
    {  83,  59,  27 },
    {  91,  67,  31 },
    {  99,  75,  31 },
    { 107,  83,  31 },
    { 115,  87,  31 },
    { 123,  95,  35 },
    { 131, 103,  35 },
    { 143, 111,  35 },
    {  11,  11,  15 },
    {  19,  19,  27 },
    {  27,  27,  39 },
    {  39,  39,  51 },
    {  47,  47,  63 },
    {  55,  55,  75 },
    {  63,  63,  87 },
    {  71,  71, 103 },
    {  79,  79, 115 },
    {  91,  91, 127 },
    {  99,  99, 139 },
    { 107, 107, 151 },
    { 115, 115, 163 },
    { 123, 123, 175 },
    { 131, 131, 187 },
    { 139, 139, 203 },
    {   0,   0,   0 },
    {   7,   7,   0 },
    {  11,  11,   0 },
    {  19,  19,   0 },
    {  27,  27,   0 },
    {  35,  35,   0 },
    {  43,  43,   7 },
    {  47,  47,   7 },
    {  55,  55,   7 },
    {  63,  63,   7 },
    {  71,  71,   7 },
    {  75,  75,  11 },
    {  83,  83,  11 },
    {  91,  91,  11 },
    {  99,  99,  11 },
    { 107, 107,  15 },
    {   7,   0,   0 },
    {  15,   0,   0 },
    {  23,   0,   0 },
    {  31,   0,   0 },
    {  39,   0,   0 },
    {  47,   0,   0 },
    {  55,   0,   0 },
    {  63,   0,   0 },
    {  71,   0,   0 },
    {  79,   0,   0 },
    {  87,   0,   0 },
    {  95,   0,   0 },
    { 103,   0,   0 },
    { 111,   0,   0 },
    { 119,   0,   0 },
    { 127,   0,   0 },
    {  19,  19,   0 },
    {  27,  27,   0 },
    {  35,  35,   0 },
    {  47,  43,   0 },
    {  55,  47,   0 },
    {  67,  55,   0 },
    {  75,  59,   7 },
    {  87,  67,   7 },
    {  95,  71,   7 },
    { 107,  75,  11 },
    { 119,  83,  15 },
    { 131,  87,  19 },
    { 139,  91,  19 },
    { 151,  95,  27 },
    { 163,  99,  31 },
    { 175, 103,  35 },
    {  35,  19,   7 },
    {  47,  23,  11 },
    {  59,  31,  15 },
    {  75,  35,  19 },
    {  87,  43,  23 },
    {  99,  47,  31 },
    { 115,  55,  35 },
    { 127,  59,  43 },
    { 143,  67,  51 },
    { 159,  79,  51 },
    { 175,  99,  47 },
    { 191, 119,  47 },
    { 207, 143,  43 },
    { 223, 171,  39 },
    { 239, 203,  31 },
    { 255, 243,  27 },
    {  11,   7,   0 },
    {  27,  19,   0 },
    {  43,  35,  15 },
    {  55,  43,  19 },
    {  71,  51,  27 },
    {  83,  55,  35 },
    {  99,  63,  43 },
    { 111,  71,  51 },
    { 127,  83,  63 },
    { 139,  95,  71 },
    { 155, 107,  83 },
    { 167, 123,  95 },
    { 183, 135, 107 },
    { 195, 147, 123 },
    { 211, 163, 139 },
    { 227, 179, 151 },
    { 171, 139, 163 },
    { 159, 127, 151 },
    { 147, 115, 135 },
    { 139, 103, 123 },
    { 127,  91, 111 },
    { 119,  83,  99 },
    { 107,  75,  87 },
    {  95,  63,  75 },
    {  87,  55,  67 },
    {  75,  47,  55 },
    {  67,  39,  47 },
    {  55,  31,  35 },
    {  43,  23,  27 },
    {  35,  19,  19 },
    {  23,  11,  11 },
    {  15,   7,   7 },
    { 187, 115, 159 },
    { 175, 107, 143 },
    { 163,  95, 131 },
    { 151,  87, 119 },
    { 139,  79, 107 },
    { 127,  75,  95 },
    { 115,  67,  83 },
    { 107,  59,  75 },
    {  95,  51,  63 },
    {  83,  43,  55 },
    {  71,  35,  43 },
    {  59,  31,  35 },
    {  47,  23,  27 },
    {  35,  19,  19 },
    {  23,  11,  11 },
    {  15,   7,   7 },
    { 219, 195, 187 },
    { 203, 179, 167 },
    { 191, 163, 155 },
    { 175, 151, 139 },
    { 163, 135, 123 },
    { 151, 123, 111 },
    { 135, 111,  95 },
    { 123,  99,  83 },
    { 107,  87,  71 },
    {  95,  75,  59 },
    {  83,  63,  51 },
    {  67,  51,  39 },
    {  55,  43,  31 },
    {  39,  31,  23 },
    {  27,  19,  15 },
    {  15,  11,   7 },
    { 111, 131, 123 },
    { 103, 123, 111 },
    {  95, 115, 103 },
    {  87, 107,  95 },
    {  79,  99,  87 },
    {  71,  91,  79 },
    {  63,  83,  71 },
    {  55,  75,  63 },
    {  47,  67,  55 },
    {  43,  59,  47 },
    {  35,  51,  39 },
    {  31,  43,  31 },
    {  23,  35,  23 },
    {  15,  27,  19 },
    {  11,  19,  11 },
    {   7,  11,   7 },
    { 255, 243,  27 },
    { 239, 223,  23 },
    { 219, 203,  19 },
    { 203, 183,  15 },
    { 187, 167,  15 },
    { 171, 151,  11 },
    { 155, 131,   7 },
    { 139, 115,   7 },
    { 123,  99,   7 },
    { 107,  83,   0 },
    {  91,  71,   0 },
    {  75,  55,   0 },
    {  59,  43,   0 },
    {  43,  31,   0 },
    {  27,  15,   0 },
    {  11,   7,   0 },
    {   0,   0, 255 },
    {  11,  11, 239 },
    {  19,  19, 223 },
    {  27,  27, 207 },
    {  35,  35, 191 },
    {  43,  43, 175 },
    {  47,  47, 159 },
    {  47,  47, 143 },
    {  47,  47, 127 },
    {  47,  47, 111 },
    {  47,  47,  95 },
    {  43,  43,  79 },
    {  35,  35,  63 },
    {  27,  27,  47 },
    {  19,  19,  31 },
    {  11,  11,  15 },
    {  43,   0,   0 },
    {  59,   0,   0 },
    {  75,   7,   0 },
    {  95,   7,   0 },
    { 111,  15,   0 },
    { 127,  23,   7 },
    { 147,  31,   7 },
    { 163,  39,  11 },
    { 183,  51,  15 },
    { 195,  75,  27 },
    { 207,  99,  43 },
    { 219, 127,  59 },
    { 227, 151,  79 },
    { 231, 171,  95 },
    { 239, 191, 119 },
    { 247, 211, 139 },
    { 167, 123,  59 },
    { 183, 155,  55 },
    { 199, 195,  55 },
    { 231, 227,  87 },
    { 127, 191, 255 },
    { 171, 231, 255 },
    { 215, 255, 255 },
    { 103,   0,   0 },
    { 139,   0,   0 },
    { 179,   0,   0 },
    { 215,   0,   0 },
    { 255,   0,   0 },
    { 255, 243, 147 },
    { 255, 247, 199 },
    { 255, 255, 255 },
    { 159,  91,  83 }
};

X3D_ColorIndex colormap[256][16];

X3D_ColorIndex x3d_colormap_get_index(X3D_ColorIndex color, uint16 shade) {
    return colormap[color][X3D_MIN(shade, 15)];
}

X3D_ColorIndex x3d_colorindex_from_rgb(uint8 r, uint8 g, uint8 b) {
    uint16 i;
    int16 min_dist = 0x7FFF;
    X3D_ColorIndex min_index = 0;
    
    for(i = 0; i < 256; ++i) {
        uint8 ir, ig, ib;
        
        int16 dist = abs((int16)r - quake_color_palette[i][0]) +
            abs((int16)g - quake_color_palette[i][1]) +
            abs((int16)b - quake_color_palette[i][2]);
            
        if(dist < min_dist) {
            min_dist = dist;
            min_index = i;
        }
    }
    
    return min_index;
}

// Finds the closest color in the palette to the given color
X3D_ColorIndex x3d_color_to_colorindex(X3D_Color c) {
    uint8 r, g, b;
    x3d_color_to_rgb(c, &r, &g, &b);
    
    return x3d_colorindex_from_rgb(r, g, b);
}

X3D_Color x3d_colorindex_to_color(X3D_ColorIndex index) {
    return x3d_platform_screen_colorindex_to_color(index); 
}



void x3d_palette_init(void) {
    x3d_platform_screen_build_color_palette(quake_color_palette);
    
    uint16 i, j;
    for(i = 0; i < 256; ++i) {
        for(j = 0; j < 64; j += 4) {
            colormap[i][j / 4] = x3d_colorindex_from_rgb(
                X3D_MIN(((uint16)quake_color_palette[i][0] * j + 16) / 32, 255),
                X3D_MIN(((uint16)quake_color_palette[i][1] * j + 16) / 32, 255),
                X3D_MIN(((uint16)quake_color_palette[i][2] * j + 16) / 32, 255)
            );
        }
    }
}