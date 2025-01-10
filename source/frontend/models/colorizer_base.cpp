//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the colorizer base class.
///
/// Derived classes of this will implement the "color by" combo boxes
/// throughout the UI and the colorizing of the timeline.
///
//=============================================================================

#include "models/colorizer_base.h"

#include <QString>

#include "qt_common/utils/qt_util.h"

#include "rmt_data_snapshot.h"
#include "rmt_print.h"

#include "managers/trace_manager.h"
#include "settings/rmv_settings.h"
#include "util/string_util.h"
#include "util/widget_util.h"

namespace rmv
{
    // The number of age buckets.
    static const int kNumAllocationAgeBuckets = 10;

    ColorizerBase::ColorizerBase()
        : combo_box_(nullptr)
        , legends_scene_(nullptr)
        , legends_view_(nullptr)
        , color_mode_(kColorModeResourceUsageType)
        , color_mode_map_{}
    {
        for (int count = 0; count < kColorModeCount; count++)
        {
            color_mode_map_[count] = kColorModeCount;
        }
    }

    ColorizerBase::~ColorizerBase()
    {
        delete legends_scene_;
    }

    void ColorizerBase::Initialize(ArrowIconComboBox* combo_box, QGraphicsView* legends_view)
    {
        combo_box_ = combo_box;

        // Make sure the legends view is fixed-size.
        rmv::widget_util::InitGraphicsView(legends_view, rmv::kColoredLegendsHeight);

        legends_scene_ = rmv::widget_util::InitColorLegend(legends_view);
        legends_view_  = legends_view;
        UpdateLegends();
    }

    QColor ColorizerBase::GetAgeColor(int32_t age_index) const
    {
        struct Color
        {
            double r;
            double g;
            double b;
        };

        // Source and destination rgb color values.
        static const Color kSrc  = {34, 68, 48};
        static const Color kDest = {240, 240, 240};

        double range = GetNumAgeBuckets();
        double lerp  = std::min<double>(1.0, static_cast<double>(age_index) / range);
        lerp         = std::max<double>(0.0, lerp);

        double r = (kSrc.r * (1.0 - lerp)) + (kDest.r * lerp);
        double g = (kSrc.g * (1.0 - lerp)) + (kDest.g * lerp);
        double b = (kSrc.b * (1.0 - lerp)) + (kDest.b * lerp);

        return QColor::fromRgb(r, g, b);
    }

    QColor ColorizerBase::GetHeapColor(RmtHeapType heap_type)
    {
        switch (heap_type)
        {
        case kRmtHeapTypeLocal:
            return rmv::RMVSettings::Get().GetColorHeapLocal();

        case kRmtHeapTypeInvisible:
            return rmv::RMVSettings::Get().GetColorHeapInvisible();

        case kRmtHeapTypeSystem:
            return rmv::RMVSettings::Get().GetColorHeapSystem();

        case kRmtHeapTypeNone:
            return rmv::RMVSettings::Get().GetColorHeapUnspecified();

        default:
            return rmv::RMVSettings::Get().GetColorResourceFreeSpace();
        }
    }

    QColor ColorizerBase::GetResourceUsageColor(RmtResourceUsageType usage_type)
    {
        QColor out = Qt::black;

        switch (usage_type)
        {
        case kRmtResourceUsageTypeDepthStencil:
            out = rmv::RMVSettings::Get().GetColorResourceDepthStencil();
            break;
        case kRmtResourceUsageTypeRenderTarget:
            out = rmv::RMVSettings::Get().GetColorResourceRenderTarget();
            break;
        case kRmtResourceUsageTypeTexture:
            out = rmv::RMVSettings::Get().GetColorResourceTexture();
            break;
        case kRmtResourceUsageTypeRayTracingBuffer:
            out = rmv::RMVSettings::Get().GetColorResourceRayTracingBuffer();
            break;
        case kRmtResourceUsageTypeShaderPipeline:
            out = rmv::RMVSettings::Get().GetColorResourceShaderPipeline();
            break;
        case kRmtResourceUsageTypeCommandBuffer:
            out = rmv::RMVSettings::Get().GetColorResourceCommandBuffer();
            break;
        case kRmtResourceUsageTypeHeap:
            out = rmv::RMVSettings::Get().GetColorResourceHeap();
            break;
        case kRmtResourceUsageTypeDescriptors:
            out = rmv::RMVSettings::Get().GetColorResourceDescriptors();
            break;
        case kRmtResourceUsageTypeBuffer:
            out = rmv::RMVSettings::Get().GetColorResourceBuffer();
            break;
        case kRmtResourceUsageTypeGpuEvent:
            out = rmv::RMVSettings::Get().GetColorResourceGPUEvent();
            break;
        case kRmtResourceUsageTypeFree:
            out = rmv::RMVSettings::Get().GetColorResourceFreeSpace();
            break;
        case kRmtResourceUsageTypeInternal:
            out = rmv::RMVSettings::Get().GetColorResourceInternal();
            break;

        default:
            out = QtCommon::QtUtils::ColorTheme::Get().GetCurrentThemeColors().graphics_scene_text_color;
            break;
        }

        return out;
    }

