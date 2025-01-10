//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Private implementation of the memory history.
//=============================================================================

#include "rmt_memory_event_history_impl.h"

#include "rmt_assert.h"
#include "rmt_resource_list.h"
#include "rmt_resource_userdata.h"

#include <string>
#include <vector>

EventHistoryImpl::EventHistoryImpl()
    : parameters_{}
{
    last_token_properties_.thread_id  = 0;
    last_token_properties_.timestamp  = 0;
    last_token_properties_.token_type = RmtTokenType::kRmtTokenTypeCount;
}

EventHistoryImpl::~EventHistoryImpl()
{
    for (auto& event_info : event_list_)
    {
        if (event_info.event_type == RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryMakeResident)
        {
            delete[] event_info.event_data.virtual_memory_resident_info.resource_identifier_array;
        }
        else if (event_info.event_type == RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryEvict)
        {
            delete[] event_info.event_data.virtual_memory_evict_info.resource_identifier_array;
        }
    }
}

/// Copy resource description data from the token to the event info structure.
static void CopyResourceDescription(const RmtTokenResourceCreate& token, RmtMemoryEventHistoryResourceCreateEventInfo& out_create_info)
{
    switch (token.resource_type)
    {
    case kRmtResourceTypeImage:
        memcpy(&out_create_info.image, &token.image, sizeof(out_create_info.image));
        break;

    case kRmtResourceTypeBuffer:
        memcpy(&out_create_info.buffer, &token.buffer, sizeof(out_create_info.buffer));
        break;

    case kRmtResourceTypeGpuEvent:
        memcpy(&out_create_info.gpu_event, &token.gpu_event, sizeof(out_create_info.gpu_event));
        break;

    case kRmtResourceTypeBorderColorPalette:
        memcpy(&out_create_info.border_color_palette, &token.border_color_palette, sizeof(out_create_info.border_color_palette));
        break;

    case kRmtResourceTypePerfExperiment:
        memcpy(&out_create_info.perf_experiment, &token.perf_experiment, sizeof(out_create_info.perf_experiment));
        break;

    case kRmtResourceTypeQueryHeap:
        memcpy(&out_create_info.query_heap, &token.query_heap, sizeof(out_create_info.query_heap));
        break;

    case kRmtResourceTypeVideoDecoder:
        memcpy(&out_create_info.video_decoder, &token.video_decoder, sizeof(out_create_info.video_decoder));
        break;

    case kRmtResourceTypeVideoEncoder:
        memcpy(&out_create_info.video_encoder, &token.video_encoder, sizeof(out_create_info.video_encoder));
        break;

    case kRmtResourceTypeHeap:
        memcpy(&out_create_info.heap, &token.heap, sizeof(out_create_info.heap));
        break;

    case kRmtResourceTypePipeline:
        memcpy(&out_create_info.pipeline, &token.pipeline, sizeof(out_create_info.pipeline));
        break;

    case kRmtResourceTypeDescriptorHeap:
        memcpy(&out_create_info.descriptor_heap, &token.descriptor_heap, sizeof(out_create_info.descriptor_heap));
        break;

    case kRmtResourceTypeDescriptorPool:
        memcpy(&out_create_info.descriptor_pool, &token.descriptor_pool, sizeof(out_create_info.descriptor_pool));
        break;

    case kRmtResourceTypeCommandAllocator:
        memcpy(&out_create_info.command_allocator, &token.command_allocator, sizeof(out_create_info.command_allocator));
        break;

    case kRmtResourceTypeMiscInternal:
        memcpy(&out_create_info.misc_internal, &token.misc_internal, sizeof(out_create_info.misc_internal));
        break;

    case kRmtResourceTypeIndirectCmdGenerator:
    case kRmtResourceTypeMotionEstimator:
    case kRmtResourceTypeTimestamp:
        // NOTE: no data associated with these types, if this changes in future we'll need to copy it here.
        break;

    default:
        // shouldn't reach here.
        RMT_ASSERT(0);
        break;
    }
}

