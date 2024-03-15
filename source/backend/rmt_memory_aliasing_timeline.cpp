//=============================================================================
// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the aliasing resource memory algorithm.
//=============================================================================

#include "rmt_memory_aliasing_timeline.h"

namespace RmtMemoryAliasingTimelineAlgorithm
{
    // Global instance of the aliased resource memory calculator.
    static RmtMemoryAliasingCalculator* memory_aliasing_calculator_ = nullptr;

    void Allocation::Init(const SizeType size)
    {
        allocation_size_ = size;
        regions_slow_part_.resize(1);
        regions_slow_part_.front() = AllocationRegionSlowPart{};  // Clear
        regions_fast_part_.resize(1);
        regions_fast_part_.front().non_zero_counters_ = 0;
        RMT_ASSERT(regions_fast_part_.front().begin_offset_ == 0);
        total_sizes_valid_ = false;
    }

    void Allocation::CreateResource(const SizeType offset, const SizeType size, const RmtResourceUsageType res_usage_type)
    {
        const SizeType end_offset = offset + size;
        RMT_ASSERT(end_offset <= allocation_size_);

        // Binary search
        const auto it =
            std::lower_bound(regions_fast_part_.begin(), regions_fast_part_.end(), offset, [](const AllocationRegionFastPart& lhs, SizeType rhsOffset) {
                return lhs.begin_offset_ < rhsOffset;
            });
        size_t index = std::distance(regions_fast_part_.begin(), it);

        if (index == regions_fast_part_.size() || regions_fast_part_[index].begin_offset_ > offset)
        {
            // Previous region needs to be split.
            RMT_ASSERT(index > 0);
            --index;
            AllocationRegionSlowPart new_region_slow = regions_slow_part_[index];  // Copy
            AllocationRegionFastPart new_region_fast = regions_fast_part_[index];  // Copy
            new_region_fast.begin_offset_            = offset;
            ++index;
            regions_slow_part_.insert(regions_slow_part_.begin() + index, new_region_slow);  // Insert `new_region` after `it`.
            regions_fast_part_.insert(regions_fast_part_.begin() + index, new_region_fast);
        }

        while (GetRegionEndOffset(index) <= end_offset)
        {
            IncrementRegion(index, res_usage_type);
            ++index;
            if (index == regions_fast_part_.size())
            {
                break;
            }
        }

        if (index != regions_fast_part_.size() && end_offset > regions_fast_part_[index].begin_offset_)
        {
            // `end_offset` of the new region falls inside the current region `it`.
            // We need to split it into 2 parts: first will be existing `it`, second will be `new_region`.
            AllocationRegionSlowPart new_region_slow = regions_slow_part_[index];  // Copy
            AllocationRegionFastPart new_region_fast = regions_fast_part_[index];  // Copy
            new_region_fast.begin_offset_            = end_offset;
            IncrementRegion(index, res_usage_type);
            ++index;
            regions_slow_part_.insert(regions_slow_part_.begin() + index, new_region_slow);  // Insert `new_region` after `it`.
            regions_fast_part_.insert(regions_fast_part_.begin() + index, new_region_fast);
        }

        total_sizes_valid_ = false;
    }

    void Allocation::DestroyResource(const SizeType offset, const SizeType size, const RmtResourceUsageType res_usage_type)
    {
        const SizeType end_offset = offset + size;
        RMT_ASSERT(end_offset <= allocation_size_);

        // Binary search.
        const auto it =
            std::lower_bound(regions_fast_part_.begin(), regions_fast_part_.end(), offset, [](const AllocationRegionFastPart& lhs, SizeType rhsOffset) {
                return lhs.begin_offset_ < rhsOffset;
            });
        RMT_ASSERT(it != regions_fast_part_.end() && it->begin_offset_ == offset);
        size_t index = std::distance(regions_fast_part_.begin(), it);

        do
        {
            DecrementRegion(index, res_usage_type);
            ++index;
        } while (index != regions_fast_part_.size() && regions_fast_part_[index].begin_offset_ < end_offset);

        total_sizes_valid_ = false;
    }

