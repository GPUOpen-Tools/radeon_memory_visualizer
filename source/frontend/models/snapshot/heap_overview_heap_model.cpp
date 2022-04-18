//=============================================================================
// Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a model for a heap layout for the Heap Overview
/// pane.
//=============================================================================

#include "models/snapshot/heap_overview_heap_model.h"

#include <QVariant>

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "models/resource_sorter.h"
#include "util/string_util.h"

namespace rmv
{
    static const QString kSamStatusText   = " (Smart Access Memory is ";
    static const QString kSamEnabledText  = "enabled)";
    static const QString kSamDisabledText = "disabled)";

    static const QString kHeapDescriptions[kRmtHeapTypeCount] = {
        QString("This heap is in local (video) memory. It is mappable by the CPU, but does not use the CPU cache."),
        QString("This heap is in local (video) memory. It is not mappable by the CPU."),
        QString("This heap is in host (system) memory. It is intended for write-only data on the CPU side.")};

    static const QString kWarningHeader = "<b>WARNING! </b><br>";

    static const QString kWarningOverSubscribed =
        "This heap is currently oversubscribed. This means more memory is requested from this heap than exists on your system.";

    static const QString kWarningCloseToOverSubscribed =
        "This heap is very close to over-subscription which may cause paging of your allocations to a non-preferred heap.";

    HeapOverviewHeapModel::HeapOverviewHeapModel(RmtHeapType heap)
        : ModelViewMapper(kHeapOverviewNumWidgets)
        , heap_(heap)
        , segment_status_{}
    {
    }

    HeapOverviewHeapModel::~HeapOverviewHeapModel()
    {
    }

    void HeapOverviewHeapModel::ResetModelValues()
    {
        memset(&segment_status_, 0, sizeof(RmtSegmentStatus));

        SetModelData(kHeapOverviewTitle, "-");
        SetModelData(kHeapOverviewDescription, "-");

        SetModelData(kHeapOverviewWarningText, "-");

        SetModelData(kHeapOverviewLocation, "-");
        SetModelData(kHeapOverviewCpuCached, "-");
        SetModelData(kHeapOverviewCpuVisible, "-");
        SetModelData(kHeapOverviewGpuCached, "-");
        SetModelData(kHeapOverviewGpuVisible, "-");
        SetModelData(kHeapOverviewSmallestAllocation, "-");
        SetModelData(kHeapOverviewLargestAllocation, "-");
        SetModelData(kHeapOverviewMeanAllocation, "-");
    }

    bool HeapOverviewHeapModel::ShowSubscriptionWarning()
    {
        RmtSegmentSubscriptionStatus status = RmtSegmentStatusGetOversubscribed(&segment_status_);

        if (status == kRmtSegmentSubscriptionStatusOverLimit || status == kRmtSegmentSubscriptionStatusCloseToLimit)
        {
            return true;
        }
        return false;
    }