/// Add a Resource Create event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenResourceCreate& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeResourceCreate))
    {
        // Create a temporary resource object that will be used to determine the resource usage type.
        RmtResource resource_object   = {};
        resource_object.resource_type = token.resource_type;

        if (token.resource_type == kRmtResourceTypeImage)
        {
            resource_object.image.usage_flags = token.image.usage_flags;
        }
        else if (token.resource_type == kRmtResourceTypeBuffer)
        {
            resource_object.buffer.usage_flags = token.buffer.usage_flags;
        }

        EventInfoImpl event_info    = {};
        const char*   resource_name = nullptr;
        if (RmtResourceUserdataGetResourceName(token.resource_identifier, &resource_name) == kRmtOk)
        {
            event_info.event_data.resource_create_info.name = resource_name;
        }
        else
        {
            event_info.event_data.resource_create_info.name = nullptr;
        }

        bool is_implicit = RmtResourceUserDataIsResourceImplicit(token.resource_identifier);

        event_info.event_type                                          = RmtResourceHistoryEventType::kRmtResourceHistoryEventResourceCreated;
        event_info.event_data.resource_create_info.resource_identifier = token.resource_identifier;
        event_info.event_data.resource_create_info.resource_type       = token.resource_type;
        event_info.event_data.resource_create_info.resource_usage_type = RmtResourceGetUsageType(&resource_object);
        event_info.event_data.resource_create_info.commit_type         = token.commit_type;
        event_info.event_data.resource_create_info.owner_type          = token.owner_type;
        event_info.timestamp                                           = token.common.timestamp;
        event_info.event_data.resource_create_info.is_implicit         = is_implicit;
        CopyResourceDescription(token, event_info.event_data.resource_create_info);
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Resource Destroy event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenResourceDestroy& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeResourceDestroy))
    {
        EventInfoImpl event_info;
        event_info.event_type                                           = RmtResourceHistoryEventType::kRmtResourceHistoryEventResourceDestroyed;
        event_info.event_data.resource_destroy_info.resource_identifier = token.resource_identifier;
        event_info.timestamp                                            = token.common.timestamp;
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Resource Bind event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenResourceBind& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeResourceBind))
    {
        EventInfoImpl event_info;
        event_info.event_type                                              = RmtResourceHistoryEventType::kRmtResourceHistoryEventResourceBound;
        event_info.event_data.resource_bind_info.resource_identifier       = token.resource_identifier;
        event_info.event_data.resource_bind_info.is_system_memory          = token.is_system_memory;
        event_info.event_data.resource_bind_info.size_in_bytes             = token.size_in_bytes;
        event_info.event_data.resource_bind_info.virtual_address           = token.virtual_address;
        event_info.event_data.resource_bind_info.resource_bound_allocation = 0;
        event_info.timestamp                                               = token.common.timestamp;
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Virtual Memory Allocation event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenVirtualAllocate& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeVirtualAllocate))
    {
        EventInfoImpl event_info;
        event_info.event_type                                           = RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryAllocated;
        event_info.event_data.virtual_memory_allocation_info.owner_type = token.owner_type;
        for (int index = 0; index < RMT_NUM_HEAP_PREFERENCES; index++)
        {
            event_info.event_data.virtual_memory_allocation_info.preference[index] = token.preference[index];
        }
        event_info.event_data.virtual_memory_allocation_info.size_in_bytes   = token.size_in_bytes;
        event_info.event_data.virtual_memory_allocation_info.virtual_address = token.virtual_address;
        event_info.event_data.virtual_memory_allocation_info.is_external     = token.is_external;
        event_info.timestamp                                                 = token.common.timestamp;
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Virtual Memory Make Resident or Virtual Memory Evict event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenResourceReference& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeResourceReference))
    {
        EventInfoImpl event_info;
        event_info.timestamp = token.common.timestamp;

        if (token.residency_update_type == RmtResidencyUpdateType::kRmtResidencyUpdateTypeAdd)
        {
            event_info.event_type                                              = RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryMakeResident;
            event_info.event_data.virtual_memory_resident_info.virtual_address = token.virtual_address;
            event_info.event_data.virtual_memory_resident_info.queue           = token.queue;
        }
        else
        {
            event_info.event_type                                           = RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryEvict;
            event_info.event_data.virtual_memory_evict_info.virtual_address = token.virtual_address;
            event_info.event_data.virtual_memory_evict_info.queue           = token.queue;
        }
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Virtual Memory Mapped or Virtual Memory Unmapped event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenCpuMap& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeCpuMap))
    {
        EventInfoImpl event_info;
        event_info.timestamp = token.common.timestamp;

        if (token.is_unmap)
        {
            event_info.event_type                                           = RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryUnmapped;
            event_info.event_data.virtual_memory_unmap_info.virtual_address = token.virtual_address;
        }
        else
        {
            event_info.event_type                                         = RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryMapped;
            event_info.event_data.virtual_memory_map_info.virtual_address = token.virtual_address;
        }
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Virtual Memory Freed event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenVirtualFree& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypeVirtualFree))
    {
        EventInfoImpl event_info;
        event_info.event_type                                          = RmtResourceHistoryEventType::kRmtResourceHistoryEventVirtualMemoryFree;
        event_info.timestamp                                           = token.common.timestamp;
        event_info.event_data.virtual_memory_free_info.virtual_address = token.virtual_address;
        event_list_.push_back(event_info);
    }
    return kRmtOk;
}

