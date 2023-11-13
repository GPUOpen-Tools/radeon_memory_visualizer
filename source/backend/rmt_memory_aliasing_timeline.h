//=============================================================================
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definitions for the aliasing memory aliasing algorithm.
//=============================================================================

#ifndef RMV_BACKEND_RMT_MEMORY_ALIASING_TIMELINE_H_
#define RMV_BACKEND_RMT_MEMORY_ALIASING_TIMELINE_H_

#include "rmt_assert.h"
#include "rmt_format.h"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <vector>

namespace RmtMemoryAliasingTimelineAlgorithm
{
    /// Type to be used as an identifier for allocations.
    typedef uint64_t AllocationIdType;

    /// Type to be used for all memory sizes and offsets, expressed in bytes.
    typedef uint64_t SizeType;

    /// Type to be used for counting the number of resources of a certain type with aliasing at the same place in memory.
    /// 16 bits should be enough. We don't expect more than 65535 resources exist in the same place at the same time.
    typedef uint16_t ResCountUsageType;

    /// Type to be used to store bit flags per resource type.
    /// Must have enough bits to accommodate for memory types defined by enum RmtResourceUsageType.
    typedef uint32_t ResBitmaskUsageType;

    static_assert(sizeof(ResBitmaskUsageType) * 8 >= RmtResourceUsageType::kRmtResourceUsageTypeCount);

    /// @brief Scans integer for index of first nonzero value from the Most Significant Bit (MSB). If mask is 0 then returns UINT8_MAX.
    ///
    /// Implementation taken from Vulkan Memory Allocator by AMD.
    ///
    /// @param [in] mask                             The bit mask to scan.
    inline uint8_t BitScanMSB(uint32_t mask)
    {
#ifdef _MSC_VER
        unsigned long pos;
        if (_BitScanReverse(&pos, mask))
        {
            return static_cast<uint8_t>(pos);
        }

#elif defined __GNUC__ || defined __clang__
        if (mask)
        {
            return 31 - static_cast<uint8_t>(__builtin_clz(mask));
        }
#else
        uint8_t  pos = 31;
        uint32_t bit = 1UL << 31;
        do
        {
            if (mask & bit)
            {
                return pos;
            }
            bit >>= 1;
        } while (pos-- > 0);
#endif
        return UINT8_MAX;
    }

    /// Stores array of counters per resource type.
    /// For given resource type as index, it stores the number of resources of that type.
    struct CounterPerResourceUsageType
    {
        ResCountUsageType counter_[RmtResourceUsageType::kRmtResourceUsageTypeCount] = {};

        bool operator==(const CounterPerResourceUsageType& rhs) const
        {
            return memcmp(counter_, rhs.counter_, sizeof(ResCountUsageType) * RmtResourceUsageType::kRmtResourceUsageTypeCount) == 0;
        }
    };

    /// A structure used to calculate memory sizes based on overlapped aliased resources.
    struct SizePerResourceUsageType
    {
        SizeType size_[RmtResourceUsageType::kRmtResourceUsageTypeCount] = {};

        SizePerResourceUsageType& operator+=(const SizePerResourceUsageType& rhs)
        {
            for (size_t i = 0; i < RmtResourceUsageType::kRmtResourceUsageTypeCount; ++i)
                this->size_[i] += rhs.size_[i];
            return *this;
        }
    };

    /// Stores part of the information for a distinct region of memory.
    /// This part is intended to be small and accessed frequently - on each recalculation of the result.
    struct AllocationRegionFastPart
    {
        /// Offset from the beginning of the allocation to the beginning of this region, in bytes.
        SizeType begin_offset_ = 0;

        /// The non_zero_counters bit i is set when equivalent regions_slow_part[same_index].counters.counter[i] > 0.
        ResBitmaskUsageType non_zero_counters_ = 0;

        bool operator<(const AllocationRegionFastPart& rhs) const
        {
            return begin_offset_ < rhs.begin_offset_;
        }
    };

    /// Stores part of the information for a distinct region of memory.
    /// This part is intended to be large and accessed infrequently - only when an allocation is added or removed.
    struct AllocationRegionSlowPart
    {
        CounterPerResourceUsageType counters_;  ///< Counter for each resource usage type.
    };

    /// @brief Represents a single virtual allocation with specific size in bytes.
    ///
    /// It stores a sequence of regions. Regions follow each other. Each region spans
    /// from its offset to the offset of the next region, or the end of the allocation
    /// if it is the last region. First region always starts at offset 0. Each region
    /// stores counters to remember how many resources of given type exist in that
    /// region. Begin or end of a resource enforces begin of a new region. After
    /// removing resources, regions with equal counters are not merged back into one.
    /// Such merge would be incorrect in certain cases.
    struct Allocation
    {
        /// @brief Initializes this allocation object.
        ///
        /// Should be always called after the object is constructed.
        ///
        /// @param [in] size                        The size of the allocation being tracked.
        void Init(const SizeType size);

