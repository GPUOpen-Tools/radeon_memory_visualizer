//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the memory map model. This handles any data needed
/// from the backend and passes it to the UI when requested
//=============================================================================

#include "models/snapshot/memory_map_model.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"

#include "models/trace_manager.h"
#include "settings/rmv_settings.h"

namespace rmv
{
    MemoryMapModel::MemoryMapModel(Colorizer* colorizer)
        : granularity_(0)
        , num_blocks_(0)
        , block_offset_(0)
        , minimum_virtual_address_(0)
        , maximum_virtual_address_(0)
        , colorizer_(colorizer)
    {
    }

    MemoryMapModel::~MemoryMapModel()
    {
    }

    void MemoryMapModel::UpdateGranularity(int granularity)
    {
        granularity_ = granularity;

        // calculate block sizes based on trim values from the snapshot
        const TraceManager&    trace_manager = TraceManager::Get();
        const RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (trace_manager.DataSetValid() && open_snapshot != nullptr)
        {
            minimum_virtual_address_ = open_snapshot->minimum_virtual_address;
            maximum_virtual_address_ = open_snapshot->maximum_virtual_address;
            RMT_ASSERT(maximum_virtual_address_ >= minimum_virtual_address_);

            block_offset_ = minimum_virtual_address_ / granularity;
            num_blocks_   = (maximum_virtual_address_ - minimum_virtual_address_) / granularity;
        }
    }

    const RmtVirtualAllocation* MemoryMapModel::GetAllocation(uint64_t base_address) const
    {
        const TraceManager&    trace_manager = TraceManager::Get();
        const RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (trace_manager.DataSetValid() && (open_snapshot != nullptr))
        {
            const RmtVirtualAllocation* current_allocation = nullptr;
            RmtErrorCode                error_code =
                RmtVirtualAllocationListGetAllocationForAddress(&open_snapshot->virtual_allocation_list, base_address, &current_allocation);
            if (error_code == RMT_OK && current_allocation != nullptr)
            {
                return current_allocation;
            }
        }
        return nullptr;
    }

    const RmtResource* MemoryMapModel::GetResource(uint64_t base_address) const
    {
        const TraceManager&    trace_manager = TraceManager::Get();
        const RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (trace_manager.DataSetValid() && (open_snapshot != nullptr))
        {
            const RmtVirtualAllocation* current_allocation = nullptr;
            RmtErrorCode                error_code =
                RmtVirtualAllocationListGetAllocationForAddress(&open_snapshot->virtual_allocation_list, base_address, &current_allocation);
            if (error_code == RMT_OK && current_allocation != nullptr)
            {
                // now walk the resources and find one with the base address.
                for (int32_t current_resource_index = 0; current_resource_index < current_allocation->resource_count; ++current_resource_index)
                {
                    const RmtResource* current_resource = current_allocation->resources[current_resource_index];
                    if (current_resource->address <= base_address && base_address < (current_resource->address + current_resource->size_in_bytes))
                    {
                        return current_resource;
                    }
                }

                return nullptr;
            }
        }
        return nullptr;
    }

    uint64_t MemoryMapModel::GetBlockOffset() const
    {
        return block_offset_;
    }

    uint64_t MemoryMapModel::GetNumBlocks() const
    {
        return num_blocks_;
    }

    const QColor MemoryMapModel::GetColor(uint64_t block_offset) const
    {
        uint64_t visible_base_address = block_offset * granularity_;
        visible_base_address += minimum_virtual_address_;

        const RmtVirtualAllocation* current_allocation = GetAllocation(visible_base_address);
        const RmtResource*          current_resource   = GetResource(visible_base_address);

        return colorizer_->GetColor(current_allocation, current_resource);
    }
}  // namespace rmv