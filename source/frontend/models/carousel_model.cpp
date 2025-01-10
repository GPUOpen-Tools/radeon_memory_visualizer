//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the carousel model.
//=============================================================================

#include "models/carousel_model.h"

#include <math.h>

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_virtual_allocation_list.h"

#include "managers/trace_manager.h"
#include "views/custom_widgets/rmv_carousel_item.h"

// The sort comparator used when sorting the resources in descending order.
static int ResourceSortComparator(const void* r1, const void* r2)
{
    int32_t a1 = reinterpret_cast<const rmv::ResourceMapping*>(r1)->usage_amount;
    int32_t a2 = reinterpret_cast<const rmv::ResourceMapping*>(r2)->usage_amount;
    return (abs(a2) - abs(a1));
}

namespace rmv
{
    CarouselModel::CarouselModel()
    {
    }

    CarouselModel::~CarouselModel()
    {
    }

    bool CarouselModel::GetCarouselData(RmtDataSnapshot* snapshot, RMVCarouselData& out_carousel_data) const
    {
        bool success = false;

        if (TraceManager::Get().DataSetValid())
        {
            if (snapshot != nullptr)
            {
                success = true;

                uint64_t total_size = 0;

                uint64_t consumed_per_type[kRmtHeapTypeCount]  = {};
                uint64_t available_per_type[kRmtHeapTypeCount] = {};

                const uint64_t total_available      = RmtVirtualAllocationListGetTotalSizeInBytes(&snapshot->virtual_allocation_list);
                const uint64_t allocated_and_used   = RmtVirtualAllocationListGetBoundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);
                const uint64_t allocated_and_unused = RmtVirtualAllocationListGetUnboundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);

                total_size = allocated_and_used + allocated_and_unused;

                RMT_ASSERT(total_size <= total_available);

                // Get the total amount of memory per heap.
                for (int heap = 0; heap < kRmtHeapTypeCount; heap++)
                {
                    available_per_type[heap] = snapshot->data_set->segment_info[heap].size;
                }

                // Do a quick check to see if the output carousel data is blank.
                RMT_ASSERT(out_carousel_data.resource_types_data.usage_map[0].usage_amount == 0);
                RMT_ASSERT(out_carousel_data.allocation_sizes_data.buckets[0] == 0);
                RMT_ASSERT(out_carousel_data.allocation_sizes_data.num_allocations == 0);
                RMT_ASSERT(out_carousel_data.memory_footprint_data.total_allocated_memory == 0);
                RMT_ASSERT(out_carousel_data.memory_footprint_data.total_unused_memory == 0);
                RMT_ASSERT(out_carousel_data.memory_footprint_data.max_memory == 0);
                RMT_ASSERT(out_carousel_data.memory_types_data.preferred_heap[0].value == 0);
                RMT_ASSERT(out_carousel_data.memory_types_data.preferred_heap[0].max == 0);

                for (int32_t i = 0; i < snapshot->virtual_allocation_list.allocation_count; i++)
                {
                    const RmtVirtualAllocation* current_allocation = &snapshot->virtual_allocation_list.allocation_details[i];
                    uint64_t                    allocation_size    = static_cast<uint64_t>(current_allocation->size_in_4kb_page) * 4096;
                    const int                   resource_count     = current_allocation->resource_count;
                    for (int j = 0; j < resource_count; j++)
                    {
                        RmtResource*         current_resource = current_allocation->resources[j];
                        RmtResourceUsageType resource_type    = RmtResourceGetUsageType(current_resource);
                        out_carousel_data.resource_types_data.usage_amount[resource_type]++;
                    }
                    if (current_allocation->heap_preferences[0] < kRmtHeapTypeCount)
                    {
                        consumed_per_type[current_allocation->heap_preferences[0]] += allocation_size;
                    }

                    int bucket_index = GetAllocationBucketIndex(allocation_size);
                    out_carousel_data.allocation_sizes_data.buckets[bucket_index]++;
                }

                // Find the resource with the maximum value and store it. Also copy the resource amounts into the map
                // so they can be sorted.
                int32_t maximum = 0;
                for (int32_t i = 0; i < kRmtResourceUsageTypeCount; i++)
                {
                    out_carousel_data.resource_types_data.usage_map[i].usage_type   = static_cast<RmtResourceUsageType>(i);
                    out_carousel_data.resource_types_data.usage_map[i].usage_amount = out_carousel_data.resource_types_data.usage_amount[i];
                    int32_t current_value                                           = out_carousel_data.resource_types_data.usage_map[i].usage_amount;
                    if (current_value > maximum)
                    {
                        maximum = current_value;
                    }
                }
                out_carousel_data.resource_types_data.usage_maximum = maximum;

                // Sort the resources by amount.
                qsort((void*)out_carousel_data.resource_types_data.usage_map, kRmtResourceUsageTypeCount, sizeof(ResourceMapping), ResourceSortComparator);

                out_carousel_data.allocation_sizes_data.num_allocations = snapshot->virtual_allocation_list.allocation_count;

                RMVCarouselMemoryFootprintData& memory_footprint = out_carousel_data.memory_footprint_data;
                memory_footprint.total_allocated_memory          = allocated_and_used;
                memory_footprint.total_unused_memory             = allocated_and_unused;
                memory_footprint.max_memory                      = total_available;

                RMVCarouselMemoryTypesData& memory_types_data = out_carousel_data.memory_types_data;
                for (int i = 0; i < kRmtHeapTypeNone; i++)
                {
                    RmtSegmentStatus segment_status;
                    RmtDataSnapshotGetSegmentStatus(snapshot, static_cast<RmtHeapType>(i), &segment_status);
                    memory_types_data.name[i]                 = QString(RmtGetHeapTypeNameFromHeapType(static_cast<RmtHeapType>(i)));
                    memory_types_data.preferred_heap[i].value = consumed_per_type[i];
                    memory_types_data.preferred_heap[i].max   = available_per_type[i];
                    memory_types_data.preferred_heap[i].color = GetColorFromSubscription(segment_status);
                    memory_types_data.physical_heap[i].value  = segment_status.total_physical_mapped_by_process;
                    memory_types_data.physical_heap[i].max    = segment_status.total_physical_size;
                    memory_types_data.physical_heap[i].color  = kDefaultCarouselBarColor;
                }
            }
        }

        return success;
    }

    QColor CarouselModel::GetColorFromSubscription(const RmtSegmentStatus& segment_status) const
    {
        RmtSegmentSubscriptionStatus status = RmtSegmentStatusGetOversubscribed(&segment_status);

        switch (status)
        {
        case kRmtSegmentSubscriptionStatusOverLimit:
            return rmv::kOverSubscribedColor;

        case kRmtSegmentSubscriptionStatusUnderLimit:
            return rmv::kUnderSubscribedColor;

        case kRmtSegmentSubscriptionStatusCloseToLimit:
            return rmv::kCloseToSubscribedColor;

        default:
            break;
        }

        return kDefaultCarouselBarColor;
    }

    bool CarouselModel::CalcGlobalCarouselData(RmtDataSnapshot* base_snapshot, RmtDataSnapshot* diff_snapshot, RMVCarouselData& out_carousel_delta_data)
    {
        if (TraceManager::Get().DataSetValid())
        {
            if (base_snapshot != nullptr && diff_snapshot != nullptr)
            {
                // Update global data.
                RMVCarouselData base_carousel_data = {};
                bool            base_success       = GetCarouselData(base_snapshot, base_carousel_data);

                RMVCarouselData diff_carousel_data = {};
                bool            diff_success       = GetCarouselData(diff_snapshot, diff_carousel_data);

                if (base_success && diff_success)
                {
                    // Memory footprint delta.
                    RMVCarouselMemoryFootprintData& memory_footprint = out_carousel_delta_data.memory_footprint_data;
                    memory_footprint.max_memory =
                        std::max<double>(base_carousel_data.memory_footprint_data.max_memory, diff_carousel_data.memory_footprint_data.max_memory);
                    memory_footprint.total_allocated_memory =
                        diff_carousel_data.memory_footprint_data.total_allocated_memory - base_carousel_data.memory_footprint_data.total_allocated_memory;
                    memory_footprint.total_unused_memory =
                        diff_carousel_data.memory_footprint_data.total_unused_memory - base_carousel_data.memory_footprint_data.total_unused_memory;

                    // Resource deltas.
                    int32_t maximum = 0;
                    for (int i = 0; i < kRmtResourceUsageTypeCount; i++)
                    {
                        out_carousel_delta_data.resource_types_data.usage_map[i].usage_type = static_cast<RmtResourceUsageType>(i);
                        int32_t diff = diff_carousel_data.resource_types_data.usage_amount[i] - base_carousel_data.resource_types_data.usage_amount[i];
                        out_carousel_delta_data.resource_types_data.usage_map[i].usage_amount = diff;

                        diff = abs(diff);
                        if (diff > maximum)
                        {
                            maximum = diff;
                        }
                    }
                    out_carousel_delta_data.resource_types_data.usage_maximum = maximum;

                    // Sort the resources by amount.
                    qsort((void*)out_carousel_delta_data.resource_types_data.usage_map,
                          kRmtResourceUsageTypeCount,
                          sizeof(ResourceMapping),
                          ResourceSortComparator);

                    // Heap deltas.
                    RMVCarouselMemoryTypesData& memory_types_data = out_carousel_delta_data.memory_types_data;
                    for (int i = 0; i < kRmtHeapTypeCount; i++)
                    {
                        memory_types_data.preferred_heap[i].max = base_carousel_data.memory_types_data.preferred_heap[i].max;
                        memory_types_data.preferred_heap[i].value =
                            diff_carousel_data.memory_types_data.preferred_heap[i].value - base_carousel_data.memory_types_data.preferred_heap[i].value;
                        memory_types_data.physical_heap[i].max = base_carousel_data.memory_types_data.physical_heap[i].max;
                        memory_types_data.physical_heap[i].value =
                            diff_carousel_data.memory_types_data.physical_heap[i].value - base_carousel_data.memory_types_data.physical_heap[i].value;

                        // Add heap name. Both snapshots contain the same heap names so use the reference snapshot.
                        memory_types_data.name[i] = base_carousel_data.memory_types_data.name[i];
                    }

                    // Allocation deltas.
                    out_carousel_delta_data.allocation_sizes_data.num_allocations =
                        std::max<int32_t>(diff_carousel_data.allocation_sizes_data.num_allocations, base_carousel_data.allocation_sizes_data.num_allocations);

                    for (int i = 0; i < kNumAllocationSizeBuckets; i++)
                    {
                        out_carousel_delta_data.allocation_sizes_data.buckets[i] =
                            diff_carousel_data.allocation_sizes_data.buckets[i] - base_carousel_data.allocation_sizes_data.buckets[i];
                    }

                    return true;
                }
            }
        }

        return false;
    }

    bool CarouselModel::GetCarouselData(RMVCarouselData& out_carousel_data) const
    {
        RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
        if (open_snapshot != nullptr)
        {
            return GetCarouselData(open_snapshot, out_carousel_data);
        }
        return false;
    }

    int CarouselModel::GetAllocationBucketIndex(uint64_t allocation_size) const
    {
        // The bucket index is calculated by using the MSB as an index ie bit 31
        // would give an index of 31. The index is then shifted down to give
        // indices for: <1MB (0), <2MB (1), <4MB (2) etc.
        // Anything >1GB will go in the last bucket.
        int bucket_index = 0;

        allocation_size >>= 19;  // Shift down by 2^19 so anything <1MB is in bucket 0.
        if (allocation_size > 0)
        {
            bucket_index = log2(allocation_size);
            if (bucket_index >= kNumAllocationSizeBuckets)
            {
                bucket_index = kNumAllocationSizeBuckets - 1;
            }
        }
        return bucket_index;
    }
}  // namespace rmv