/// Add a Physical Memory Map to Host, Physical Memory Map to Local or Physical Memory Unmap event to the history.
RmtErrorCode EventHistoryImpl::AddEvent(const RmtTokenPageTableUpdate& token)
{
    if (!CheckDuplicateEvent(token.common, RmtTokenType::kRmtTokenTypePageTableUpdate))
    {
        EventInfoImpl event_info;
        event_info.timestamp = token.common.timestamp;

        if (token.is_unmapping)
        {
            event_info.event_type                                      = RmtResourceHistoryEventType::kRmtResourceHistoryEventPhysicalUnmap;
            event_info.event_data.physical_unmap_info.controller       = token.controller;
            event_info.event_data.physical_unmap_info.page_size        = token.page_size;
            event_info.event_data.physical_unmap_info.size_in_pages    = token.size_in_pages;
            event_info.event_data.physical_unmap_info.physical_address = token.physical_address;
            event_info.event_data.physical_unmap_info.update_type      = token.update_type;
            event_info.event_data.physical_unmap_info.virtual_address  = token.virtual_address;
        }
        else
        {
            if (token.physical_address == 0)
            {
                event_info.event_type                                            = RmtResourceHistoryEventType::kRmtResourceHistoryEventPhysicalMapToHost;
                event_info.event_data.physical_map_to_host_info.controller       = token.controller;
                event_info.event_data.physical_map_to_host_info.page_size        = token.page_size;
                event_info.event_data.physical_map_to_host_info.size_in_pages    = token.size_in_pages;
                event_info.event_data.physical_map_to_host_info.physical_address = token.physical_address;
                event_info.event_data.physical_map_to_host_info.update_type      = token.update_type;
                event_info.event_data.physical_map_to_host_info.virtual_address  = token.virtual_address;
            }
            else
            {
                event_info.event_type                                             = RmtResourceHistoryEventType::kRmtResourceHistoryEventPhysicalMapToLocal;
                event_info.event_data.physical_map_to_local_info.controller       = token.controller;
                event_info.event_data.physical_map_to_local_info.page_size        = token.page_size;
                event_info.event_data.physical_map_to_local_info.size_in_pages    = token.size_in_pages;
                event_info.event_data.physical_map_to_local_info.physical_address = token.physical_address;
                event_info.event_data.physical_map_to_local_info.update_type      = token.update_type;
                event_info.event_data.physical_map_to_local_info.virtual_address  = token.virtual_address;
            }
        }

        event_list_.push_back(event_info);
    }

    return kRmtOk;
}

const EventHistoryImpl* EventHistoryImpl::FromHandle(const RmtMemoryEventHistoryHandle history_handle)
{
    EventHistoryImpl* history = nullptr;
    if (history_handle != nullptr)
    {
        history = reinterpret_cast<EventHistoryImpl*>(history_handle);
    }
    return history;
}

bool EventHistoryImpl::CheckEventIndex(RmtMemoryEventHistoryEventIndex index) const
{
    bool result = false;
    if ((index != kRmtMemoryEventHistoryInvalidEventIndex) && (index < event_list_.size()))
    {
        result = true;
    }

    return result;
}

size_t EventHistoryImpl::GetEventCount() const
{
    return event_list_.size();
}

const RmtMemoryEventHistoryUsageParameters* EventHistoryImpl::GetUsageParameters() const
{
    return &parameters_;
}

void EventHistoryImpl::SetUsageParameters(const RmtMemoryEventHistoryUsageParameters& parameters)
{
    parameters_ = parameters;
}

