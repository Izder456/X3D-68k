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

#include "memory/CircularQueue.hpp"

#define X_ENGINEQUEUE_SIZE 32

enum EngineEventType
{
    EVENT_LEVEL_LOAD_PROGRESS = 1,
    EVENT_LEVEL_LOAD_FAILED
};

struct EngineEvent
{
    EngineEventType type;

    union
    {
        // EVENT_LEVEL_LOAD_PROGRESS
        struct
        {
            const char* fileName;
            int currentStep;
            int totalSteps;
        } levelLoadProgress;

        // EVENT_LEVEL_LOAD_FAILED
        struct
        {
            const char* fileName;
        };
    };
};

struct EngineContext;

class EngineEventHandler
{
public:
    EngineEventHandler(const char* name_, bool (*handler)(EngineEvent& event, EngineContext* engineContext), int priority_)
        : name(name_),
        handleEvent(handler),
        priority(priority_)
    {

    }

    bool invoke(EngineEvent& event, EngineContext* engineContext)
    {
        return handleEvent(event, engineContext);
    }

    void remove();

    EngineEventHandler* next;

    const char* name;
    bool (*handleEvent)(EngineEvent& event, EngineContext* engineContext);
    int priority;
};

class EngineQueue
{
public:
    EngineQueue(EngineContext* engineContext_)
        : engineContext(engineContext_),
        handlerHead(nullptr)
    {
        addHandler("syshandler", systemHandler, 100000);
    }

    void addEvent(EngineEvent& event);
    void flush();

    void addHandler(const char* name, bool (*handler)(EngineEvent& event, EngineContext* engineContext), int priority);
    void removeHandler(const char* name);

private:
    static bool systemHandler(EngineEvent& event, EngineContext* engineContext);

    EngineContext* engineContext;
    CircularQueue<EngineEvent, X_ENGINEQUEUE_SIZE> queue;
    EngineEventHandler* handlerHead;
};

