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

#include "ComponentType.hpp"
#include "CameraComponent.hpp"
#include "InputComponent.hpp"
#include "BoxColliderComponent.hpp"
#include "TransformComponent.hpp"
#include "ScriptableComponent.hpp"
#include "PhysicsComponent.hpp"
#include "RenderComponent.hpp"

struct ComponentRecord
{
    template<typename T>
    T* getComponent();

    Flags<ComponentType> types;

    // TODO: do these need to be initialized?
    CameraComponent* cameraComponent = nullptr;
    InputComponent* inputComponent = nullptr;
    BoxColliderComponent* boxColliderComponent = nullptr;
    TransformComponent* transformComponent = nullptr;
    ScriptableComponent* scriptableComponent = nullptr;
    PhysicsComponent* physicsComponent = nullptr;
    RenderComponent* renderComponent = nullptr;
};

template<> inline CameraComponent* ComponentRecord::getComponent() { return cameraComponent; }
template<> inline InputComponent* ComponentRecord::getComponent() { return inputComponent; }
template<> inline BoxColliderComponent* ComponentRecord::getComponent() { return boxColliderComponent; }
template<> inline TransformComponent* ComponentRecord::getComponent() { return transformComponent; }
template<> inline ScriptableComponent* ComponentRecord::getComponent() { return scriptableComponent; }
template<> inline PhysicsComponent* ComponentRecord::getComponent() { return physicsComponent; }
template<> inline RenderComponent* ComponentRecord::getComponent() { return renderComponent; }

template<> inline BrushModelPhysicsComponent* ComponentRecord::getComponent()
{
    return static_cast<BrushModelPhysicsComponent*>(physicsComponent);
}

template<> inline QuakeModelRenderComponent* ComponentRecord::getComponent()
{
    return static_cast<QuakeModelRenderComponent*>(renderComponent);
}

template<> inline BillboardRenderComponent* ComponentRecord::getComponent()
{
    return static_cast<BillboardRenderComponent*>(renderComponent);
}


template<typename T> constexpr inline bool isValidComponentType() { return false; }
template<> constexpr bool inline isValidComponentType<CameraComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<InputComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<BoxColliderComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<TransformComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<ScriptableComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<PhysicsComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<BrushModelPhysicsComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<RenderComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<QuakeModelRenderComponent>() { return true; }
template<> constexpr bool inline isValidComponentType<BillboardRenderComponent>() { return true; }

template<typename T> constexpr inline ComponentType getComponentType();
template<> constexpr inline ComponentType getComponentType<CameraComponent>() { return ComponentType::camera; }
template<> constexpr inline ComponentType getComponentType<InputComponent>() { return ComponentType::input; }
template<> constexpr inline ComponentType getComponentType<BoxColliderComponent>() { return ComponentType::collider; }
template<> constexpr inline ComponentType getComponentType<TransformComponent>() { return ComponentType::transform; }
template<> constexpr inline ComponentType getComponentType<ScriptableComponent>() { return ComponentType::scriptable; }
template<> constexpr inline ComponentType getComponentType<PhysicsComponent>() { return ComponentType::physics; }
template<> constexpr inline ComponentType getComponentType<QuakeModelRenderComponent>() { return ComponentType::render; }
template<> constexpr inline ComponentType getComponentType<BillboardRenderComponent>() { return ComponentType::render; }

