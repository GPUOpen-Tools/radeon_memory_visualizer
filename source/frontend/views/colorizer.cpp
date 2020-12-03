//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for a colorizer control.
/// The colorizer is responsible for the functionality for the "color by"
/// combo box across multiple panes. It sets up the combo box with all or
/// a subset of the available coloring modes and updates the allocations and
/// resource widgets and the legends depending on which coloring mode is
/// required.
//=============================================================================

#include "views/colorizer.h"

#include <QString>

#include "rmt_assert.h"

#include "util/widget_util.h"

static const QString kColorByAllocation        = "Color by allocation";
static const QString kColorByPreferredHeap     = "Color by preferred heap";
static const QString kColorByActualHeap        = "Color by actual heap";
static const QString kColorByResourceUsage     = "Color by resource usage";
static const QString kColorByAllocationAge     = "Color by allocation age";
static const QString kColorByResourceCreateAge = "Color by resource create time";
static const QString kColorByResourceBindAge   = "Color by resource bind time";
static const QString kColorByResourceGUID      = "Color by resource id";
static const QString kColorByCPUMapped         = "Color by CPU mapped";
static const QString kColorByNotAllPreferred   = "Color by not all in preferred heap";
static const QString kColorByAliasing          = "Color by aliasing";
static const QString kColorByCommitType        = "Color by commit type";

Colorizer::Colorizer()
    : ColorizerBase()
{
}

Colorizer::~Colorizer()
{
    disconnect(combo_box_, &ArrowIconComboBox::SelectionChanged, this, &Colorizer::ApplyColorMode);
}

void Colorizer::Initialize(QWidget* parent, ArrowIconComboBox* combo_box, ColoredLegendGraphicsView* legends_view, const ColorMode* mode_list)
{
    // Add text strings to the sort combo box
    static const std::map<ColorMode, QString> kColorMap = {{kColorModeResourceUsageType, kColorByResourceUsage},
                                                           {kColorModePreferredHeap, kColorByPreferredHeap},
                                                           {kColorModeActualHeap, kColorByActualHeap},
                                                           {kColorModeAllocationAge, kColorByAllocationAge},
                                                           {kColorModeResourceCreateAge, kColorByResourceCreateAge},
                                                           {kColorModeResourceBindAge, kColorByResourceBindAge},
                                                           {kColorModeResourceGUID, kColorByResourceGUID},
                                                           {kColorModeResourceCPUMapped, kColorByCPUMapped},
                                                           {kColorModeNotAllPreferred, kColorByNotAllPreferred},
                                                           {kColorModeAliasing, kColorByAliasing},
                                                           {kColorModeCommitType, kColorByCommitType}};

    // Set up the combo box. Get the title string from the first entry in mode_list
    auto    it                 = kColorMap.begin();
    QString combo_title_string = (*it).second;
    RMT_ASSERT(mode_list != nullptr);
    if (mode_list != nullptr)
    {
        color_mode_ = mode_list[0];
        if (color_mode_ < kColorModeCount)
        {
            it = kColorMap.find(color_mode_);
            if (it != kColorMap.end())
            {
                combo_title_string = (*it).second;
            }
        }
    }

    rmv::widget_util::InitSingleSelectComboBox(parent, combo_box, combo_title_string, false);

    // Add the required coloring modes to the combo box and the internal map
    combo_box->ClearItems();
    if (mode_list != nullptr)
    {
        int  list_index = 0;
        bool done       = false;
        do
        {
            ColorMode color_mode = mode_list[list_index];
            if (color_mode < kColorModeCount)
            {
                it = kColorMap.find(color_mode);
                if (it != kColorMap.end())
                {
                    combo_box->AddItem((*it).second);
                    color_mode_map_[list_index] = (*it).first;
                    list_index++;
                }
            }
            else
            {
                done = true;
            }
        } while (!done);
    }

    // Set up connections when combo box items are selected
    connect(combo_box, &ArrowIconComboBox::SelectionChanged, this, &Colorizer::ApplyColorMode);

    ColorizerBase::Initialize(combo_box, legends_view);
}

void Colorizer::ApplyColorMode()
{
    int index   = combo_box_->CurrentRow();
    color_mode_ = color_mode_map_[index];
    UpdateLegends();
}
