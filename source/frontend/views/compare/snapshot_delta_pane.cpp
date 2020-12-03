//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of Snapshot delta pane.
//=============================================================================

#include "views/compare/snapshot_delta_pane.h"

#include "qt_common/utils/scaling_manager.h"

#include "util/rmv_util.h"
#include "util/widget_util.h"

typedef struct RmtDataSnapshot RmtDataSnapshot;

SnapshotDeltaPane::SnapshotDeltaPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::SnapshotDeltaPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::SnapshotDeltaModel();

    model_->InitializeModel(ui_->base_snapshot_label_, rmv::kHeapDeltaCompareBaseName, "text");
    model_->InitializeModel(ui_->diff_snapshot_label_, rmv::kHeapDeltaCompareDiffName, "text");

    delta_items_.push_back({"Available size", kDeltaValueTypeValueLabeled, false, 0, "", QColor()});
    delta_items_.push_back({"Allocated and bound", kDeltaValueTypeValueLabeled, true, 0, "", QColor()});
    delta_items_.push_back({"Allocated and unbound", kDeltaValueTypeValueLabeled, true, 0, "", QColor()});
    delta_items_.push_back({"Allocations", kDeltaValueTypeValue, true, 0, "", QColor()});
    delta_items_.push_back({"Resources", kDeltaValueTypeValue, true, 0, "", QColor()});

    delta_line_pairs_[0].display = ui_->delta_view_heap_0_;
    delta_line_pairs_[0].line    = nullptr;

    delta_line_pairs_[1].display = ui_->delta_view_heap_1_;
    delta_line_pairs_[1].line    = ui_->delta_view_line_1_;

    delta_line_pairs_[2].display = ui_->delta_view_heap_2_;
    delta_line_pairs_[2].line    = ui_->delta_view_line_2_;

    for (int32_t current_heap_index = 0; current_heap_index <= kRmtHeapTypeSystem; current_heap_index++)
    {
        delta_line_pairs_[current_heap_index].display->Init(model_->GetHeapName(current_heap_index), delta_items_);
    }

    rmv::widget_util::InitGraphicsView(ui_->carousel_view_, ScalingManager::Get().Scaled(kCarouselItemHeight));

    RMVCarouselConfig config = {};
    config.height            = ui_->carousel_view_->height();
    config.data_type         = kCarouselDataTypeDelta;

    carousel_ = new RMVCarousel(config);
    ui_->carousel_view_->setScene(carousel_->Scene());

    ui_->legends_view_->setFrameStyle(QFrame::NoFrame);
    ui_->legends_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui_->legends_view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    rmv::widget_util::InitColorLegend(legends_, ui_->legends_view_);
    AddMemoryDeltaLegends();

    QRectF legend_rect = legends_->itemsBoundingRect();
    ui_->legends_view_->setFixedSize(legend_rect.toRect().size());
    ui_->legends_view_->setSceneRect(legend_rect);

    connect(ui_->switch_button_, &QPushButton::pressed, this, &SnapshotDeltaPane::SwitchSnapshots);

    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &SnapshotDeltaPane::OnScaleFactorChanged);
}

SnapshotDeltaPane::~SnapshotDeltaPane()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &SnapshotDeltaPane::OnScaleFactorChanged);
    delete carousel_;
    delete model_;
}

void SnapshotDeltaPane::showEvent(QShowEvent* event)
{
    Refresh();

    QWidget::showEvent(event);
}

void SnapshotDeltaPane::OnScaleFactorChanged()
{
    // Carousel
    ui_->carousel_view_->setFixedHeight(ScalingManager::Get().Scaled(kCarouselItemHeight));

    // Legend
    QRectF legend_rect = legends_->itemsBoundingRect();
    ui_->legends_view_->setFixedSize(legend_rect.toRect().size());
    ui_->legends_view_->setSceneRect(legend_rect);

    // Delta Displays
    ui_->delta_view_heap_0_->setFixedHeight(ScalingManager::Get().Scaled(kHeapDeltaWidgetHeight));
    ui_->delta_view_heap_1_->setFixedHeight(ScalingManager::Get().Scaled(kHeapDeltaWidgetHeight));
    ui_->delta_view_heap_2_->setFixedHeight(ScalingManager::Get().Scaled(kHeapDeltaWidgetHeight));
}