        /// @brief Adds a resource to the allocation.
        ///
        /// Internal data structures are updated accordingly, using the main algorithm.
        ///
        /// @param [in] offset                      The offset in bytes from the start of the virtual allocation the resource is bound to.
        /// @param [in] size                        The size in bytes of the resource.
        /// @param [in] res_usage_type              The usage type of the resource.
        void CreateResource(const SizeType offset, const SizeType size, const RmtResourceUsageType res_usage_type);

        /// @brief Removes a resource from the allocation.
        ///
        /// Internal data structures are updated accordingly, using the main algorithm.
        /// Parameters of the resource must exactly match former call to CreateResource() on the same allocation!
        /// Otherwise, effects are undefined.
        ///
        /// @param [in] offset                      The starting offset of the region to be deleted.
        /// @param [in] size                        The size in bytes of the region to be deleted.
        /// @param [in] res_usage_type              The resource usage type.
        void DestroyResource(const SizeType offset, const SizeType size, const RmtResourceUsageType res_usage_type);

        /// @brief Increments resource usage sizes.
        ///
        /// @param [in/out] inout_sizes             The array of sizes to add for each of the resource usage types.  Updated with the new sizes on exit.
        /// @param [in/out] inout_unbound_size      The size in bytes of unbound virtual allocation space to add.  Updated with the new unbound size on exit.
        void AddSizes(SizePerResourceUsageType& inout_sizes, SizeType& inout_unbound_size);

    private:
        /// Size of this allocation, in bytes.
        SizeType allocation_size_ = 0;

        /// Arrays of structures that keep information about regions.
        /// Both vectors always have equal length.
        /// They are always sorted by regions_fast_part_[i].begin_offset.
        /// Full information about the region is stored in two parts: regions_fast_part_[i] plus regions_slow_part_[i].
        /// It is split into two as a performance optimization - to optimize memory access
        /// pattern for better cache utilization.
        std::vector<AllocationRegionFastPart> regions_fast_part_;  ///< Used when a resource is bound or destroyed.
        std::vector<AllocationRegionSlowPart> regions_slow_part_;  ///< Used when an allocation is added or removed.

        /// Calculated and cached total sizes per resource type, which consider aliasing.
        /// Valid only when total_sizes_valid_ is true.
        SizePerResourceUsageType total_sizes_per_resource_;

        /// Calculated and cached total size of regions where any resource is bound.
        /// (allocation_size_ - total_bound_size_) gives the unbound size of this allocation.
        /// Valid only when total_sizes_valid_ is true.
        SizeType total_bound_size_ = 0;

        /// True when total_sizes_per_resource and total_bound_size are calculated and up-to-date.
        /// False if they are uninitialized or outdated and should not be used, but recalculated if needed.
        bool total_sizes_valid_ = false;

        /// @brief Returns end offset (one byte past the end) of the region with given index.
        ///
        /// It is calculated based on the begin offset of the next region or the size
        /// of the entire allocation, if it is the last region.
        ///
        /// @param [in] index                       The index of a region.
        ///
        /// @returns The offset to the next byte after the end of a region.
        SizeType GetRegionEndOffset(const size_t index) const
        {
            return index + 1 == regions_fast_part_.size() ? allocation_size_ : regions_fast_part_[index + 1].begin_offset_;
        }

        /// @brief Returns type of the resource that exist (has non-zero counter) in the region with given index.
        ///
        /// If multiple resource types exist in the region, returns the type that has the highest priority.
        /// If all counters are zero (region is empty, no resources exist there), returns kRmtResourceUsageTypeCount.
        ///
        /// @param [in] region_index                The index of the region to examine.
        ///
        /// @returns The resource usage type with the highest priority for this region.
        RmtResourceUsageType GetHighestPriorityType(const size_t region_index) const
        {
            const ResBitmaskUsageType bitmask = regions_fast_part_[region_index].non_zero_counters_;

            if (bitmask == 0)
            {
                return RmtResourceUsageType::kRmtResourceUsageTypeCount;
            }

            return (RmtResourceUsageType)BitScanMSB(bitmask);
        }

        /// @brief Internal, ancillary function that updates data structures accordingly.
        ///
        /// Increments counters to reflect adding new resource of given type to the region with given index.
        ///
        /// @param [in] region_index                The index of the region being updated.
        /// @param [in] res_usage_type              The resource usage type of the counter being incremented.
        void IncrementRegion(const size_t region_index, const RmtResourceUsageType res_usage_type)
        {
            RMT_ASSERT(regions_slow_part_[region_index].counters_.counter_[res_usage_type] < std::numeric_limits<ResCountUsageType>::max());
            ++regions_slow_part_[region_index].counters_.counter_[res_usage_type];
            regions_fast_part_[region_index].non_zero_counters_ |= (ResBitmaskUsageType)(1u << res_usage_type);  // Set the bit.
        }