    QColor ColorizerBase::GetColor(const uint32_t color_index)
    {
        switch (color_mode_)
        {
        case kColorModePreferredHeap:
        case kColorModeActualHeap:
            return GetHeapColor((RmtHeapType)color_index);

        case kColorModeResourceUsageType:
            return GetResourceUsageColor(RmtResourceUsageType(color_index));

        default:
            break;
        }

        // Default is free.
        return rmv::RMVSettings::Get().GetColorResourceFreeSpace();
    }

    QColor ColorizerBase::GetColor(const RmtVirtualAllocation* const allocation, const RmtResource* const resource) const
    {
        switch (color_mode_)
        {
        case kColorModePreferredHeap:
            if (allocation != nullptr)
            {
                return GetHeapColor(allocation->heap_preferences[0]);
            }
            break;

        case kColorModeActualHeap:
        {
            if (rmv::TraceManager::Get().DataSetValid())
            {
                const RmtDataSnapshot* snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();
                if (snapshot != nullptr)
                {
                    RmtHeapType heap_type = RmtResourceGetActualHeap(snapshot, resource);
                    return GetHeapColor(heap_type);
                }
            }
            break;
        }

        case kColorModeResourceUsageType:
            if (allocation != nullptr)
            {
                if (allocation->resource_count == 0)
                {
                    return rmv::RMVSettings::Get().GetColorResourceFreeSpace();
                }
                if (resource != nullptr)
                {
                    if (resource->identifier != 0)
                    {
                        return GetResourceUsageColor(RmtResourceGetUsageType(resource));
                    }
                }
                return rmv::RMVSettings::Get().GetColorResourceFreeSpace();
            }
            break;

        case kColorModeAllocationAge:
            if (allocation != nullptr)
            {
                int32_t age = GetAgeIndex(allocation->timestamp);
                if (age != -1)
                {
                    return GetAgeColor(age);
                }
            }
            break;

        case kColorModeResourceCreateAge:
            if (resource != nullptr)
            {
                int32_t age = GetAgeIndex(resource->create_time);
                if (age != -1)
                {
                    return GetAgeColor(age);
                }
            }
            break;

        case kColorModeResourceBindAge:
            if (resource != nullptr)
            {
                int32_t age = GetAgeIndex(resource->bind_time);
                if (age != -1)
                {
                    return GetAgeColor(age);
                }
            }
            break;

        case kColorModeResourceGUID:
            if (resource != nullptr)
            {
                srand(resource->identifier);
                return QColor::fromRgb(rand() % 255, rand() % 255, rand() % 255);
            }
            break;

        case kColorModeResourceCPUMapped:
            if (resource != nullptr && resource->bound_allocation != nullptr)
            {
                if ((resource->bound_allocation->flags & kRmtAllocationDetailIsCpuMapped) == kRmtAllocationDetailIsCpuMapped)
                    return rmv::RMVSettings::Get().GetColorCPUMapped();
                else
                    return rmv::RMVSettings::Get().GetColorNotCPUMapped();
            }
            break;

        case kColorModeNotAllPreferred:
        {
            const RmtDataSnapshot* open_snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();
            if (open_snapshot != nullptr)
            {
                uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};
                RmtResourceGetBackingStorageHistogram(open_snapshot, resource, memory_segment_histogram);

                if (!resource || !resource->bound_allocation || resource->resource_type == kRmtResourceTypeCount)
                    return rmv::RMVSettings::Get().GetColorResourceFreeSpace();

                // Check that the preferred heap contains all the bytes.
                const RmtHeapType preferred_heap = resource->bound_allocation->heap_preferences[0];
                if (memory_segment_histogram[preferred_heap] != resource->size_in_bytes && preferred_heap != kRmtHeapTypeNone)
                    return rmv::RMVSettings::Get().GetColorNotInPreferredHeap();
                else
                    return rmv::RMVSettings::Get().GetColorInPreferredHeap();
            }
        }
        break;

        case kColorModeAliasing:
        {
            const RmtDataSnapshot* open_snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();
            if (open_snapshot != nullptr)
            {
                uint64_t memory_segment_histogram[kRmtResourceBackingStorageCount] = {0};
                RmtResourceGetBackingStorageHistogram(open_snapshot, resource, memory_segment_histogram);

                if (!resource || !resource->bound_allocation || resource->resource_type == kRmtResourceTypeCount)
                    return rmv::RMVSettings::Get().GetColorResourceFreeSpace();

                // Check if the resource is aliased.
                if (RmtResourceGetAliasCount(resource) > 0)
                {
                    return rmv::RMVSettings::Get().GetColorAliased();
                }
                else
                {
                    return rmv::RMVSettings::Get().GetColorNotAliased();
                }
            }
        }
        break;

        case kColorModeCommitType:
            if (resource != nullptr && resource->bound_allocation != nullptr)
            {
                switch (resource->commit_type)
                {
                case kRmtCommitTypeCommitted:
                    return rmv::RMVSettings::Get().GetColorCommitTypeCommitted();
                case kRmtCommitTypePlaced:
                    return rmv::RMVSettings::Get().GetColorCommitTypePlaced();
                case kRmtCommitTypeVirtual:
                    return rmv::RMVSettings::Get().GetColorCommitTypeVirtual();
                default:
                    break;
                }
            }
            break;

        default:
            break;
        }

