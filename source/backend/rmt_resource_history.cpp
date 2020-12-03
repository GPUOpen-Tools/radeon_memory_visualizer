//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author
/// \brief Implementation of functions operating on a resource history structure.
//=============================================================================

#include <rmt_resource_history.h>
#include <rmt_assert.h>

// add an event to resource history log
RmtErrorCode RmtResourceHistoryAddEvent(RmtResourceHistory*         resource_history,
                                        RmtResourceHistoryEventType event_type,
                                        uint64_t                    thread_id,
                                        uint64_t                    timestamp,
                                        bool                        compact)
{
    RMT_ASSERT(resource_history);
    RMT_RETURN_ON_ERROR(resource_history, RMT_ERROR_INVALID_POINTER);

    if (resource_history->event_count >= RMT_MAXIMUM_RESOURCE_HISTORY_EVENTS)
    {
        return RMT_ERROR_OUT_OF_MEMORY;
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
            if (last_event->event_type == event_type && last_event->timestamp == timestamp && last_event->thread_id == thread_id)
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
        resource_history->event_count++;
    }

    return RMT_OK;
}

// destroy resource history
RmtErrorCode RmtResourceHistoryDestroy(RmtResourceHistory* resource_history)
{
    RMT_ASSERT(resource_history);
    RMT_RETURN_ON_ERROR(resource_history, RMT_ERROR_INVALID_POINTER);

    return RMT_OK;
}
