//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RMV's carousel memory types widget
//=============================================================================

#include "views/custom_widgets/rmv_carousel_memory_types.h"

#include <QPainter>

#include "rmt_print.h"

static const int kBarWidth   = 20;
static const int kEdgeMargin = 10;

RMVCarouselMemoryTypes::RMVCarouselMemoryTypes(const RMVCarouselConfig& config)
    : RMVCarouselItem(config)
    , data_{}
    , physical_heap_(false)
{
}

RMVCarouselMemoryTypes::~RMVCarouselMemoryTypes()
{
}

void RMVCarouselMemoryTypes::SetIsPhysicalHeap(bool is_physical_heap)
{
    physical_heap_ = is_physical_heap;
}

void RMVCarouselMemoryTypes::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    QString   title_string;
    HeapData* heap;
    if (physical_heap_)
    {
        title_string = "Committed virtual memory";
        heap         = data_.physical_heap;
    }
    else
    {
        title_string = "Requested virtual memory";
        heap         = data_.preferred_heap;
    }

    DrawCarouselBaseComponents(painter, title_string);

    int bar_length = config_.width - (2 * kEdgeMargin);

    DrawColoredHorizontalBarComponent(painter,
                                      RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeLocal),
                                      heap[kRmtHeapTypeLocal].color,
                                      kEdgeMargin,
                                      60,
                                      bar_length,
                                      kBarWidth,
                                      heap[kRmtHeapTypeLocal].value,
                                      heap[kRmtHeapTypeLocal].max,
                                      true);
    DrawColoredHorizontalBarComponent(painter,
                                      RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeInvisible),
                                      heap[kRmtHeapTypeInvisible].color,
                                      kEdgeMargin,
                                      130,
                                      bar_length,
                                      kBarWidth,
                                      heap[kRmtHeapTypeInvisible].value,
                                      heap[kRmtHeapTypeInvisible].max,
                                      true);
    DrawColoredHorizontalBarComponent(painter,
                                      RmtGetHeapTypeNameFromHeapType(kRmtHeapTypeSystem),
                                      heap[kRmtHeapTypeSystem].color,
                                      kEdgeMargin,
                                      200,
                                      bar_length,
                                      kBarWidth,
                                      heap[kRmtHeapTypeSystem].value,
                                      heap[kRmtHeapTypeSystem].max,
                                      true);
}

void RMVCarouselMemoryTypes::SetData(const RMVCarouselData& data)
{
    data_ = data.memory_types_data;
    update();
}