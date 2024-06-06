//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the Memory leak finder pane.
//=============================================================================

#include "views/compare/memory_leak_finder_pane.h"

#include <QScrollBar>

#include "qt_common/utils/qt_util.h"

#include "managers/message_manager.h"
#include "managers/trace_manager.h"
#include "models/compare/memory_leak_finder_model.h"
#include "models/proxy_models/memory_leak_finder_proxy_model.h"
#include "models/resource_item_model.h"
#include "settings/rmv_settings.h"
#include "views/custom_widgets/rmv_colored_checkbox.h"
#include "util/rmv_util.h"

MemoryLeakFinderPane::MemoryLeakFinderPane(QWidget* parent)
    : ComparePane(parent)
    , ui_(new Ui::MemoryLeakFinderPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::MemoryLeakFinderModel();

    model_->InitializeModel(ui_->base_allocations_label_, rmv::kMemoryLeakFinderBaseStats, "text");
    model_->InitializeModel(ui_->both_allocations_label_, rmv::kMemoryLeakFinderBothStats, "text");
    model_->InitializeModel(ui_->diff_allocations_label_, rmv::kMemoryLeakFinderDiffStats, "text");
    model_->InitializeModel(ui_->total_resources_label_, rmv::kMemoryLeakFinderTotalResources, "text");
    model_->InitializeModel(ui_->total_size_label_, rmv::kMemoryLeakFinderTotalSize, "text");
    model_->InitializeModel(ui_->base_allocations_checkbox_, rmv::kMemoryLeakFinderBaseCheckbox, "text");
    model_->InitializeModel(ui_->diff_allocations_checkbox_, rmv::kMemoryLeakFinderDiffCheckbox, "text");
    model_->InitializeModel(ui_->base_snapshot_label_, rmv::kMemoryLeakFinderBaseSnapshot, "text");
    model_->InitializeModel(ui_->diff_snapshot_label_, rmv::kMemoryLeakFinderDiffSnapshot, "text");

    model_->InitializeTableModel(ui_->resource_table_view_, 0, rmv::kResourceColumnCount, rmv::kSnapshotCompareIdCommon);
    ui_->resource_table_view_->setCursor(Qt::PointingHandCursor);
    ui_->resource_table_view_->sortByColumn(rmv::kResourceColumnVirtualAddress, Qt::AscendingOrder);

    rmv::widget_util::InitMultiSelectComboBox(this, ui_->preferred_heap_combo_box_, rmv::text::kPreferredHeap);
    rmv::widget_util::InitMultiSelectComboBox(this, ui_->resource_usage_combo_box_, rmv::text::kResourceUsage);

    preferred_heap_combo_box_model_ = new rmv::HeapComboBoxModel();
    preferred_heap_combo_box_model_->SetupHeapComboBox(ui_->preferred_heap_combo_box_);
    connect(preferred_heap_combo_box_model_, &rmv::HeapComboBoxModel::FilterChanged, this, &MemoryLeakFinderPane::HeapChanged);

    resource_usage_combo_box_model_ = new rmv::ResourceUsageComboBoxModel();
    resource_usage_combo_box_model_->SetupResourceComboBox(ui_->resource_usage_combo_box_, true);
    connect(resource_usage_combo_box_model_, &rmv::ResourceUsageComboBoxModel::FilterChanged, this, &MemoryLeakFinderPane::ResourceChanged);

    compare_id_delegate_ = new RMVCompareIdDelegate();
    ui_->resource_table_view_->setItemDelegateForColumn(rmv::kResourceColumnCompareId, compare_id_delegate_);

    // Set the row height according to the compare ID column delegate.
    ui_->resource_table_view_->verticalHeader()->setDefaultSectionSize(compare_id_delegate_->DefaultSizeHint().height());

    rmv::widget_util::InitCommonFilteringComponents(ui_->search_box_, ui_->size_slider_);
    rmv::widget_util::InitRangeSlider(ui_->size_slider_);

    ui_->base_allocations_checkbox_->Initialize(false, rmv::RMVSettings::Get().GetColorSnapshotViewed(), Qt::black);
    ui_->both_allocations_checkbox_->Initialize(
        true, rmv::RMVSettings::Get().GetColorSnapshotViewed(), rmv::RMVSettings::Get().GetColorSnapshotCompared(), true);
    ui_->diff_allocations_checkbox_->Initialize(false, rmv::RMVSettings::Get().GetColorSnapshotCompared(), Qt::black);

    CompareFilterChanged();

    connect(ui_->size_slider_, &DoubleSliderWidget::SpanChanged, this, &MemoryLeakFinderPane::FilterBySizeSliderChanged);
    connect(ui_->search_box_, &QLineEdit::textChanged, this, &MemoryLeakFinderPane::SearchBoxChanged);
    connect(ui_->resource_table_view_, &QTableView::doubleClicked, this, &MemoryLeakFinderPane::TableDoubleClicked);
    connect(ui_->both_allocations_checkbox_, &RMVColoredCheckbox::Clicked, this, &MemoryLeakFinderPane::CompareFilterChanged);
    connect(ui_->base_allocations_checkbox_, &RMVColoredCheckbox::Clicked, this, &MemoryLeakFinderPane::CompareFilterChanged);
    connect(ui_->diff_allocations_checkbox_, &RMVColoredCheckbox::Clicked, this, &MemoryLeakFinderPane::CompareFilterChanged);
    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::HashesChanged, this, &MemoryLeakFinderPane::UpdateHashes);

    // Set up a connection between the timeline being sorted and making sure the selected event is visible.
    connect(model_->GetResourceProxyModel(), &rmv::MemoryLeakFinderProxyModel::layoutChanged, this, &MemoryLeakFinderPane::ScrollToSelectedResource);

    connect(&rmv::MessageManager::Get(), &rmv::MessageManager::SwapSnapshotsRequested, this, &MemoryLeakFinderPane::SwitchSnapshots);
}

