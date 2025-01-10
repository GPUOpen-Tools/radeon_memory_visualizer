//=============================================================================
// Copyright (c) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Private Structures and functions for collecting memory event history.
//=============================================================================

#ifndef RMV_MEMORY_EVENT_HISTORY_IMPL_H_
#define RMV_MEMORY_EVENT_HISTORY_IMPL_H_

#include "rmt_memory_event_history.h"

#include <string>
#include <vector>

/// Private Event information structure used by the internal history implementation.
typedef struct EventInfoImpl
{
    EventInfoImpl()
    {
        event_type = static_cast<RmtResourceHistoryEventType>(0);
        timestamp  = 0;
        event_data = {};
    }

    RmtResourceHistoryEventType event_type;  ///< The event type that this structure holds.
    uint64_t                    timestamp;   ///< The time that the event occured.
    union EventData
    {
        RmtMemoryEventHistoryPhysicalMapToLocalEventInfo physical_map_to_local_info;  ///< Parameters associated with the Physical Memory Map to Local event.
        RmtMemoryEventHistoryPhysicalMapToHostEventInfo  physical_map_to_host_info;   ///< Parameters associated with the Physical Memory Mapped to Host event.
        RmtMemoryEventHistoryPhysicalUnmapEventInfo      physical_unmap_info;         ///< Parameters associated with the Physical Memory Unmapped event.
        RmtMemoryEventHistoryResourceBindEventInfo       resource_bind_info;          ///< Parameters associated with the Resource Bind event.
        RmtMemoryEventHistoryResourceCreateEventInfo     resource_create_info;        ///< Parameters associated with the Resource Create event.
        RmtMemoryEventHistoryResourceDestroyEventInfo    resource_destroy_info;       ///< Parameters associated with the Resource Destroy event.
        RmtMemoryEventHistoryVirtualMemoryAllocationEventInfo
                                                         virtual_memory_allocation_info;   ///< Parameters associated with the Virtual Memory Allocation event.
        RmtMemoryEventHistoryVirtualMemoryFreeEventInfo  virtual_memory_free_info;         ///< Parameters associated with the Virtual Memory Free event.
        RmtMemoryEventHistoryVirtualMemoryMapEventInfo   virtual_memory_map_info;          ///< Parameters associated with the Virtual Memory Mapped event.
        RmtMemoryEventHistoryVirtualMemoryUnmapEventInfo virtual_memory_unmap_info;        ///< Parameters associated with the Virtual Memory Unmapped event.
        RmtMemoryEventHistoryVirtualMemoryEvictEventInfo virtual_memory_evict_info;        ///< Parameters associated with the Virtual Memory Evict event.
        RmtMemoryEventHistoryVirtualMemoryResidentEventInfo virtual_memory_resident_info;  ///< Parameters associated with the Make Resident event.
    } event_data;
} EventInfoImpl;

/// Type definition for the list of events contained in a history instance.
typedef std::vector<EventInfoImpl> EventHistoryListType;

/// Private structure containing unique properties of a token.
struct TokenPropertiesImpl
{
    uint64_t     timestamp;   ///< The timestamp (in RMT clocks) when the token was generated.
    uint64_t     thread_id;   ///< The thread ID that the token was emitted from.
    RmtTokenType token_type;  ///< The type of the RMT token.
};

/// Private event history class used by the internal implementation.
class EventHistoryImpl
{
public:
    /// Constructor.
    EventHistoryImpl();

    /// Destructor.
    ~EventHistoryImpl();

    /// Adds a Resource Create event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenResourceCreate& token);

    /// Adds a Resource Destroy event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenResourceDestroy& token);

    /// Adds a Resource Bind event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenResourceBind& token);

    /// Adds a Virtual Memory Allocation event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenVirtualAllocate& token);

    /// Adds a Virtual Memory Make Resident or Virtual Memory Evict event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenResourceReference& token);

    /// Adds a Virtual Memory Mapped or Virtual Memory Unmapped event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenCpuMap& token);

    /// Adds a Virtual Memory Freed event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenVirtualFree& token);

    /// Adds a Physical Memory Map to Host, Physical Memory Map to Local or Physical Memory Unmap event to the history.
    ///
    /// @param [in]  token                          The RMT Token for the event to be added to the history.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    RmtErrorCode AddEvent(const RmtTokenPageTableUpdate& token);

    /// Gets the Event History class object from a history handle.
    ///
    /// @param [in]  history_handle                 The instance handle for a previously generated history of events.
    ///
    /// @retval                                     A pointer to an <c><i>EventHistoryPrivate</i></c> object associated with the history handle.
    ///
    static const EventHistoryImpl* FromHandle(const RmtMemoryEventHistoryHandle history_handle);

    /// Verifies that an index is within the valid range of values.
    ///
    /// @param [in]  index                          The history's event index to be checked.
    ///
    /// @retval                                     If the index is valid, returns true.  Otherwise returns false.
    ///
    bool CheckEventIndex(RmtMemoryEventHistoryEventIndex index) const;

    /// Returns the number of events in the event history.
    ///
    /// @retval                                     The number of events in the history.
    ///
    size_t GetEventCount() const;

    /// Returns a pointer to the parameters used to generate the history.
    ///
    /// @retval                                     A pointer to the usage parameters associated with this history instance.
    ///
    const RmtMemoryEventHistoryUsageParameters* GetUsageParameters() const;

    ///  Sets the parameters used when the history is generated.
    ///
    /// @param [in]  parameters                     A reference to the usage parameters to be assigned to this history instance.
    ///
    void SetUsageParameters(const RmtMemoryEventHistoryUsageParameters& parameters);

