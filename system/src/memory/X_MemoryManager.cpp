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

#include "X_MemoryManager.hpp"
#include "log/X_Log.hpp"
#include "error/X_Exception.hpp"

namespace X3D
{
    MemoryManager::MemoryManager(MemoryManagerConfig& config)
        : linearAllocator(config.linearAllocatorSize, systemAllocator, cache),
        cache(linearAllocator),
        zoneAllocator(config.zoneAllocatorSize, linearAllocator)
    {
        Log::info("Initialized memory manager");
    }

    void* MemoryManager::alloc(int size, AllocationSource source)
    {
        switch(source)
        {
            case AllocationSource::system:
                return systemAllocator.alloc(size);

            case AllocationSource::zone:
                return zoneAllocator.alloc<unsigned char>(size);

            default:
                throw Exception("Unknow allocator type in MemoryManager::alloc");
        }
    }

    void MemoryManager::free(void* data, AllocationSource source)
    {
        printf("Source: %d\n", (int)source);
        switch(source)
        {
            case AllocationSource::system:
                systemAllocator.free(data);
                break;

            case AllocationSource::zone:
                zoneAllocator.free(data);
                break;

            default:
                throw Exception("Unknow allocator type in MemoryManager::free");
        }
    }

    MemoryManager::~MemoryManager()
    {
        Log::info("Shutdown memory manager");
    }
};