MemoryLeakFinderPane::~MemoryLeakFinderPane()
{
    delete compare_id_delegate_;
    delete resource_usage_combo_box_model_;
    delete preferred_heap_combo_box_model_;
    delete model_;
}

void MemoryLeakFinderPane::showEvent(QShowEvent* event)
{
    HeapChanged(false);
    ResourceChanged(false, -1);
    Update(false);

    QWidget::showEvent(event);
}

void MemoryLeakFinderPane::SwitchSnapshots()
{
    const rmv::SnapshotCompareId compare_filter = GetCompareIdFilter();
    if (model_->SwapSnapshots(compare_filter) == true)
    {
        Update(false);
    }
}

void MemoryLeakFinderPane::Refresh()
{
    Update(true);
}

rmv::SnapshotCompareId MemoryLeakFinderPane::GetCompareIdFilter() const
{
    uint32_t filter = 0;

    if (ui_->base_allocations_checkbox_->isChecked())
    {
        filter |= rmv::kSnapshotCompareIdOpen;
    }

    if (ui_->both_allocations_checkbox_->isChecked())
    {
        filter |= rmv::kSnapshotCompareIdCommon;
    }

    if (ui_->diff_allocations_checkbox_->isChecked())
    {
        filter |= rmv::kSnapshotCompareIdCompared;
    }

    return (rmv::SnapshotCompareId)filter;
}

void MemoryLeakFinderPane::Update(bool reset_filters)
{
    if (reset_filters == true)
    {
        ui_->base_allocations_checkbox_->setChecked(false);
        ui_->both_allocations_checkbox_->setChecked(true);
        ui_->diff_allocations_checkbox_->setChecked(false);
    }

    // Prior to doing a table update, disable sorting since Qt is super slow about it.
    ui_->resource_table_view_->setSortingEnabled(false);

    const rmv::SnapshotCompareId compare_filter = GetCompareIdFilter();
    model_->Update(compare_filter);

    ui_->resource_table_view_->setSortingEnabled(true);

    SetMaximumResourceTableHeight();
}

