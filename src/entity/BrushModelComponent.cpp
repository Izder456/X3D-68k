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

#include "BrushModelComponent.hpp"
#include "EntityDictionary.hpp"
#include "level/BspLevel.hpp"
#include "system/Clock.hpp"

namespace internal
{
    BrushModel::BrushModel(const X_Edict& edict, const BspLevel& level)
    {
        BrushModelId id;
        edict.getRequiredValue("model", id);
        
        model = x_bsplevel_get_model(&level, id.id);
    }

    void BrushModel::initiateMoveTo(const Vec3fp &destination, Duration moveLength, BrushModelReachedDestinationHandler onArrive)
    {
        movement.endTime = Clock::getTicks() + moveLength;
        movement.finalPosition = destination;
        movement.direction = (destination - model->center) / moveLength.toSeconds();
        movement.onArriveHandler = onArrive;
        movement.isMoving = true;
    }
}