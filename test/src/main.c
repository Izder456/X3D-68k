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

#include <X3D/X3D.h>
#include <SDL/SDL.h>
#include <unistd.h>
#include <math.h>

#include "palette.h"
#include "Context.h"
#include "screen.h"
#include "keys.h"

_Bool init_sdl(Context* context, int screenW, int screenH)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    context->screen = SDL_SetVideoMode(screenW, screenH, 32, SDL_SWSURFACE);
    
    return context->screen != NULL;
}

void init_x3d(Context* context, int screenW, int screenH)
{
    X_EngineContext* xContext = &context->context;
    
    x_enginecontext_init(xContext, screenW, screenH);
    
    context->cam = x_cameraobject_new(xContext);
    x_viewport_init(&context->cam->viewport, (X_Vec2) { 0, 0 }, 640, 480, X_ANG_60);
    x_screen_attach_camera(&xContext->screen, context->cam);
    
    context->cam->base.orientation = x_quaternion_identity();
    
    X_Vec3_fp16x16 axis = x_vec3_make(0, X_FP16x16_ONE, 0);
    X_Quaternion rotation = x_quaternion_identity();
    
    //x_quaternion_init_from_axis_angle(&rotation, &axis, X_ANG_45);
    
    X_Quaternion newOrientation;
    x_quaternion_mul(&context->cam->base.orientation, &rotation, &newOrientation);
    context->cam->base.orientation = newOrientation;
    
    X_Vec3 up, right, forward;
    x_gameobject_extract_view_vectors(&context->cam->base, &forward, &right, &up);
    
    x_vec3_print(&up, "Up");
    x_vec3_print(&forward, "Foreward");
    x_vec3_print(&right, "Right");
    
    X_Vec3 camPos = x_vec3_make(0, 0, 0);
    
    x_viewport_update_frustum(&context->cam->viewport, &camPos, &forward, &right, &up);
    x_frustum_print(&context->cam->viewport.viewFrustum);
}

void init(Context* context, int screenW, int screenH)
{
    context->quit = 0;
    
    init_sdl(context, screenW, screenH);
    init_x3d(context, screenW, screenH);
    build_color_table(context->screen);
}

void cleanup_sdl(Context* context)
{
    SDL_Quit();
}

void cleanup_x3d(Context* context)
{
    x_enginecontext_cleanup(&context->context);
}

void cleanup(Context* context)
{
    cleanup_sdl(context);
    cleanup_x3d(context);
}

void handle_keys(Context* context)
{
    handle_key_events();
    
    _Bool adjustCam = 0;
    
    if(key_is_down(SDLK_UP))
    {
        context->cam->angleX--;
        adjustCam = 1;
    }
    else if(key_is_down(SDLK_DOWN))
    {
        context->cam->angleX++;
        adjustCam = 1;
    }
    
    if(key_is_down(SDLK_LEFT))
    {
        context->cam->angleY++;
        adjustCam = 1;
    }
    else if(key_is_down(SDLK_RIGHT))
    {
        context->cam->angleY--;
        adjustCam = 1;
    }
    
    if(key_is_down(SDLK_ESCAPE))
        context->quit = 1;
    
    if(adjustCam)
    {
        x_cameraobject_update_view(context->cam);
    }
}

int main(int argc, char* argv[])
{
    Context context;
    
    init(&context, 640, 480);
    
#if 1
    
    X_RenderContext rcontext;
    rcontext.cam = context.cam;
    rcontext.canvas = &context.context.screen.canvas;
    rcontext.viewFrustum = &rcontext.cam->viewport.viewFrustum;
    rcontext.viewMatrix = &context.cam->viewMatrix;
    
    context.cam->angleX = 0;
    context.cam->angleY = 0;
    context.cam->base.position = x_vec3_make(0, 0, 0);
    
    x_cameraobject_update_view(context.cam);
    
    x_mat4x4_print(&context.cam->viewMatrix);
    
    X_Vec2 a = { 10, 10 };
    X_Vec2 b = { 200, 100 };
    
    X_Cube cube;
    x_cube_init(&cube, 50, 50, 50);
    
    X_Cube rotatedCube;
    
    int invSqrt3 = (1.0 / sqrt(3)) * 65536;
    
    while(!context.quit)
    {
        x_canvas_fill(&context.context.screen.canvas, 0);

        handle_keys(&context);
        
        X_Mat4x4 rotation;
        X_Quaternion quat;
        
        X_Vec3_fp16x16 axis = x_vec3_make(invSqrt3, invSqrt3, invSqrt3);
        x_quaternion_init_from_euler_angles(&quat, 0, 0, 0);
        //x_quaternion_init_from_axis_angle(&quat, &axis, i);
        x_quaternion_to_mat4x4(&quat, &rotation);
        
        x_cube_transform(&cube, &rotatedCube, &rotation);
        
        x_cube_translate(&rotatedCube, x_vec3_make(0, 0, 500));
        x_cube_render(&rotatedCube, &rcontext, 4);
        
        update_screen(&context);
    }
    
#endif
    cleanup(&context);
}

