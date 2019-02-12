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

#include "Config.hpp"
#include "object/CameraObject.hpp"
#include "entity/EntityManager.hpp"

#include <time.h>

#ifdef X_GET_TIME_USING_SDL
#include <SDL/SDL.h>
#endif

#include "EngineContext.hpp"
#include "error/Error.hpp"
#include "render/RenderContext.hpp"
#include "system/Clock.hpp"
#include "level/LevelManager.hpp"

BspLevel* X_EngineContext::getCurrentLevel() const
{
    return levelManager->getCurrentLevel();
}
