//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the timeline colorizer control.
///
/// The colorizer is responsible for the functionality for the coloring the
/// timeline depending on the timeline type. It sets up the timeline type
/// combo box with the required timeline types currently supported by the
/// backend and updates the timeline and the legends depending on which
/// coloring mode is required.
///
//=============================================================================

#include "models/timeline/timeline_colorizer.h"

#include <QString>

#include "rmt_assert.h"
#include "rmt_print.h"

#include "util/widget_util.h"

namespace rmv
{
    TimelineColorizer::TimelineColorizer()
        : timeline_type_(kRmtDataTimelineTypeResourceUsageVirtualSize)
        , timeline_type_map_{}
    {
    }

    TimelineColorizer::~TimelineColorizer()
    {
    }

    void TimelineColorizer::Initialize(QWidget*                   parent,
                                       ArrowIconComboBox*         combo_box,
                                       ColoredLegendGraphicsView* legends_view,
                                       const RmtDataTimelineType* type_list)
    {
        // Map of timeline type to text string. These need to match the RmtDataTimelineType struct.
        struct TimelineInfo
        {
            QString   text;
            ColorMode color_mode;
        };

        static std::vector<TimelineInfo> type_info = {{QString("Process view"), kColorModeCount},
                                                      {QString("Page size view"), kColorModeCount},
                                                      {QString("Committed view"), kColorModePreferredHeap},
                                                      {QString("Resource usage count view"), kColorModeResourceUsageType},
                                                      {QString("Resource usage size view"), kColorModeResourceUsageType},
                                                      {QString("Paging view"), kColorModeCount},
                                                      {QString("Virtual memory heap view"), kColorModePreferredHeap}};

        // Set up the combo box. Get the title string from the first entry in mode_list.
        QString combo_title_string = type_info[0].text;
        RMT_ASSERT(type_list != nullptr);
        if (type_list != nullptr)
        {
            timeline_type_ = type_list[0];
            if (timeline_type_ < type_info.size())
            {
                combo_title_string = type_info[timeline_type_].text;
                color_mode_        = type_info[timeline_type_].color_mode;
            }
        }

        rmv::widget_util::InitSingleSelectComboBox(parent, combo_box, combo_title_string, false);

        // Add the required coloring modes to the combo box and the internal map.
        combo_box->ClearItems();
        if (type_list != nullptr)
        {
            int  list_index = 0;
            bool done       = false;
            do
            {
                int index = type_list[list_index];
                if (index > -1)
                {
                    combo_box->AddItem(type_info[index].text);
                    timeline_type_map_[list_index] = static_cast<RmtDataTimelineType>(index);
                    color_mode_map_[list_index]    = type_info[index].color_mode;
                    list_index++;
                }
                else
                {
                    done = true;
                }
            } while (!done);
        }

        ColorizerBase::Initialize(combo_box, legends_view);
    }

    void TimelineColorizer::UpdateLegends()
    {
        if (color_mode_ == ColorMode::kColorModeResourceUsageType)
        {
            // Handle special case for TimelineColorizer: hide heap resource type on legend.
            legends_scene_->Clear();

            // Note: Usage types on the legend are drawn in reverse order so that highest aliased priority usage are on the left, lowest on the right.
            for (int32_t index = kRmtResourceUsageTypeCount - 1; index > -1; --index)
            {
                if ((index != RmtResourceUsageType::kRmtResourceUsageTypeUnknown) && (index != RmtResourceUsageType::kRmtResourceUsageTypeHeap))
                {
                    const RmtResourceUsageType resource_usage_type = (RmtResourceUsageType)index;
                    legends_scene_->AddColorLegendItem(GetResourceUsageColor(resource_usage_type),
                                                       RmtGetResourceUsageTypeNameFromResourceUsageType(resource_usage_type));
                }
            }
        }
        else
        {
            ColorizerBase::UpdateLegends(legends_scene_, color_mode_);
        }
    }

    RmtDataTimelineType TimelineColorizer::ApplyColorMode(int index)
    {
        timeline_type_ = timeline_type_map_[index];
        color_mode_    = color_mode_map_[index];

        return timeline_type_;
    }
}  // namespace rmv