        // Default is free.
        return rmv::RMVSettings::Get().GetColorResourceFreeSpace();
    }

    void ColorizerBase::UpdateLegends()
    {
        UpdateLegends(legends_scene_, color_mode_);
    }

    void ColorizerBase::UpdateLegends(ColoredLegendScene* legends_scene, ColorMode color_mode)
    {
        Q_ASSERT(legends_scene != nullptr);

        legends_scene->Clear();

        switch (color_mode)
        {
        case kColorModePreferredHeap:
        case kColorModeActualHeap:
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorHeapSystem(), RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeSystem));
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorHeapLocal(), RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeLocal));
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorHeapInvisible(), RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeInvisible));
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorHeapUnspecified(), RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeNone));
            break;

        case kColorModeResourceUsageType:
        {
            // Note: Usage types on the legend are drawn in reverse order so that highest aliased priority usage are on the left, lowest on the right.
            for (int32_t index = kRmtResourceUsageTypeCount - 1; index > -1; --index)
            {
                if (index != kRmtResourceUsageTypeUnknown)
                {
                    const RmtResourceUsageType resource_usage_type = (RmtResourceUsageType)index;
                    legends_scene->AddColorLegendItem(GetResourceUsageColor(resource_usage_type),
                                                      RmtGetResourceUsageTypeNameFromResourceUsageType(resource_usage_type));
                }
            }
        }
        break;

        case kColorModeAllocationAge:
        case kColorModeResourceCreateAge:
        case kColorModeResourceBindAge:
        {
            int num_resource_age_buckets = GetNumAgeBuckets();
            for (int32_t current_allocation_age_range_bucket_index = 0; current_allocation_age_range_bucket_index < num_resource_age_buckets;
                 ++current_allocation_age_range_bucket_index)
            {
                QString text;
                if (current_allocation_age_range_bucket_index == 0)
                {
                    text = QString("Oldest");
                }
                else if (current_allocation_age_range_bucket_index == num_resource_age_buckets - 1)
                {
                    text = QString("Youngest");
                }
                legends_scene->AddColorLegendItem(GetAgeColor(current_allocation_age_range_bucket_index), text);
            }
        }
        break;

        case kColorModeResourceGUID:
            legends_scene->AddTextLegendItem("Each color represents a different resource.");
            break;

        case kColorModeResourceCPUMapped:
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorCPUMapped(), "CPU mapped");
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorNotCPUMapped(), "Not CPU mapped");
            break;

        case kColorModeNotAllPreferred:
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorInPreferredHeap(), "All in preferred heap");
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorNotInPreferredHeap(), "Not all in preferred heap");
            break;

        case kColorModeAliasing:
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorAliased(), "Aliased");
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorNotAliased(), "Not aliased");
            break;

        case kColorModeCommitType:
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorCommitTypeCommitted(), "Committed");
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorCommitTypePlaced(), "Placed");
            legends_scene->AddColorLegendItem(rmv::RMVSettings::Get().GetColorCommitTypeVirtual(), "Virtual");
            break;

        default:
            break;
        }

        // Set the view sizes to match the scene sizes so the legends appear left-justified.
        if (legends_view_ != nullptr && legends_scene != nullptr)
        {
            legends_view_->setFixedSize(legends_scene->itemsBoundingRect().size().toSize());
        }
    }

    int32_t ColorizerBase::GetNumAgeBuckets()
    {
        return kNumAllocationAgeBuckets;
    }

    int32_t ColorizerBase::GetAgeIndex(uint64_t timestamp)
    {
        const RmtDataSnapshot* snapshot = rmv::SnapshotManager::Get().GetOpenSnapshot();
        if (snapshot == nullptr)
        {
            return -1;
        }

        uint64_t       age_range    = snapshot->maximum_allocation_timestamp - snapshot->minimum_allocation_timestamp;
        const uint64_t bucket_width = age_range / GetNumAgeBuckets();
        if (bucket_width == 0)
        {
            return -1;
        }

        const uint64_t allocation_age = timestamp - snapshot->minimum_allocation_timestamp;
        int32_t        result         = allocation_age / bucket_width;
        result                        = std::min(result, (GetNumAgeBuckets() - 1));
        result                        = std::max(result, 0);
        return result;
    }
}  // namespace rmv
