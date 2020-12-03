//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Heap Overview pane.
//=============================================================================

#include "views/snapshot/heap_overview_pane.h"

#include "qt_common/utils/scaling_manager.h"

#include "rmt_types.h"

#include "models/snapshot/heap_overview_heap_model.h"
#include "util/widget_util.h"

HeapOverviewPane::HeapOverviewPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::HeapOverviewPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    ui_->local_heap_view_->Initialize(kRmtHeapTypeLocal);
    ui_->invisible_heap_view_->Initialize(kRmtHeapTypeInvisible);
    ui_->system_heap_view_->Initialize(kRmtHeapTypeSystem);
}

HeapOverviewPane::~HeapOverviewPane()
{
}

void HeapOverviewPane::Refresh()
{
    ui_->local_heap_view_->Update();
    ui_->invisible_heap_view_->Update();
    ui_->system_heap_view_->Update();
}

void HeapOverviewPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    Q_UNUSED(snapshot);

    Refresh();
}

void HeapOverviewPane::ResizeItems()
{
    // Ensure the donut sections have matching widths.
    int row1_donut_width = ui_->local_heap_view_->DonutSectionWidth();
    int row2_donut_width = ui_->invisible_heap_view_->DonutSectionWidth();
    int row3_donut_width = ui_->system_heap_view_->DonutSectionWidth();

    int widest_donut_width = std::max<int>(row1_donut_width, row2_donut_width);
    widest_donut_width     = std::max<int>(widest_donut_width, row3_donut_width);

    ui_->local_heap_view_->SetDonutSectionWidth(widest_donut_width);
    ui_->invisible_heap_view_->SetDonutSectionWidth(widest_donut_width);
    ui_->system_heap_view_->SetDonutSectionWidth(widest_donut_width);
}

void HeapOverviewPane::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    ResizeItems();
}

void HeapOverviewPane::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    ResizeItems();
}