    void Allocation::AddSizes(SizePerResourceUsageType& inout_sizes, SizeType& inout_unbound_size)
    {
        EnsureTotalSizes();
        inout_sizes += total_sizes_per_resource_;
        inout_unbound_size += allocation_size_ - total_bound_size_;
    }

    void Allocation::EnsureTotalSizes()
    {
        if (total_sizes_valid_)
        {
            return;
        }

        total_sizes_per_resource_ = SizePerResourceUsageType{};  // Clear
        total_bound_size_         = 0;

        for (size_t index = 0, count = regions_fast_part_.size(); index < count; ++index)
        {
            const AllocationRegionFastPart& region      = regions_fast_part_[index];
            const SizeType                  region_size = GetRegionEndOffset(index) - region.begin_offset_;
            const RmtResourceUsageType      res_type    = GetHighestPriorityType(index);
            if (res_type < RmtResourceUsageType::kRmtResourceUsageTypeCount)
            {
                total_sizes_per_resource_.size_[res_type] += region_size;
                total_bound_size_ += region_size;
            }
        }

        total_sizes_valid_ = true;
    }

    AllocationPool::~AllocationPool()
    {
        for (auto alloc : all_allocations_)
        {
            delete alloc;
        }
    }

    Allocation* AllocationPool::CreateAllocation()
    {
        Allocation* alloc;

        if (unused_allocations_.empty())
        {
            alloc = new Allocation{};
            all_allocations_.push_back(alloc);
        }
        else
        {
            alloc = unused_allocations_.back();
            unused_allocations_.pop_back();
        }

        return alloc;
    }

    void AllocationPool::DestroyAllocation(Allocation* alloc)
    {
        unused_allocations_.push_back(alloc);
    }

    RmtMemoryAliasingCalculator::RmtMemoryAliasingCalculator()
    {
    }

    RmtMemoryAliasingCalculator::~RmtMemoryAliasingCalculator()
    {
    }

    void RmtMemoryAliasingCalculator::CreateAllocation(const AllocationIdType id, const SizeType size)
    {
        Allocation* alloc = allocation_pool_.CreateAllocation();
        alloc->Init(size);
        allocations_.insert({id, alloc});
    }

    void RmtMemoryAliasingCalculator::DestroyAllocation(const AllocationIdType id)
    {
        auto alloc_it = allocations_.find(id);
        RMT_ASSERT(alloc_it != allocations_.end());
        allocation_pool_.DestroyAllocation(alloc_it->second);
        allocations_.erase(alloc_it);
    }

    Allocation* RmtMemoryAliasingCalculator::FindAllocation(const AllocationIdType id) const
    {
        const auto it = allocations_.find(id);
        return it != allocations_.end() ? it->second : nullptr;
    }

    void RmtMemoryAliasingCalculator::CalculateSizes(SizePerResourceUsageType& out_sizes, SizeType& out_unbound_size) const
    {
        out_sizes        = SizePerResourceUsageType{};  // Zero-initalize.
        out_unbound_size = 0;

        for (const auto& alloc_it : allocations_)
        {
            alloc_it.second->AddSizes(out_sizes, out_unbound_size);
        }
    }

    void RmtMemoryAliasingCalculatorCleanup()
    {
        delete memory_aliasing_calculator_;
        memory_aliasing_calculator_ = nullptr;
    }

    RmtMemoryAliasingCalculator* RmtMemoryAliasingCalculatorInstance()
    {
        if (memory_aliasing_calculator_ == nullptr)
        {
            memory_aliasing_calculator_ = new RmtMemoryAliasingCalculator;
        }

        return memory_aliasing_calculator_;
    }

}  // namespace RmtMemoryAliasingTimelineAlgorithm
