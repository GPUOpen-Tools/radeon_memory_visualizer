//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the carousel model
//=============================================================================

#ifndef RMV_MODELS_CAROUSEL_MODEL_H_
#define RMV_MODELS_CAROUSEL_MODEL_H_

#include <QString>
#include <QColor>
#include <stdint.h>

#include "rmt_data_snapshot.h"
#include "rmt_resource_list.h"

/// The number of buckets for the allocation sizes carousel item. Currently
/// caters for less than 1MB, then in power of 2 increments up to greater than 1GB.
static const int kNumAllocationSizeBuckets = 12;

/// Specific to the carousel, for the memory footprint component.
struct RMVCarouselMemoryFootprintData
{
    double total_allocated_memory;  ///< Total allocated memory.
    double total_unused_memory;     ///< Total memory that was allocated but not used.
    double max_memory;              ///< Max memory.
};

/// Mapping of resource type to amount.
struct ResourceMapping
{
    RmtResourceUsageType usage_type;    ///< the resource type.
    int32_t              usage_amount;  ///< the amount of this type.
};

/// Specific to the carousel, for the resource types component.
struct RMVCarouselResourceTypesData
{
    int32_t         usage_amount[kRmtResourceUsageTypeCount];  ///< How much was used for this usage (the raw data).
    ResourceMapping usage_map[kRmtResourceUsageTypeCount];     ///< How much was used for this usage (the sort results).
    int32_t         usage_maximum;                             ///< The highest resource value.
};

/// Struct to describe the heap.
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

/// Specific to the carousel, for the memory types component.
struct RMVCarouselMemoryTypesData
{
    HeapData preferred_heap[kRmtHeapTypeCount];  ///< Holds how much of each preferred heap memory type is used and max.
    HeapData physical_heap[kRmtHeapTypeCount];   ///< Holds how much of each physical heap memory type is used and max.
    QString  name[kRmtHeapTypeCount];            ///< The name of each heap.
};

/// Specific to the carousel, for the allocation sizes component.
struct RMVCarouselAllocationSizesData
{
    int32_t num_allocations;                     ///< The total number of allocations in the current snapshot.
    int32_t buckets[kNumAllocationSizeBuckets];  ///< The number of allocations in each bucket.
};

/// Hold all carousel data.
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
    /// Constructor.
    CarouselModel();

    /// Destructor.
    ~CarouselModel();

    /// Parse the dataset for carousel data for the currently opened snapshot.
    /// \param out_carousel_data The data needed for the carousel.
    /// \return true if successful, false if not.
    bool GetCarouselData(RMVCarouselData& out_carousel_data) const;

    /// Compute delta between carousels.
    /// \param base_snapshot The first (base) snapshot.
    /// \param diff_snapshot The second snapshot to compare against the first.
    /// \param out_carousel_delta_data The delta of the carousel data.
    /// \return true if successful, false if not.
    bool CalcGlobalCarouselData(RmtDataSnapshot* base_snapshot, RmtDataSnapshot* diff_snapshot, RMVCarouselData& out_carousel_delta_data);

private:
    /// Parse the dataset for carousel data for a snapshot. Assumes that
    /// out_carousel_data has been initialized to all 0's.
    /// \param snapshot_id target snapshot.
    /// \param out_carousel_data The data needed for the carousel.
    /// \return true if successful, false if not.
    bool GetCarouselData(RmtDataSnapshot* snapshot, RMVCarouselData& out_carousel_data) const;

    /// Calculate which allocation bucket this allocation will go into.
    /// \param allocation_size The current allocation size to add.
    /// \return the bucket index.
    int GetAllocationBucketIndex(uint64_t allocation_size) const;

    /// Get the color based on the memory subscription.
    /// \param segment_status The segment status.
    /// \return The color to use.
    QColor GetColorFromSubscription(const RmtSegmentStatus& segment_status) const;
};

#endif  // RMV_MODELS_CAROUSEL_MODEL_H_
