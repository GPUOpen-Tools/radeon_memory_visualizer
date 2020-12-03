//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of Allocation List pane.
//=============================================================================

#include "views/snapshot/resource_list_pane.h"

#include <QScrollBar>
#include <QCheckBox>

#include "qt_common/custom_widgets/double_slider_widget.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_assert.h"
#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"
#include "rmt_resource_list.h"
#include "rmt_util.h"

#include "models/message_manager.h"
#include "models/proxy_models/resource_proxy_model.h"
#include "models/resource_item_model.h"
#include "models/snapshot/resource_list_model.h"
#include "settings/rmv_settings.h"
#include "views/pane_manager.h"

ResourceListPane::ResourceListPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::ResourceListPane)
    , model_valid_(false)
    , selected_resource_identifier_(0)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::ResourceListModel();

    model_->InitializeModel(ui_->total_resources_label_, rmv::kResourceListTotalResources, "text");
    model_->InitializeModel(ui_->total_size_label_, rmv::kResourceListTotalSize, "text");

    model_->InitializeTableModel(ui_->resource_table_view_, 0, kResourceColumnCount);

    rmv::widget_util::InitMultiSelectComboBox(this, ui_->preferred_heap_combo_box_, rmv::text::kPreferredHeap);
    rmv::widget_util::InitMultiSelectComboBox(this, ui_->resource_usage_combo_box_, rmv::text::kResourceUsage);

    preferred_heap_combo_box_model_ = new rmv::HeapComboBoxModel();
    preferred_heap_combo_box_model_->SetupHeapComboBox(ui_->preferred_heap_combo_box_);
    connect(preferred_heap_combo_box_model_, &rmv::HeapComboBoxModel::FilterChanged, this, &ResourceListPane::HeapChanged);

    resource_usage_combo_box_model_ = new rmv::ResourceUsageComboBoxModel();
    resource_usage_combo_box_model_->SetupResourceComboBox(ui_->resource_usage_combo_box_);
    connect(resource_usage_combo_box_model_, &rmv::ResourceUsageComboBoxModel::FilterChanged, this, &ResourceListPane::ResourceChanged);

    rmv::widget_util::InitGraphicsView(ui_->carousel_view_, ScalingManager::Get().Scaled(kCarouselItemHeight));

    RMVCarouselConfig config = {};
    config.height            = ui_->carousel_view_->height();
    config.data_type         = kCarouselDataTypeRegular;

    carousel_ = new RMVCarousel(config);
    ui_->carousel_view_->setScene(carousel_->Scene());

    rmv::widget_util::InitCommonFilteringComponents(ui_->search_box_, ui_->size_slider_);

    connect(ui_->size_slider_, &DoubleSliderWidget::SpanChanged, this, &ResourceListPane::FilterBySizeSliderChanged);
    connect(ui_->search_box_, &QLineEdit::textChanged, this, &ResourceListPane::SearchBoxChanged);
    connect(ui_->resource_table_view_, &QTableView::clicked, this, &ResourceListPane::TableClicked);
    connect(ui_->resource_table_view_, &QTableView::doubleClicked, this, &ResourceListPane::TableDoubleClicked);

    // set up a connection between the timeline being sorted and making sure the selected event is visible
    connect(model_->GetResourceProxyModel(), &rmv::ResourceProxyModel::layoutChanged, this, &ResourceListPane::ScrollToSelectedResource);

    connect(&MessageManager::Get(), &MessageManager::ResourceSelected, this, &ResourceListPane::SelectResource);
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &ResourceListPane::OnScaleFactorChanged);
}

ResourceListPane::~ResourceListPane()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &ResourceListPane::OnScaleFactorChanged);

    delete carousel_;
    delete resource_usage_combo_box_model_;
    delete preferred_heap_combo_box_model_;
    delete model_;
}

void ResourceListPane::OnScaleFactorChanged()
{
    // Carousel
    ui_->carousel_view_->setFixedHeight(ScalingManager::Get().Scaled(kCarouselItemHeight));
}

void ResourceListPane::showEvent(QShowEvent* event)
{
    if (model_valid_ == false)
    {
        Refresh();
    }
    QWidget::showEvent(event);
}

void ResourceListPane::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
}

void ResourceListPane::Refresh()
{
    PopulateResourceTable();
    UpdateCarousel();

    ResizeItems();

    QString heap_filter_string = preferred_heap_combo_box_model_->GetFilterString(ui_->preferred_heap_combo_box_);
    model_->UpdatePreferredHeapList(heap_filter_string);
    QString resource_filter_string = resource_usage_combo_box_model_->GetFilterString(ui_->resource_usage_combo_box_);
    model_->UpdateResourceUsageList(resource_filter_string);
}

void ResourceListPane::OnTraceClose()
{
    preferred_heap_combo_box_model_->ResetHeapComboBox(ui_->preferred_heap_combo_box_);
    resource_usage_combo_box_model_->ResetResourceComboBox(ui_->resource_usage_combo_box_);
}

