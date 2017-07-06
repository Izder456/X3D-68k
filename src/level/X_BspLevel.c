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

#include "X_BspLevel.h"
#include "X_BspLevelLoader.h"
#include "render/X_RenderContext.h"
#include "geo/X_Ray3.h"

void x_bsplevel_render_wireframe(X_BspLevel* level, X_RenderContext* rcontext, X_Color color)
{
    for(int i = 0; i < level->totalEdges; ++i)
    {
        X_BspEdge* edge = level->edges + i;
        
        X_Ray3 ray = x_ray3_make
        (
            level->vertices[edge->v[0]].v,
            level->vertices[edge->v[1]].v
        );
        
        x_ray3d_render(&ray, rcontext, color);
    }
}

static int x_bsplevel_find_leaf_point_is_in_recursive(X_BspLevelLoader* level, int nodeId, X_Vec3* point)
{
    if(nodeId < 0)
    {
        return ~nodeId;
    }
    
    X_BspLoaderNode* node = level->nodes + nodeId;    
    X_BspLoaderPlane* plane = level->planes + node->planeNum;
    
    int childNode = x_plane_point_is_on_normal_facing_side(&plane->plane, point)
        ? node->children[0] : node->children[1];
        
    return x_bsplevel_find_leaf_point_is_in_recursive(level, childNode, point);
}

int x_bsplevel_find_leaf_point_is_in(X_BspLevel* level, X_Vec3* point)
{
    return 0;//return x_bsplevel_find_leaf_point_is_in_recursive(level, x_bsplevel_get_root_node(level), point);
}

void x_bspllevel_decompress_pvs_for_leaf(X_BspLevel* level, X_BspLoaderLeaf* leaf, unsigned char* decompressedPvsDest)
{
//     // The PVS is compressed using zero run-length encoding
//     int pos = 0;
//     int pvsSize = x_bspfile_node_pvs_size(level);
//     unsigned char* pvsData = NULL;//leaf->compressedPvsData;
//     
//     // No visibility info (whoever compiled the map didn't run the vis tool)
//     if(pvsData == NULL)
//     {
//         // Mark all leaves as visible
//         for(int i = 0; i < pvsSize; ++i)
//             decompressedPvsDest[i] = 0xFF;
//             
//         return;
//     }
//     
//     while(pos < pvsSize)
//     {
//         if(*pvsData == 0)
//         {
//             ++pvsData;
//             int count = *pvsData++;
//             
//             for(int i = 0; i < count && pos < pvsSize; ++i)
//                 decompressedPvsDest[pos++] = 0;
//         }
//         else
//         {
//             decompressedPvsDest[pos++] = *pvsData++;
//         }
//     }
}

void x_bsplevel_init_empty(X_BspLevel* level)
{
//     level->compressedPvsData = NULL;
//     
//     level->edges = NULL;
//     level->totalEdges = 0;
//     
//     level->faces = NULL;
//     level->totalFaces = 0;
//     
//     level->file.file = NULL;
//     
//     level->leaves = NULL;
//     level->totalLeaves = 0;
//     
//     level->nodes = NULL;
//     level->totalNodes = 0;
//     
//     level->planes = NULL;
//     level->totalPlanes = 0;
//     
//     level->vertices = NULL;
//     level->totalVertices = 0;
    
    level->flags = 0;
}

void x_bsplevel_mark_leaves_from_pvs(X_BspLevelLoader* level, unsigned char* pvs, int currentFrame)
{
//     X_BspLoaderModel* levelModel = x_bsploaderlevel_get_level_model(level);
//     
//     for(int i = 0; i < levelModel->totalBspLeaves; ++i)
//     {
//         _Bool potentiallVisible = pvs[i / 8] & (1 << ())
//     }
}

