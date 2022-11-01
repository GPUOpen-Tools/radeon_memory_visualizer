//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel model.
//=============================================================================

#ifndef RMV_MODELS_CAROUSEL_MODEL_H_
#define RMV_MODELS_CAROUSEL_MODEL_H_

#include <QString>
#include <QColor>
#include <stdint.h>

#include "rmt_data_snapshot.h"

namespace rmv
{
    /// The number of buckets for the allocation sizes carousel item. Currently
    /// caters for less than 1MB, then in power of 2 increments up to greater than 1GB.
    static const int kNumAllocationSizeBuckets = 12;

    /// @brief Specific to the carousel, for the memory footprint component.
    struct RMVCarouselMemoryFootprintData
    {
        double total_allocated_memory;  ///< Total allocated memory.
        double total_unused_memory;     ///< Total memory that was allocated but not used.
        double max_memory;              ///< Max memory.
    };

    /// @brief Mapping of resource type to amount.
    struct ResourceMapping
    {
        RmtResourceUsageType usage_type;    ///< The resource type.
        int32_t              usage_amount;  ///< The amount of this type.
    };

    /// @brief Specific to the carousel, for the resource types component.
    struct RMVCarouselResourceTypesData
    {
        int32_t         usage_amount[kRmtResourceUsageTypeCount];  ///< How much was used for this usage (the raw data).
        ResourceMapping usage_map[kRmtResourceUsageTypeCount];     ///< How much was used for this usage (the sort results).
        int32_t         usage_maximum;                             ///< The highest resource value.
    };

    /// @brief Struct to describe the heap.
    struct HeapData
    {
        HeapData()
            : value(0)
            , max(0)
        {
        }

        int64_t value;  ///< The value.
        int64_t max;    ///< Possible maximum.
        QColor  color;  ///< The bar color.
    };

    /// @brief Specific to the carousel, for the memory types component.
    struct RMVCarouselMemoryTypesData
    {
        HeapData preferred_heap[kRmtHeapTypeCount];  ///< Holds how much of each preferred heap memory type is used and max.
        HeapData physical_heap[kRmtHeapTypeCount];   ///< Holds how much of each physical heap memory type is used and max.
        QString  name[kRmtHeapTypeCount];            ///< The name of each heap.
    };

    /// @brief Specific to the carousel, for the allocation sizes component.
    struct RMVCarouselAllocationSizesData
    {
        int32_t num_allocations;                     ///< The total number of allocations in the current snapshot.
        int32_t buckets[kNumAllocationSizeBuckets];  ///< The number of allocations in each bucket.
    };

    /// @brief Hold all carousel data.
    struct RMVCarouselData
    {
        RMVCarouselMemoryFootprintData memory_footprint_data;  ///< Data for mem footprint component.
        RMVCarouselResourceTypesData   resource_types_data;    ///< Data for resource types component.
        RMVCarouselMemoryTypesData     memory_types_data;      ///< Data for mem types component.
        RMVCarouselAllocationSizesData allocation_sizes_data;  ///< Data for allocation sizes component.
    };

    class CarouselModel
    {
    public:
        /// @brief Constructor.
        CarouselModel();

        /// @brief Destructor.
        ~CarouselModel();

        /// @brief Parse the dataset for carousel data for the currently opened snapshot.
        ///
        /// @param [out] out_carousel_data The data needed for the carousel.
        ///
        /// @return true if successful, false if not.
        bool GetCarouselData(RMVCarouselData& out_carousel_data) const;

        /// @brief Compute delta between carousels.
        ///
        /// @param [in]     base_snapshot           The first (base) snapshot.
        /// @param [in]     diff_snapshot           The second snapshot to compare against the first.
        /// @param [out]    out_carousel_delta_data The delta of the carousel data.
        ///
        /// @return true if successful, false if not.
        bool CalcGlobalCarouselData(RmtDataSnapshot* base_snapshot, RmtDataSnapshot* diff_snapshot, RMVCarouselData& out_carousel_delta_data);

    private:
        /// @brief Parse the dataset for carousel data for a snapshot.
        ///
        /// Assumes that out_carousel_data has been initialized to all 0's.
        ///
        /// @param [in]     snapshot          The target snapshot.
        /// @param [in,out] out_carousel_data The data needed for the carousel.
        ///
        /// @return true if successful, false if not.
        bool GetCarouselData(RmtDataSnapshot* snapshot, RMVCarouselData& out_carousel_data) const;

        /// @brief Calculate which allocation bucket this allocation will go into.
        ///
        /// @param [in] allocation_size The current allocation size to add.
        ///
        /// @return the bucket index.
        int GetAllocationBucketIndex(uint64_t allocation_size) const;

        /// @brief Get the color based on the memory subscription.
        ///
        /// @param [in] segment_status The segment status.
        ///
        /// @return The color to use.
        QColor GetColorFromSubscription(const RmtSegmentStatus& segment_status) const;
    };
}  // namespace rmv

#endif  // RMV_MODELS_CAROUSEL_MODEL_H_