    void HeapOverviewHeapModel::Update()
    {
        const RmtDataSnapshot* snapshot = GetSnapshot();
        if (snapshot == nullptr)
        {
            return;
        }

        ResetModelValues();

        // Update global data.
        if (heap_ == kRmtHeapTypeLocal)
        {
            QString sam_status_string = kSamStatusText + (IsSAMSupported() ? kSamEnabledText : kSamDisabledText);
            SetModelData(kHeapOverviewSamStatus, sam_status_string);
        }
        else
        {
            SetModelData(kHeapOverviewSamStatus, "");
        }

        SetModelData(kHeapOverviewTitle, RmtGetHeapTypeNameFromHeapType(heap_));
        SetModelData(kHeapOverviewDescription, kHeapDescriptions[heap_]);

        // Call the backend to get the segment data.
        RmtDataSnapshotGetSegmentStatus(snapshot, heap_, &segment_status_);

        // Update subscription warning.
        RmtSegmentSubscriptionStatus status = RmtSegmentStatusGetOversubscribed(&segment_status_);
        if (status == kRmtSegmentSubscriptionStatusOverLimit)
        {
            SetModelData(kHeapOverviewWarningText, kWarningHeader + kWarningOverSubscribed);
        }
        else if (status == kRmtSegmentSubscriptionStatusCloseToLimit)
        {
            SetModelData(kHeapOverviewWarningText, kWarningHeader + kWarningCloseToOverSubscribed);
        }

        // Update summary data.
        if ((segment_status_.flags & kRmtSegmentStatusFlagVram) != 0)
        {
            SetModelData(kHeapOverviewLocation, "Video memory");
        }
        else if ((segment_status_.flags & kRmtSegmentStatusFlagHost) != 0)
        {
            SetModelData(kHeapOverviewLocation, "System memory");
        }

        SetModelData(kHeapOverviewCpuCached, ((segment_status_.flags & kRmtSegmentStatusFlagCpuCached) != 0) ? "Yes" : "No");
        SetModelData(kHeapOverviewCpuVisible, ((segment_status_.flags & kRmtSegmentStatusFlagCpuVisible) != 0) ? "Yes" : "No");
        SetModelData(kHeapOverviewGpuCached, ((segment_status_.flags & kRmtSegmentStatusFlagGpuCached) != 0) ? "Yes" : "No");
        SetModelData(kHeapOverviewGpuVisible, ((segment_status_.flags & kRmtSegmentStatusFlagGpuVisible) != 0) ? "Yes" : "No");
        SetModelData(kHeapOverviewSmallestAllocation, rmv::string_util::LocalizedValueMemory(segment_status_.min_allocation_size, false, false));
        SetModelData(kHeapOverviewLargestAllocation, rmv::string_util::LocalizedValueMemory(segment_status_.max_allocation_size, false, false));
        SetModelData(kHeapOverviewMeanAllocation, rmv::string_util::LocalizedValueMemory(segment_status_.mean_allocation_size, false, false));
    }

    void HeapOverviewHeapModel::GetMemoryParameters(uint64_t&                     total_physical_size,
                                                    uint64_t&                     total_virtual_memory_requested,
                                                    uint64_t&                     total_bound_virtual_memory,
                                                    uint64_t&                     total_physical_mapped_by_process,
                                                    uint64_t&                     total_physical_mapped_by_other_processes,
                                                    RmtSegmentSubscriptionStatus& subscription_status) const
    {
        total_physical_size                      = segment_status_.total_physical_size;
        total_virtual_memory_requested           = segment_status_.total_virtual_memory_requested;
        total_bound_virtual_memory               = segment_status_.total_bound_virtual_memory;
        total_physical_mapped_by_process         = segment_status_.total_physical_mapped_by_process;
        total_physical_mapped_by_other_processes = segment_status_.total_physical_mapped_by_other_processes;
        subscription_status                      = RmtSegmentStatusGetOversubscribed(&segment_status_);
    }

    int HeapOverviewHeapModel::GetResourceData(int num_resources, uint64_t* resource_info, int* other_value) const
    {
        const uint64_t* physical_bytes_per_resource_usage = segment_status_.physical_bytes_per_resource_usage;

        // Add all resource totals to the sorter and sort.
        ResourceSorter sorter;
        for (int i = 0; i < kRmtResourceUsageTypeCount; i++)
        {
            sorter.AddResource((RmtResourceUsageType)i, physical_bytes_per_resource_usage[i]);
        }
        sorter.Sort();

        // Return the most abundant resources.
        int resource_count = 0;
        for (resource_count = 0; resource_count < num_resources; resource_count++)
        {
            int64_t value = sorter.GetResourceValue(resource_count);
            if (value <= 0)
            {
                break;
            }
            *resource_info++ = sorter.GetResourceType(resource_count);
            *resource_info++ = value;
        }

        // Get what's left as a single value.
        *other_value = sorter.GetRemainder(num_resources);

        return resource_count;
    }

    const RmtDataSnapshot* HeapOverviewHeapModel::GetSnapshot() const
    {
        if (!TraceManager::Get().DataSetValid())
        {
            return nullptr;
        }

        const RmtDataSnapshot* snapshot = SnapshotManager::Get().GetOpenSnapshot();
        RMT_ASSERT(snapshot != nullptr);
        return snapshot;
    }

    bool HeapOverviewHeapModel::IsSAMSupported()
    {
        return TraceManager::Get().GetDataSet()->sam_enabled;
    }

    RmtHeapType HeapOverviewHeapModel::GetHeapType() const
    {
        return heap_;
    }

}  // namespace rmv