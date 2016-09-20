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
#include "X3D_enginestate.h"
#include "X3D_keys.h"
#include "X3D_gameloop.h"
#include "X3D_render.h"

#include <SDL/SDL.h>

void x3d_game_loop_quit(void) {
  x3d_enginestate_get()->exit_gameloop = X3D_TRUE;
}

void test_lightmap();

///////////////////////////////////////////////////////////////////////////////
/// The main game loop.
///////////////////////////////////////////////////////////////////////////////
void x3d_game_loop() {
  X3D_EngineState* state = x3d_enginestate_get();
  X3D_RenderContext render_context = {
      .render_type = X3D_RENDER_LIGHTMAP
  };
  
  do {
    // Update the key state
    x3d_read_keys();
    x3d_keymanager_get()->key_handler();


    x3d_send_frame_events_to_objects();
    
    // Render from the player's perspective
    x3d_screen_clear(0);
    x3d_screen_zbuf_clear();
    x3d_render(x3d_playermanager_get()->player[0].cam, &render_context);
    
    
    //if(x3d_key_down(X3D_KEY_10)) {
    //    x3d_screen_zbuf_clear();
    //    test_lightmap();
    //}
    
    x3d_screen_flip();

    /// @todo Platform-independent solution

#ifdef __linux__
    if(x3d_key_down(X3D_KEY_15))
      SDL_Delay(500);
    else
      SDL_Delay(10);
#endif

    x3d_enginestate_next_step();
  } while(!state->exit_gameloop);
}
