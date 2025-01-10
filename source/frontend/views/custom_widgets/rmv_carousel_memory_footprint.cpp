//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the carousel memory footprint widget.
//=============================================================================

#include "views/custom_widgets/rmv_carousel_memory_footprint.h"

#include <QPainter>

static const int kBarWidth   = 20;
static const int kEdgeMargin = 10;

RMVCarouselMemoryFootprint::RMVCarouselMemoryFootprint(const RMVCarouselConfig& config)
    : RMVCarouselItem(config)
    , data_{}
{
}

RMVCarouselMemoryFootprint::~RMVCarouselMemoryFootprint()
{
}

void RMVCarouselMemoryFootprint::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    DrawCarouselBaseComponents(painter, "Virtual memory");

    int bar_length = config_.width - (2 * kEdgeMargin);
    DrawHorizontalBarComponent(painter, "Bound virtual memory", kEdgeMargin, 80, bar_length, kBarWidth, data_.total_allocated_memory, data_.max_memory, true);
    DrawHorizontalBarComponent(painter, "Unbound virtual memory", kEdgeMargin, 170, bar_length, kBarWidth, data_.total_unused_memory, data_.max_memory, true);
}

void RMVCarouselMemoryFootprint::SetData(const rmv::RMVCarouselData& data)
{
    data_ = data.memory_footprint_data;
    update();
}
