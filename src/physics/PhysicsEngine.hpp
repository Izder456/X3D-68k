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

#include "system/Time.hpp"
#include "math/FixedPoint.hpp"
#include "geo/Vec3.hpp"
#include "entity/EntityEvent.hpp"

class Entity;
class BspLevel;

struct CollideEntityEvent : EntityEvent
{
    static constexpr StringId Name = StringId("onCollide");

    CollideEntityEvent(Entity* collideWith_)
        : EntityEvent(Name),
        collideWith(collideWith_)
    {

    }

    Entity* collideWith;
};

struct PickupEntityEvent : EntityEvent
{
    static constexpr StringId Name = StringId("onPickup");

    constexpr PickupEntityEvent(Entity* pickedUp_)
        : EntityEvent(StringId("onPickup")),    // FIXME
        pickedUp(pickedUp_)
    {

    }

    Entity* pickedUp;
};

struct CrushEntityEvent : EntityEvent
{
    static constexpr StringId Name = StringId("onCrush");

    CrushEntityEvent(Entity* crushedBy_)
    : EntityEvent(Name),
    crushedBy(crushedBy_)
    {

    }

    Entity* crushedBy;
};

class PhysicsEngine
{
public:
    static void update(BspLevel& level, fp timeDelta);
    static void sendCollideEvent(Entity* a, Entity* b);
    static void sendTriggerEvent(Entity* trigger, Entity* entityThatHitTrigger);
    
private:
    static void step(BspLevel& level, fp dt);
    static void moveBrushModels(BspLevel& level, fp dt);
    static void pushBrushEntity(Entity* brushEntity, const Vec3fp& movement, BspLevel& level);

    template<typename TEvent, typename ...TConstructorArgs>
    static EntityEventResponse sendEventToEntity(Entity* receivingEntity, TConstructorArgs&&... args)
    {
        TEvent event(std::forward<TConstructorArgs>(args)...);

        return sendEventToEntityImplementation(receivingEntity, event);
    }

    static EntityEventResponse sendEventToEntityImplementation(Entity* receivingEntity, const EntityEvent& event);
};