void ResourceListPane::Reset()
{
    model_->ResetModelValues();
    model_valid_                  = false;
    selected_resource_identifier_ = 0;

    ui_->size_slider_->SetLowerValue(0);
    ui_->size_slider_->SetUpperValue(rmv::kSizeSliderRange);
    ui_->search_box_->setText("");

    carousel_->ClearData();
    carousel_->update();
}

void ResourceListPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    Q_UNUSED(snapshot);

    if (this->isVisible())
    {
        // This pane is visible so showEvent won't get called to update the resource table
        // so update it now.
        Refresh();
    }
    else
    {
        // Indicate the model data is not valid so the table will be updated when showEvent
        // is called
        model_valid_ = false;
    }
}

void ResourceListPane::ResizeItems()
{
    if (carousel_ != nullptr)
    {
        carousel_->ResizeEvent(ui_->carousel_view_->width(), ui_->carousel_view_->height());
    }

    SetMaximumResourceTableHeight();
}

void ResourceListPane::resizeEvent(QResizeEvent* event)
{
    ResizeItems();
    QWidget::resizeEvent(event);
}

void ResourceListPane::PopulateResourceTable()
{
    // create the Resource table. Only do this once when showing the pane for the first
    // time for the current snapshot
    // Prior to doing a table update, disable sorting since Qt is super slow about it
    ui_->resource_table_view_->setSortingEnabled(false);
    model_->Update();
    ui_->resource_table_view_->setSortingEnabled(true);
    ui_->resource_table_view_->sortByColumn(kResourceColumnName, Qt::DescendingOrder);
    ui_->resource_table_view_->horizontalHeader()->adjustSize();
    model_valid_ = true;
    SelectResourceInTable();
    ScrollToSelectedResource();
}

void ResourceListPane::UpdateCarousel()
{
    carousel_->UpdateModel();
}

void ResourceListPane::SearchBoxChanged()
{
    model_->SearchBoxChanged(ui_->search_box_->text());
    SetMaximumResourceTableHeight();
}

void ResourceListPane::FilterBySizeSliderChanged(int min_value, int max_value)
{
    model_->FilterBySizeChanged(min_value, max_value);
    SetMaximumResourceTableHeight();
}

void ResourceListPane::HeapChanged(bool checked)
{
    // rebuild the table depending on what the state of the combo box items is
    RMT_UNUSED(checked);

    QString filter_string = preferred_heap_combo_box_model_->GetFilterString(ui_->preferred_heap_combo_box_);
    model_->UpdatePreferredHeapList(filter_string);
    SetMaximumResourceTableHeight();
}

void ResourceListPane::ResourceChanged(bool checked)
{
    // rebuild the table depending on what the state of the combo box items is
    RMT_UNUSED(checked);

    QString filter_string = resource_usage_combo_box_model_->GetFilterString(ui_->resource_usage_combo_box_);
    model_->UpdateResourceUsageList(filter_string);
    SetMaximumResourceTableHeight();
}

void ResourceListPane::SelectResource(RmtResourceIdentifier resource_identifier)
{
    selected_resource_identifier_ = resource_identifier;
    SelectResourceInTable();
}

void ResourceListPane::TableClicked(const QModelIndex& index)
{
    if (index.isValid() == true)
    {
        const RmtResourceIdentifier resource_identifier = model_->GetResourceProxyModel()->GetData(index.row(), kResourceColumnGlobalId);
        emit                        MessageManager::Get().ResourceSelected(resource_identifier);
    }
}

void ResourceListPane::TableDoubleClicked(const QModelIndex& index)
{
    if (index.isValid() == true)
    {
        const RmtResourceIdentifier resource_identifier = model_->GetResourceProxyModel()->GetData(index.row(), kResourceColumnGlobalId);
        emit                        MessageManager::Get().ResourceSelected(resource_identifier);
        emit                        MessageManager::Get().NavigateToPane(rmv::kPaneSnapshotResourceDetails);
    }
}

void ResourceListPane::ScrollToSelectedResource()
{
    QItemSelectionModel* selected_item = ui_->resource_table_view_->selectionModel();
    if (selected_item->hasSelection())
    {
        QModelIndexList item_list = selected_item->selectedRows();
        if (item_list.size() > 0)
        {
            // get the model index of the name column since column 0 (compare ID) is hidden and scrollTo
            // doesn't appear to scroll on hidden columns
            QModelIndex model_index = model_->GetResourceProxyModel()->index(item_list[0].row(), kResourceColumnName);
            ui_->resource_table_view_->scrollTo(model_index, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}

void ResourceListPane::SelectResourceInTable()
{
    if (selected_resource_identifier_ != 0)
    {
        // select the resource in the resource table
        const QModelIndex& resource_index = model_->GetResourceProxyModel()->FindModelIndex(selected_resource_identifier_, kResourceColumnGlobalId);
        if (resource_index.isValid() == true)
        {
            ui_->resource_table_view_->selectRow(resource_index.row());
        }
    }
}