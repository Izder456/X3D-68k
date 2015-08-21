/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/

#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_engine.h"
#include "X3D_object.h"


_Bool x3d_is_object_active(X3D_Object* obj) {
  return obj->flags & X3D_OBJECT_ACTIVE;
}

_Bool x3d_activate_object(X3D_Context* context, X3D_Object* obj) {
  uint16 i;
  X3D_ObjectManager* manager = &context->object_manager;


  for(i = 0; i < X3D_MAX_ACTIVE_OBJECTS; ++i) {
    if(manager->active_list[i] == NULL) {
      manager->active_list[i] = obj;
      obj->flags |= X3D_OBJECT_ACTIVE;
      return TRUE;
    }
  }

  return FALSE;
}

void x3d_deactivate_object(X3D_Context* context, X3D_Object* obj) {
  uint16 i;
  X3D_ObjectManager* manager = &context->object_manager;


  for(i = 0; i < X3D_MAX_ACTIVE_OBJECTS; ++i) {
    if(manager->active_list[i] == obj) {
      manager->active_list[i] = NULL;
      obj->flags &= ~X3D_OBJECT_ACTIVE;
    }
  }
}

X3D_Object* x3d_get_object(X3D_Context* context, uint16 id) {
  return context->object_manager.object_data + id * X3D_OBJECT_SIZE;
}

X3D_Object* x3d_create_object(X3D_Context* context, uint16 object_type, Vex3D pos, Vex3D_angle256 angle, Vex3D_fp8x8 velocity) {

}

X3D_Camera* x3d_create_camera(X3D_Context* context, uint16 id, Vex3D pos, Vex3D_angle256 angle) {

}