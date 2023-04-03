//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of functions operating on a resource history structure.
//=============================================================================

#include "rmt_resource_history.h"
#include "rmt_assert.h"

// add an event to resource history log
RmtErrorCode RmtResourceHistoryAddEvent(RmtResourceHistory*         resource_history,
                                        RmtResourceHistoryEventType event_type,
                                        uint64_t                    thread_id,
                                        uint64_t                    timestamp,
                                        uint64_t                    virtual_address,
                                        uint64_t                    physical_address,
                                        uint64_t                    size_in_bytes,
                                        uint64_t                    page_size_in_bytes,
                                        bool                        compact)
{
    RMT_ASSERT(resource_history);
    RMT_RETURN_ON_ERROR(resource_history, kRmtErrorInvalidPointer);

    if (resource_history->event_count >= RMT_MAXIMUM_RESOURCE_HISTORY_EVENTS)
    {
        return kRmtErrorOutOfMemory;
    }

    // if the events need compacting, ignore this event if it's indentical to the previous event
    bool duplicate = false;
    if (compact)
    {
        int32_t last_event_index = resource_history->event_count - 1;
        if (last_event_index >= 0)
        {
            // if the last event is the same as the current event, it's a duplicate
            RmtResourceHistoryEvent* last_event = &resource_history->events[last_event_index];
            if (last_event->event_type == event_type && last_event->timestamp == timestamp && last_event->thread_id == thread_id &&
                last_event->virtual_address == virtual_address)
            {
                duplicate = true;
            }
        }
    }

    if (!duplicate)
    {
        RmtResourceHistoryEvent* next_event = &resource_history->events[resource_history->event_count];
        next_event->event_type              = event_type;
        next_event->timestamp               = timestamp;
        next_event->thread_id               = thread_id;
        next_event->virtual_address         = virtual_address;
        next_event->physical_address        = physical_address;
        next_event->size_in_bytes           = size_in_bytes;
        next_event->page_size_in_bytes      = static_cast<uint32_t>(page_size_in_bytes);
        resource_history->event_count++;
    }

    return kRmtOk;
}

// destroy resource history
RmtErrorCode RmtResourceHistoryDestroy(RmtResourceHistory* resource_history)
{
    RMT_ASSERT(resource_history);
    RMT_RETURN_ON_ERROR(resource_history, kRmtErrorInvalidPointer);

    return kRmtOk;
}
