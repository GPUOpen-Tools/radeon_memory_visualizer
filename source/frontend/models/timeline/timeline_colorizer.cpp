//=============================================================================
// Copyright (c) 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
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

        static std::vector<TimelineInfo> type_info = {{QString("Process"), kColorModeCount},
                                                      {QString("Page size"), kColorModeCount},
                                                      {QString("Committed"), kColorModePreferredHeap},
                                                      {QString("Resource usage count"), kColorModeResourceUsageType},
                                                      {QString("Resource usage size"), kColorModeResourceUsageType},
                                                      {QString("Paging"), kColorModeCount},
                                                      {QString("Virtual memory heap"), kColorModePreferredHeap}};

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

    RmtDataTimelineType TimelineColorizer::ApplyColorMode(int index)
    {
        timeline_type_ = timeline_type_map_[index];
        color_mode_    = color_mode_map_[index];

        return timeline_type_;
    }
}  // namespace rmv