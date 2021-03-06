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

#include <cstring>

#include "Renderer.hpp"
#include "error/Log.hpp"

void Renderer::render()
{
    if(currentRenderer != nullptr)
    {

    }
}

bool Renderer::switchRenderer(const char *name)
{
    for(RendererOption &option : renderers)
    {
        if(strcmp(option.name, name) == 0)
        {
            currentRenderer = option.renderer;

            return true;
        }
    }

    Log::error("No such renderer %s", name);

    return false;
}

void Renderer::addRenderer(IRenderer *renderer, const char* name)
{
    renderers.push_back(RendererOption(renderer, name));
}
