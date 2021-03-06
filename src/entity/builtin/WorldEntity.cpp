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

#include "WorldEntity.hpp"
#include "entity/EntityManager.hpp"
#include "entity/EntityBuilder.hpp"

//WorldEntity::WorldEntity(X_Edict& edict, BspLevel& level)
//    : Entity(level)
//{
//    BspModel* worldModel = &getLevel().getLevelModel();
//    addComponent<BrushModelPhysicsComponent>(worldModel);
//}
//
//

Entity* WorldEntity::build(EntityBuilder& builder)
{
    WorldEntity* world = builder
        .withComponent<TransformComponent>()
        .withPhysicsComponent(PhysicsComponentType::brushModel)
        .build<WorldEntity>();

    BrushModelPhysicsComponent* brushModelPhysicsComponent = world->getComponent<BrushModelPhysicsComponent>();
    brushModelPhysicsComponent->model = &builder.level->getLevelModel();

    return world;
}