RmtErrorCode EventHistoryImpl::GetEventType(const RmtMemoryEventHistoryEventIndex event_index, RmtResourceHistoryEventType* out_event_type) const
{
    RMT_RETURN_ON_ERROR(out_event_type != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtErrorIndexOutOfRange;

    if (CheckEventIndex(event_index))
    {
        *out_event_type = event_list_[event_index].event_type;
        result          = kRmtOk;
    }

    return result;
}

RmtErrorCode EventHistoryImpl::GetEventTimestamp(const RmtMemoryEventHistoryEventIndex index, uint64_t* out_timestamp) const
{
    RMT_RETURN_ON_ERROR(out_timestamp != nullptr, kRmtErrorInvalidPointer);

    RmtErrorCode result = kRmtErrorIndexOutOfRange;

    if (CheckEventIndex(index))
    {
        *out_timestamp = event_list_[index].timestamp;
        result         = kRmtOk;
    }

    return result;
}

bool EventHistoryImpl::CheckDuplicateEvent(const RmtTokenCommon& token, const RmtTokenType token_type)
{
    bool is_duplicate = false;

    // Only events for duplicate page table update tokens are filtered.
    if (token_type == RmtTokenType::kRmtTokenTypePageTableUpdate)
    {
        if (GetUsageParameters()->hide_duplicate_page_table_events)
        {
            if ((last_token_properties_.thread_id == token.thread_id) && (last_token_properties_.timestamp == token.timestamp) &&
                (last_token_properties_.token_type == token_type))
            {
                is_duplicate = true;
            }
        }
    }

    last_token_properties_.thread_id  = token.thread_id;
    last_token_properties_.timestamp  = token.timestamp;
    last_token_properties_.token_type = token_type;

    return is_duplicate;
}

// Update the resource allocation virtual address for a bind event.
RmtErrorCode EventHistoryImpl::UpdateResourceAllocationVirtualAddress(const RmtMemoryEventHistoryEventIndex event_index, RmtGpuAddress virtual_address)
{
    RmtErrorCode result = kRmtErrorMalformedData;

    if (CheckEventIndex(event_index))
    {
        EventInfoImpl& event_info = event_list_[event_index];
        if (event_info.event_type == kRmtResourceHistoryEventResourceBound)
        {
            event_list_[event_index].event_data.resource_bind_info.resource_bound_allocation = virtual_address;
            result                                                                           = kRmtOk;
        }
    }
    else
    {
        result = kRmtErrorIndexOutOfRange;
    }

    return result;
}

// Update the heap preferences for a bind event.
RmtErrorCode EventHistoryImpl::UpdateResourceHeapPreferences(const RmtMemoryEventHistoryEventIndex event_index, const RmtHeapType* heap_preferences)
{
    RmtErrorCode result = kRmtErrorMalformedData;

    if (CheckEventIndex(event_index))
    {
        EventInfoImpl& event_info = event_list_[event_index];
        if (event_info.event_type == kRmtResourceHistoryEventResourceBound)
        {
            memcpy(event_list_[event_index].event_data.resource_bind_info.heap_preferences,
                   heap_preferences,
                   sizeof(event_list_[event_index].event_data.resource_bind_info.heap_preferences));
            result = kRmtOk;
        }
    }
    else
    {
        result = kRmtErrorIndexOutOfRange;
    }

    return result;
}

// Update the list of resource IDs for mark resident and evict history events.
RmtErrorCode EventHistoryImpl::UpdateAffectedResourcesForMemorySwapEvent(const RmtMemoryEventHistoryEventIndex     event_index,
                                                                         const std::vector<RmtResourceIdentifier>& resource_list)
{
    RmtErrorCode result = kRmtErrorIndexOutOfRange;
    if (CheckEventIndex(event_index))
    {
        EventInfoImpl&         event_info     = event_list_[event_index];
        size_t                 resource_count = resource_list.size();
        RmtResourceIdentifier* resource_array = nullptr;

        result = kRmtOk;
        if (resource_count > 0)
        {
            // Allocate memory for the resource array.  This is deleted when the history is freed.
            resource_array     = new RmtResourceIdentifier[resource_count];
            size_t array_index = 0;
            for (const auto resource_id_entry : resource_list)
            {
                if (array_index < resource_count)
                {
                    resource_array[array_index] = resource_id_entry;
                }
                array_index++;
            }

            if (event_info.event_type == kRmtResourceHistoryEventVirtualMemoryEvict)
            {
                event_info.event_data.virtual_memory_evict_info.resource_identifier_array = resource_array;
                event_info.event_data.virtual_memory_evict_info.resource_count            = resource_count;
            }
            else if ((event_info.event_type == kRmtResourceHistoryEventVirtualMemoryMakeResident))
            {
                event_info.event_data.virtual_memory_resident_info.resource_identifier_array = resource_array;
                event_info.event_data.virtual_memory_resident_info.resource_count            = resource_count;
            }
            else
            {
                delete[] resource_array;
                result = kRmtErrorMalformedData;
                RMT_ASSERT(false);
            }
        }
    }

    return result;
}