    /// Template function used to retrieve event information structures.
    ///
    /// @param [in]  history_handle                 The instance handle for a previously generated history of events.
    /// @param [in]  event_index                    The index of the event information in the history to retrieve.
    /// @param [in]  event_type                     The template type of event to retrieve.
    /// @param [in]  out_event_info                 A pointer to the structure populated with the event information.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    /// @retval
    /// kRmtErrorIndexOutOfRange                    Invalid <c><i>event_index</i></c>.
    /// @retval
    /// kRmtErrorMalformedData                      The <c><i>event_type</i></c> doesn't match the structure type retrieved.
    /// @retval
    /// kRmtErrorInvalidPointer                     The <c><i>history_handle</i></c> is an invalid pointer.
    ///
    template <typename EventInfoType>
    static RmtErrorCode GetEventInfo(const RmtMemoryEventHistoryHandle     history_handle,
                                     const RmtMemoryEventHistoryEventIndex event_index,
                                     const RmtResourceHistoryEventType     event_type,
                                     const EventInfoType**                 out_event_info)
    {
        RmtErrorCode            result  = kRmtErrorInvalidPointer;
        const EventHistoryImpl* history = EventHistoryImpl::FromHandle(history_handle);

        if (history != nullptr)
        {
            RmtResourceHistoryEventType expected_event_type;
            result = history->GetEventType(event_index, &expected_event_type);
            if (result == kRmtOk)
            {
                // Verify that the event type matches the event type being retrieved from the history.
                if (event_type == expected_event_type)
                {
                    // Typecast to the correct event info unionized structure using the <EventInfoType> template parameter.
                    *out_event_info = reinterpret_cast<const EventInfoType*>(&history->event_list_[event_index].event_data);
                    result          = kRmtOk;
                }
                else
                {
                    result = kRmtErrorMalformedData;
                }
            }
        }

        return result;
    }

    ///  Returns the event type for an event in the history.
    ///
    /// @param [in]  event_index                    The index of the event to retrieve the type for.
    /// @param [in]  out_event_type                 A pointer that receives the event type value.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    /// @retval
    /// kRmtErrorIndexOutOfRange                    Invalid <c><i>event_index</i></c>.
    /// @retval
    /// kRmtErrorInvalidPointer                     The <c><i>event_type</i></c> is an invalid pointer.
    ///
    RmtErrorCode GetEventType(const RmtMemoryEventHistoryEventIndex event_index, RmtResourceHistoryEventType* out_event_type) const;

    /// Returns the time in clock ticks that an event was generated.
    ///
    /// @param [in]  event_index                    The index of the event to retrieve the timestamp for.
    /// @param [in]  out_timestamp                  A pointer that receives the timestamp value.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    /// @retval
    /// kRmtErrorIndexOutOfRange                    Invalid <c><i>event_index</i></c>.
    /// @retval
    /// kRmtErrorInvalidPointer                     The <c><i>timestamp</i></c> is an invalid pointer.
    ///
    RmtErrorCode GetEventTimestamp(const RmtMemoryEventHistoryEventIndex event_index, uint64_t* out_timestamp) const;

    /// Update the address of the virtual allocation for a bind event.
    ///
    /// @param [in]  event_index                    The index of the event to be updated.
    /// @param [in]  virtual_address                The virtual address of the allocation bound to this resource.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    /// @retval
    /// kRmtErrorMalformedData                      The index points to an event which is not a bind event.
    /// @retval
    ///
    RmtErrorCode UpdateResourceAllocationVirtualAddress(const RmtMemoryEventHistoryEventIndex event_index, RmtGpuAddress virtual_address);

    /// Update the heap preferences for a resource.
    ///
    /// @param [in]  event_index                    The index of the event to be updated.
    /// @param [in]  heap_preferences               A pointer to the heap types.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    /// @retval
    /// kRmtErrorMalformedData                      The index points to an event which is not a bind event.
    /// @retval
    ///
    RmtErrorCode UpdateResourceHeapPreferences(const RmtMemoryEventHistoryEventIndex event_index, const RmtHeapType* heap_preferences);

    /// Update the list of resources affected by a make resident or evict event.
    ///
    /// @param [in]  event_index                    The index of the event to be updated.
    /// @param [in]  resource_list                  The list of resource identifiers for the resources affected by the event.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    /// @retval
    /// kRmtErrorIndexOutOfRange                    Invalid <c><i>event_index</i></c>.
    /// @retval
    /// kRmtErrorMalformedData                      The index points to an event which is not a make resident or evict event.
    /// @retval
    ///
    RmtErrorCode UpdateAffectedResourcesForMemorySwapEvent(const RmtMemoryEventHistoryEventIndex     event_index,
                                                           const std::vector<RmtResourceIdentifier>& resource_list);

private:
    ///  Checks for duplicate page table update events.
    ///
    /// @param [in]  token                          The common data for the token.
    /// @param [in]  token_type                     The type for the token being checked.
    ///
    /// @retval
    /// kRmtOk                                      The operation completed successfully.
    ///
    bool CheckDuplicateEvent(const RmtTokenCommon& token, const RmtTokenType token_type);

private:
    EventHistoryListType event_list_;  ///< A list of events included in the history timeline.
        /// NOTE: Events should only be added to event_list_ when generating history.  Adding events after pointers to events have been retrieved
        /// may result in the container re-allocating memory, causing the pointers be invalid.

    RmtMemoryEventHistoryUsageParameters parameters_;             ///< Describes the history operating mode.
    struct TokenPropertiesImpl           last_token_properties_;  ///< Used to identify duplicate events.
};
#endif  // #ifndef RMV_MEMORY_EVENT_HISTORY_IMPL_H_
