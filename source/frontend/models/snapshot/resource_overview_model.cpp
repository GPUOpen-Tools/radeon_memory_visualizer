//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the Resource Overview model.
//=============================================================================

#include "models/snapshot/resource_overview_model.h"

#include <QTableView>

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "util/string_util.h"

namespace rmv
{
    ResourceOverviewModel::ResourceOverviewModel()
        : ModelViewMapper(kResourceOverviewNumWidgets)
        , min_resource_size_(0)
        , max_resource_size_(0)
    {
    }

    ResourceOverviewModel::~ResourceOverviewModel()
    {
    }

    void ResourceOverviewModel::ResetModelValues(bool use_unbound)
    {
        SetModelData(kResourceOverviewTotalAvailableSize, "-");
        SetModelData(kResourceOverviewTotalAllocatedAndUsed, "-");
        SetModelData(kResourceOverviewTotalAllocatedAndUnused, "-");
        SetModelData(kResourceOverviewAllocationCount, "-");
        SetModelData(kResourceOverviewResourceCount, "-");

        const RmtDataSnapshot* open_snapshot = SnapshotManager::Get().GetOpenSnapshot();
        if (TraceManager::Get().DataSetValid() && (open_snapshot != nullptr))
        {
            min_resource_size_ = open_snapshot->minimum_resource_size_in_bytes;
            if (use_unbound)
            {
                max_resource_size_ = std::max<uint64_t>(open_snapshot->maximum_resource_size_in_bytes, open_snapshot->maximum_unbound_resource_size_in_bytes);
            }
            else
            {
                max_resource_size_ = open_snapshot->maximum_resource_size_in_bytes;
            }
        }
    }

    void ResourceOverviewModel::FilterBySizeChanged(int32_t min_value, int32_t max_value, bool use_unbound)
    {
        const SnapshotManager& snapshot_manager = SnapshotManager::Get();
        const RmtDataSnapshot* open_snapshot    = snapshot_manager.GetOpenSnapshot();
        if (TraceManager::Get().DataSetValid() && (open_snapshot != nullptr))
        {
            min_resource_size_ = snapshot_manager.GetSizeFilterThreshold(min_value, use_unbound);
            max_resource_size_ = snapshot_manager.GetSizeFilterThreshold(max_value, use_unbound);
        }
    }

    bool ResourceOverviewModel::IsSizeInSliderRange(uint64_t resource_size) const
    {
        if (resource_size >= min_resource_size_ && resource_size <= max_resource_size_)
        {
            return true;
        }
        return false;
    }

    void ResourceOverviewModel::Update(bool use_unbound)
    {
        ResetModelValues(use_unbound);

        const RmtDataSnapshot* snapshot = SnapshotManager::Get().GetOpenSnapshot();
        if (!TraceManager::Get().DataSetValid() || snapshot == nullptr)
        {
            return;
        }

        const uint64_t total_available      = RmtVirtualAllocationListGetTotalSizeInBytes(&snapshot->virtual_allocation_list);
        const uint64_t allocated_and_used   = RmtVirtualAllocationListGetBoundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);
        const uint64_t allocated_and_unused = RmtVirtualAllocationListGetUnboundTotalSizeInBytes(snapshot, &snapshot->virtual_allocation_list);

        SetModelData(kResourceOverviewTotalAvailableSize, rmv::string_util::LocalizedValueMemory(total_available, false, false));
        SetModelData(kResourceOverviewTotalAllocatedAndUsed, rmv::string_util::LocalizedValueMemory(allocated_and_used, false, false));
        SetModelData(kResourceOverviewTotalAllocatedAndUnused, rmv::string_util::LocalizedValueMemory(allocated_and_unused, false, false));

        SetModelData(kResourceOverviewAllocationCount, rmv::string_util::LocalizedValue(snapshot->virtual_allocation_list.allocation_count));
        SetModelData(kResourceOverviewResourceCount, rmv::string_util::LocalizedValue(snapshot->resource_list.resource_count));
    }

    bool ResourceOverviewModel::GetTooltipString(const RmtResource* resource, QString& text_string) const
    {
        if (resource != nullptr)
        {
            text_string = "Id: " + QString::number(resource->identifier);
            if (strlen(resource->name) > 0)
            {
                text_string += "\nName: " + QString(resource->name);
            }
            text_string += "\nSize: " + rmv::string_util::LocalizedValueMemory(resource->size_in_bytes, false, false);

            const uint64_t offset = RmtResourceGetOffsetFromBoundAllocation(resource);
            text_string += "\nOffset: " + rmv::string_util::LocalizedValue(offset);

            const RmtVirtualAllocation* allocation = resource->bound_allocation;
            if (allocation != nullptr)
            {
                text_string += "\nAllocation " + QString::number(static_cast<qulonglong>(allocation->base_address));
                if (resource->identifier == 0)
                {
                    text_string += " (unbound)";
                }
            }
            else
            {
                text_string += "\nNo parent allocation";
            }

            const RmtResourceUsageType usage_type = (resource->identifier != 0) ? RmtResourceGetUsageType(resource) : kRmtResourceUsageTypeFree;
            text_string += "\nUsage: " + rmv::string_util::GetResourceUsageString(usage_type);

            // If the resource type is an image, display dimensions and format information.
            if (resource->resource_type == kRmtResourceTypeImage)
            {
                text_string += "\nImage type: " + QString(RmtGetImageTypeNameFromImageType(resource->image.image_type));
                text_string += "\nDimensions: (" + QString::number(resource->image.dimension_x) + ", " + QString::number(resource->image.dimension_y) + ", " +
                               QString::number(resource->image.dimension_z) + ")";
                text_string += "\nFormat: " + QString(RmtGetFormatNameFromFormat(resource->image.format.format));
            }

            return true;
        }
        return false;
    }
}  // namespace rmv