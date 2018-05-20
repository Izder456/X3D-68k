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

#include <string>

#include "X_Allocator.hpp"

using String = std::basic_string<char, std::char_traits<char>, XAllocator<char>>;

typedef struct X_String
{
    char* data;
} X_String;

void x_string_init(X_String* str, const char* initialValue);
void x_string_cleanup(X_String* str);
X_String* x_string_assign(X_String* str, const char* value);
X_String* x_string_concat(X_String* strToAppendTo, const X_String* strToAppend);
X_String* x_string_concat_cstr(X_String* strToAppendTo, const char* strToAppend);

