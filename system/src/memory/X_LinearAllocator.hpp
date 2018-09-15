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

#include "X_FixedLengthString.hpp"

namespace X3D
{
    class SystemAllocator;
    class Cache;

    class LinearAllocator
    {
    public:
        LinearAllocator(int size, SystemAllocator& sysAllocator_, Cache& cache_);

        void* allocHigh(int size, const char* name);
        void* allocLow(int size, const char* name);

        unsigned char* getHighMark()
        {
            return highMark;
        }

        void freeToHighMark(unsigned char* highMark);

        unsigned char* getLowMark()
        {
            return lowMark;
        }

        void freeToLowMark(unsigned char* lowMark);

        void print();

        ~LinearAllocator();

    private:
        struct Header
        {
            int size;
            unsigned int sentinel;
            FixedLengthString<8> name;
        };

        void printHighHunk(unsigned char* ptr);

        static const unsigned int SENTINEL = 0xEC53DB01;

        SystemAllocator& sysAllocator;
        Cache& cache;

        unsigned char* memoryStart;
        unsigned char* memoryEnd;

        unsigned char* highMark;
        unsigned char* lowMark;  
    };
};

