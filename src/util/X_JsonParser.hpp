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

#include "Json.hpp"

class JsonParser
{
public:
    JsonParser(const char* start) : next(start)
    {

    }

    void expect(char c);
    void expectWord(const char* word);

    JsonValue* parse();

private:
    JsonValue* parseValue();
    JsonValue* parseXString();
    JsonValue* parseObject();
    JsonValue* parseArray();
    JsonValue* parseNumber();
    JsonValue* parseTrue();
    JsonValue* parseFalse();
    JsonValue* parseNull();

    void parseXStringLiteral(XString& dest);

    void skipWhitespace();

    const char* next;
};