void MemoryLeakFinderPane::OnTraceClose()
{
    preferred_heap_combo_box_model_->ResetHeapComboBox(ui_->preferred_heap_combo_box_);
    resource_usage_combo_box_model_->ResetResourceComboBox(ui_->resource_usage_combo_box_);
}

void MemoryLeakFinderPane::Reset()
{
    model_->ResetModelValues();

    ui_->size_slider_->SetLowerValue(0);
    ui_->size_slider_->SetUpperValue(ui_->size_slider_->maximum());
    ui_->search_box_->setText("");
}

void MemoryLeakFinderPane::ChangeColoring()
{
    ui_->base_allocations_checkbox_->UpdatePrimaryColor(rmv_util::GetSnapshotStateColor(kSnapshotStateViewed));
    ui_->diff_allocations_checkbox_->UpdatePrimaryColor(rmv_util::GetSnapshotStateColor(kSnapshotStateCompared));
}

void MemoryLeakFinderPane::SearchBoxChanged()
{
    model_->SearchBoxChanged(ui_->search_box_->text());
    SetMaximumResourceTableHeight();
}

void MemoryLeakFinderPane::FilterBySizeSliderChanged(int min_value, int max_value)
{
    model_->FilterBySizeChanged(min_value, max_value);
    SetMaximumResourceTableHeight();
}

void MemoryLeakFinderPane::CompareFilterChanged()
{
    rmv::SnapshotCompareId filter = GetCompareIdFilter();
    model_->Update(filter);
    SetMaximumResourceTableHeight();
}

void MemoryLeakFinderPane::HeapChanged(bool checked)
{
    // Rebuild the table depending on what the state of the combo box items is.
    RMT_UNUSED(checked);

    QString filter_string = preferred_heap_combo_box_model_->GetFilterString(ui_->preferred_heap_combo_box_);
    model_->UpdatePreferredHeapList(filter_string);
    SetMaximumResourceTableHeight();
}

void MemoryLeakFinderPane::ResourceChanged(bool checked, int changed_item_index)
{
    RMT_UNUSED(checked);

    // Rebuild the table depending on what the state of the combo box items is.
    resource_usage_combo_box_model_->UpdateCheckboxes(changed_item_index, ui_->resource_usage_combo_box_);
    QString filter_string = resource_usage_combo_box_model_->GetFilterString(ui_->resource_usage_combo_box_);
    model_->UpdateResourceUsageList(filter_string);
    SetMaximumResourceTableHeight();
}

void MemoryLeakFinderPane::UpdateHashes()
{
    if (rmv::SnapshotManager::Get().GetCompareSnapshot(rmv::kSnapshotCompareDiff) != nullptr)
    {
        Update(false);
    }
}

void MemoryLeakFinderPane::TableDoubleClicked(const QModelIndex& index)
{
    if (index.isValid() == true)
    {
        const RmtResourceIdentifier resource_identifier = model_->GetResourceProxyModel()->GetData(index.row(), rmv::kResourceColumnGlobalId);
        RmtSnapshotPoint*           snapshot_point      = model_->FindSnapshot(index);
        if (snapshot_point)
        {
            emit rmv::SnapshotManager::Get().SnapshotOpened(resource_identifier);
        }
    }
}

void MemoryLeakFinderPane::ScrollToSelectedResource()
{
    QItemSelectionModel* selected_item = ui_->resource_table_view_->selectionModel();
    if (selected_item->hasSelection())
    {
        QModelIndexList item_list = selected_item->selectedRows();
        if (item_list.size() > 0)
        {
            QModelIndex model_index = item_list[0];
            ui_->resource_table_view_->scrollTo(model_index, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}
