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


#include <engine/GlobalConfiguration.hpp>
#include <memory/Set.hpp>
#include "IEntitySystem.hpp"

class RenderSystem : public IEntitySystem
{
public:
    using SetType = Set<Entity*, Configuration::ENTITIES_MAX>;

    void createEntity(Entity& entity);

    void destroyEntity(Entity& entity);

    SetType& getAllQuakeModels()
    {
        return quakeModels;
    }

    SetType& getAllBillboards()
    {
        return billboards;
    }

private:
    SetType quakeModels;
    SetType brushModels;
    SetType billboards;
};