void SnapshotDeltaPane::SwitchSnapshots()
{
    if (model_->SwapSnapshots() == true)
    {
        UpdateUI();
    }
}

void SnapshotDeltaPane::UpdateUI()
{
    // update delta information
    for (int32_t current_heap_index = 0; current_heap_index <= kRmtHeapTypeSystem; current_heap_index++)
    {
        delta_line_pairs_[current_heap_index].display->Init(model_->GetHeapName(current_heap_index), delta_items_);
    }

    model_->UpdateCarousel(carousel_);

    // Update heap data
    for (int32_t current_heap_index = 0; current_heap_index <= kRmtHeapTypeSystem; current_heap_index++)
    {
        rmv::SnapshotDeltaModel::HeapDeltaData heap_delta_data;

        if (model_->CalcPerHeapDelta((RmtHeapType)current_heap_index, heap_delta_data) == true)
        {
            delta_items_[kSnapshotDeltaTypeAvailableSize].value_num       = heap_delta_data.total_available_size;
            delta_items_[kSnapshotDeltaTypeAllocatedAndBound].value_num   = heap_delta_data.total_allocated_and_bound;
            delta_items_[kSnapshotDeltaTypeAllocatedAndUnbound].value_num = heap_delta_data.total_allocated_and_unbound;
            delta_items_[kSnapshotDeltaTypeAllocationCount].value_num     = heap_delta_data.allocation_count;
            delta_items_[kSnapshotDeltaTypeResourceCount].value_num       = heap_delta_data.resource_count;

            for (int j = 0; j < kSnapshotDeltaTypeCount; j++)
            {
                delta_line_pairs_[current_heap_index].display->UpdateItem(delta_items_[j]);
            }
        }
    }

    ResizeItems();
}

void SnapshotDeltaPane::Refresh()
{
    if (model_->Update() == true)
    {
        UpdateUI();

        for (int32_t current_heap_index = 0; current_heap_index <= kRmtHeapTypeSystem; current_heap_index++)
        {
            if (delta_line_pairs_[current_heap_index].display != nullptr)
            {
                delta_line_pairs_[current_heap_index].display->show();
            }

            if (delta_line_pairs_[current_heap_index].line != nullptr)
            {
                delta_line_pairs_[current_heap_index].line->show();
            }
        }
    }
}

void SnapshotDeltaPane::PaneSwitched()
{
    ResizeItems();
}

void SnapshotDeltaPane::Reset()
{
    model_->ResetModelValues();
}

void SnapshotDeltaPane::ChangeColoring()
{
    legends_->Clear();
    AddMemoryDeltaLegends();
}

void SnapshotDeltaPane::ResizeItems()
{
    if (carousel_ != nullptr)
    {
        carousel_->ResizeEvent(ui_->carousel_view_->width(), ui_->carousel_view_->height());
    }
}

void SnapshotDeltaPane::resizeEvent(QResizeEvent* event)
{
    ResizeItems();
    QWidget::resizeEvent(event);
}

void SnapshotDeltaPane::AddMemoryDeltaLegends()
{
    legends_->AddColorLegendItem(rmv_util::GetDeltaChangeColor(kDeltaChangeIncrease), "Increase");
    legends_->AddColorLegendItem(rmv_util::GetDeltaChangeColor(kDeltaChangeDecrease), "Decrease");
    legends_->AddColorLegendItem(rmv_util::GetDeltaChangeColor(kDeltaChangeNone), "No delta");
}