        /// @brief Internal, ancillary function that updates data structures accordingly.
        ///
        /// Decrements counters to reflect removing a resource of given type from the region with given index.
        ///
        /// @param [in] region_index                The index of the region to update.
        /// @param [in] res_usage_type              The resource usage type of the counter being decremented.
        void DecrementRegion(const size_t region_index, const RmtResourceUsageType res_usage_type)
        {
            RMT_ASSERT(regions_slow_part_[region_index].counters_.counter_[res_usage_type] > 0);
            if (--regions_slow_part_[region_index].counters_.counter_[res_usage_type] == 0)
            {
                regions_fast_part_[region_index].non_zero_counters_ &= (ResBitmaskUsageType) ~(1u << res_usage_type);  // Clear the bit.
            }
        }

        /// @brief Ensures that total_sizes_per_resource_ and total_bound_size_ are valid.
        ///
        /// If total_sizes_valid_ is false, recalculates them by traversing all the
        /// regions in this allocation and summing them up. This is the main algorithm
        /// for result calculation.
        void EnsureTotalSizes();
    };

    /// @brief Internal class used to own and create objects of Allocation class.
    ///
    /// Each object of class AliasingResMemCalculator has one object of this class.
    /// It saves Allocation objects that were freed as unused instead of deleting them so they can be reused,
    /// as a performance optimization.
    class AllocationPool
    {
    public:
        /// @brief AllocationPool destructor.
        ~AllocationPool();

        /// @brief Creates a new Allocation object.
        ///
        /// In reality, it may return one from currently existing but unused objects, but this is opaque to the user.
        ///
        /// @returns
        Allocation* CreateAllocation();

        /// @brief Destroys given Allocation object.
        ///
        /// In reality, it just saves it to the list of unused objects, but this is opaque to the user.
        ///
        /// @param [in] alloc                       A pointer to the allocation tracker to be destroyed.
        ///
        void DestroyAllocation(Allocation* alloc);

    private:
        std::vector<Allocation*> all_allocations_;     ///< All of the allocation objects being tracked.  Objects are freed by the AllocationPool destructor.
        std::vector<Allocation*> unused_allocations_;  ///< List of free allocation objects that can be reused.
    };

    /// @brief Main class for calculating total resource usage type sizes.
    ///
    /// Represents a data structure and algorithm to be used for calculating total sizes of resources of various
    /// types in an entire set of allocations. Create one object of this class for the time you do the calculation.
    /// It doesn't have a notion of time. It just stores the current state. As you process events that tell about
    /// new allocations created, freed, resources inside allocations created, destroyed, call appropriate methods
    /// of this class to submit this event and update the current state.
    ///
    /// You can also call method CalculateSizes() at any given moment to calculate and get the result - resource
    /// sizes summed up per resource type, based on the current state, as well as unbound size.
    class RmtMemoryAliasingCalculator
    {
    public:
        /// @brief Constructor for the RmtMemoryAliasingCalculator class.
        RmtMemoryAliasingCalculator();

        /// @brief Destructor for the RmtMemoryAliasingCalculator class.
        ~RmtMemoryAliasingCalculator();

        /// @brief Updates current state to reflect that a new allocation has been created with given size.
        ///
        /// @param [in] id                          A unique identifier for the allocation.
        /// @param [in] size                        The new size in bytes of the allocation.
        void CreateAllocation(const AllocationIdType id, const SizeType size);

        /// @brief Updates current state to reflect that an allocation with given id has been freed.
        ///
        /// @param [in] id                          The allocation identifier of the allocation being destroyed.
        void DestroyAllocation(const AllocationIdType id);

        /// @brief Remember returned pointer if you can instead of searching for it every time.
        ///
        /// @param [in] id                          A unique ID representing an allocation.
        ///
        /// @returns A pointer to the allocation found or nullptr if the id was not found.
        Allocation* FindAllocation(const AllocationIdType id) const;

        /// @brief Calculates and returns total sizes per resource type and unbound size based on the current state.
        ///
        /// @param [out] out_sizes                  The array of calculated resource usage sizes for all allocations.
        /// @param [out] out_unbound_size           The amount of unbound memory for all allocations.
        void CalculateSizes(SizePerResourceUsageType& out_sizes, SizeType& out_unbound_size) const;

    private:
        AllocationPool                                    allocation_pool_;  ///< A pool of Allocation objects and a mechanism to create them.
        std::unordered_map<AllocationIdType, Allocation*> allocations_;      ///< A map of allocations that currently exist.
    };

    /// @brief Retrieves the global instance of the aliasing resource memory calculator.
    ///
    /// A new instance is created if one doesn't already exist.
    ///
    /// @returns A pointer to the RmtMemoryAliasingCalculator object.
    RmtMemoryAliasingCalculator* RmtMemoryAliasingCalculatorInstance();

    /// @brief Deletes the global instance of the aliasing resource memory calculator.
    void RmtMemoryAliasingCalculatorCleanup();

}  // namespace RmtMemoryAliasingTimelineAlgorithm
#endif  // #ifdef RMV_BACKEND_RMT_MEMORY_ALIASING_TIMELINE_H_
