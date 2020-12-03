//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of Block Explorer pane.
//=============================================================================

#include "views/snapshot/allocation_explorer_pane.h"

#include <QDebug>
#include <QScrollBar>
#include <QListWidget>

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"

#include "qt_common/utils/scaling_manager.h"

#include "models/message_manager.h"
#include "models/proxy_models/allocation_proxy_model.h"
#include "models/resource_item_model.h"
#include "models/snapshot/allocation_explorer_model.h"
#include "settings/rmv_settings.h"
#include "views/colorizer.h"
#include "views/pane_manager.h"

// Enum for the number of allocation models needed. For this pane, there is only a single
// allocation graphic.
enum
{
    kAllocationModelIndex,

    kNumAllocationModels,
};

AllocationExplorerPane::AllocationExplorerPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::AllocationExplorerPane)
{
    ui_->setupUi(this);

    // fix up the ratios of the 3 splitter regions
    ui_->splitter_->setStretchFactor(0, 4);
    ui_->splitter_->setStretchFactor(1, 1);
    ui_->splitter_->setStretchFactor(2, 3);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::VirtualAllocationExplorerModel(kNumAllocationModels);

    // initialize allocation table
    model_->InitializeAllocationTableModel(ui_->allocation_table_view_, 0, kVirtualAllocationColumnCount);

    // initialize resource table
    model_->InitializeResourceTableModel(ui_->resource_table_view_, 0, kResourceColumnCount);

    rmv::widget_util::InitCommonFilteringComponents(ui_->resource_search_box_, ui_->resource_size_slider_);
    rmv::widget_util::InitCommonFilteringComponents(ui_->allocation_search_box_, ui_->allocation_size_slider_);
    ui_->aliased_resource_checkbox_->Initialize(false, rmv::kCheckboxEnableColor, Qt::black);

    colorizer_ = new Colorizer();

    allocation_scene_ = new QGraphicsScene;
    allocation_item_  = new RMVAllocationBar(model_->GetAllocationBarModel(), 0, kAllocationModelIndex, colorizer_);
    allocation_scene_->addItem(allocation_item_);
    ui_->memory_block_view_->setScene(allocation_scene_);

    // Set up a list of required coloring modes, in order.
    // The list is terminated with COLOR_MODE_COUNT
    static const Colorizer::ColorMode mode_list[] = {
        Colorizer::kColorModeResourceUsageType,
        Colorizer::kColorModePreferredHeap,
        Colorizer::kColorModeAllocationAge,
        Colorizer::kColorModeResourceCreateAge,
        Colorizer::kColorModeResourceBindAge,
        Colorizer::kColorModeResourceGUID,
        Colorizer::kColorModeResourceCPUMapped,
        Colorizer::kColorModeNotAllPreferred,
        Colorizer::kColorModeAliasing,
        Colorizer::kColorModeCommitType,
        Colorizer::kColorModeCount,
    };

    // Initialize the "color by" UI elements. Set up the combo box, legends and signals etc
    colorizer_->Initialize(parent, ui_->color_combo_box_, ui_->legends_view_, mode_list);

    connect(ui_->resource_size_slider_, &DoubleSliderWidget::SpanChanged, this, &AllocationExplorerPane::ResourceSizeFilterChanged);
    connect(ui_->resource_search_box_, &QLineEdit::textChanged, this, &AllocationExplorerPane::ResourceSearchBoxChanged);
    connect(ui_->allocation_size_slider_, &DoubleSliderWidget::SpanChanged, this, &AllocationExplorerPane::AllocationSizeFilterChanged);
    connect(ui_->allocation_search_box_, &QLineEdit::textChanged, this, &AllocationExplorerPane::AllocationSearchBoxChanged);
    connect(ui_->color_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &AllocationExplorerPane::ColorModeChanged);
    connect(ui_->resource_table_view_, &QTableView::doubleClicked, this, &AllocationExplorerPane::ResourceTableDoubleClicked);
    connect(ui_->aliased_resource_checkbox_, &RMVColoredCheckbox::Clicked, this, &AllocationExplorerPane::AliasedResourceClicked);

    connect(ui_->allocation_table_view_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AllocationExplorerPane::AllocationTableChanged);
    connect(ui_->resource_table_view_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AllocationExplorerPane::ResourceTableSelectionChanged);

    // Resize the memory block if the splitter is moved
    connect(ui_->splitter_, &QSplitter::splitterMoved, this, [=]() { ResizeItems(); });

    // intercept the AllocationSelected signal so the chosen resource can be set up. This signal is sent
    // before the pane navigation
    connect(&MessageManager::Get(), &MessageManager::ResourceSelected, this, &AllocationExplorerPane::SelectResource);
    connect(&MessageManager::Get(), &MessageManager::UnboundResourceSelected, this, &AllocationExplorerPane::SelectUnboundResource);

    // set up a connection between the tables being sorted and making sure the selected event is visible
    connect(model_->GetAllocationProxyModel(), &rmv::ResourceProxyModel::layoutChanged, this, &AllocationExplorerPane::ScrollToSelectedAllocation);
    connect(model_->GetResourceProxyModel(), &rmv::ResourceProxyModel::layoutChanged, this, &AllocationExplorerPane::ScrollToSelectedResource);

    connect(allocation_item_, &RMVAllocationBar::ResourceSelected, this, &AllocationExplorerPane::SelectedResource);

    ui_->resource_table_valid_switch_->setCurrentIndex(0);
}

AllocationExplorerPane::~AllocationExplorerPane()
{
    delete colorizer_;
    delete allocation_scene_;
}

void AllocationExplorerPane::showEvent(QShowEvent* event)
{
    ResizeItems();

    SetMaximumAllocationTableHeight();

    QWidget::showEvent(event);
}

void AllocationExplorerPane::Refresh()
{
    // Prior to doing a table update, disable sorting since Qt is super slow about it
    ui_->resource_table_view_->setSortingEnabled(false);

    model_->BuildResourceSizeThresholds();
    int32_t resource_count = model_->UpdateResourceTable();
    model_->ResourceSizeFilterChanged(ui_->resource_size_slider_->minimum(), ui_->resource_size_slider_->maximum());

    ui_->resource_table_view_->setSortingEnabled(true);
    ui_->resource_table_view_->sortByColumn(kResourceColumnMappedInvisible, Qt::DescendingOrder);
    ui_->resource_table_view_->horizontalHeader()->adjustSize();

    ResizeItems();

    // if there are no resources to show, don't display the resources table
    if (resource_count == 0)
    {
        ui_->resource_table_valid_switch_->setCurrentIndex(0);
    }
    else
    {
        ui_->resource_table_valid_switch_->setCurrentIndex(1);
    }

    SetMaximumResourceTableHeight();
}

void AllocationExplorerPane::Reset()
{
    ui_->color_combo_box_->SetSelectedRow(0);
    colorizer_->ApplyColorMode();
    ui_->aliased_resource_checkbox_->setChecked(false);
    AliasedResourceClicked();

    model_->ResetModelValues();

    ui_->allocation_size_slider_->SetLowerValue(0);
    ui_->allocation_size_slider_->SetUpperValue(rmv::kSizeSliderRange);
    ui_->allocation_search_box_->setText("");

    ui_->resource_size_slider_->SetLowerValue(0);
    ui_->resource_size_slider_->SetUpperValue(rmv::kSizeSliderRange);
    ui_->resource_search_box_->setText("");

    SelectResource(0);
}

void AllocationExplorerPane::ChangeColoring()
{
    colorizer_->UpdateLegends();
    ui_->memory_block_view_->viewport()->update();
}

void AllocationExplorerPane::ColorModeChanged()
{
    ChangeColoring();
}

void AllocationExplorerPane::OpenSnapshot(RmtDataSnapshot* snapshot)
{
    RMT_ASSERT(snapshot != nullptr);
    if (snapshot != nullptr)
    {
        if (model_->OpenSnapshot(snapshot) == false)
        {
            Reset();
            return;
        }

        SelectResource(0);

        // Build the allocation table and set sorting to the allocation size
        ui_->allocation_table_view_->setSortingEnabled(false);
        model_->UpdateAllocationTable();
        ui_->allocation_table_view_->setSortingEnabled(true);
        ui_->allocation_table_view_->sortByColumn(kVirtualAllocationColumnAllocationSize, Qt::DescendingOrder);

        // and select it
        ui_->allocation_table_view_->selectRow(0);

        Refresh();
    }
}

void AllocationExplorerPane::resizeEvent(QResizeEvent* event)
{
    ResizeItems();
    QWidget::resizeEvent(event);
}

void AllocationExplorerPane::ResizeItems()
{
    qreal        width      = ui_->memory_block_view_->width();
    qreal        height     = ui_->memory_block_view_->height();
    const QRectF scene_rect = QRectF(0, 0, width, height);

    allocation_scene_->setSceneRect(scene_rect);
    allocation_item_->UpdateDimensions(width, height);
}

void AllocationExplorerPane::AllocationSearchBoxChanged()
{
    model_->AllocationSearchBoxChanged(ui_->allocation_search_box_->text());
    SetMaximumAllocationTableHeight();
}

void AllocationExplorerPane::AllocationSizeFilterChanged(int min_value, int max_value)
{
    model_->AllocationSizeFilterChanged(min_value, max_value);
    SetMaximumAllocationTableHeight();
}

void AllocationExplorerPane::ResourceSearchBoxChanged()
{
    model_->ResourceSearchBoxChanged(ui_->resource_search_box_->text());
    SetMaximumResourceTableHeight();
}

void AllocationExplorerPane::ResourceSizeFilterChanged(int min_value, int max_value)
{
    model_->ResourceSizeFilterChanged(min_value, max_value);
    SetMaximumResourceTableHeight();
}

void AllocationExplorerPane::AliasedResourceClicked() const
{
    model_->GetAllocationBarModel()->ShowAliased(ui_->aliased_resource_checkbox_->isChecked());
    ui_->memory_block_view_->viewport()->update();
}

void AllocationExplorerPane::SelectedResource(RmtResourceIdentifier resource_identifier, bool navigate_to_pane)
{
    // Broadcast the resource selection to any listening panes.
    emit MessageManager::Get().ResourceSelected(resource_identifier);

    if (navigate_to_pane == true)
    {
        emit MessageManager::Get().NavigateToPane(rmv::kPaneSnapshotResourceDetails);
    }
}

void AllocationExplorerPane::SelectResource(RmtResourceIdentifier resource_identifier)
{
    const RmtVirtualAllocation* allocation = model_->GetAllocationBarModel()->GetAllocationFromResourceID(resource_identifier, kAllocationModelIndex);
    if (allocation != nullptr)
    {
        // Find the allocation in the allocation table and select it if found.
        const QModelIndex& allocation_index =
            model_->GetAllocationProxyModel()->FindModelIndex(reinterpret_cast<uint64_t>(allocation), kVirtualAllocationColumnId);
        if (allocation_index.isValid() == true)
        {
            ui_->allocation_table_view_->selectRow(allocation_index.row());
        }

        bool aliased = model_->GetAllocationBarModel()->SetSelectedResourceForAllocation(allocation, -1, kAllocationModelIndex);
        ui_->aliased_resource_checkbox_->setEnabled(aliased);

        // Select the resource in the resource table.
        const QModelIndex& resource_index = model_->GetResourceProxyModel()->FindModelIndex(resource_identifier, kResourceColumnGlobalId);
        if (resource_index.isValid() == true)
        {
            ui_->resource_table_view_->selectRow(resource_index.row());
        }
    }
}

void AllocationExplorerPane::SelectUnboundResource(const RmtVirtualAllocation* virtual_allocation)
{
    if (virtual_allocation == nullptr)
    {
        return;
    }

    model_->GetAllocationBarModel()->SetSelectedResourceForAllocation(virtual_allocation, -1, kAllocationModelIndex);

    Refresh();

    // find the allocation in the allocation table and select it if found
    QModelIndex allocation_index =
        model_->GetAllocationProxyModel()->FindModelIndex(reinterpret_cast<uint64_t>(virtual_allocation), kVirtualAllocationColumnId);
    if (allocation_index.isValid() == true)
    {
        ui_->allocation_table_view_->selectRow(allocation_index.row());
    }
}

void AllocationExplorerPane::ResourceTableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);

    // Figure out the model index of the currently selected event
    QModelIndexList modelIndexList = selected.indexes();
    if (modelIndexList.size() != 0)
    {
        const QModelIndex index = modelIndexList.at(0);

        if (index.isValid())
        {
            const QModelIndex source_index = model_->GetResourceProxyModel()->mapToSource(index);
            model_->GetAllocationBarModel()->SelectResource(0, 0, source_index.row());
            allocation_item_->update();
            const RmtResourceIdentifier resource_identifier = model_->GetResourceProxyModel()->GetData(index.row(), kResourceColumnGlobalId);
            emit                        MessageManager::Get().ResourceSelected(resource_identifier);
        }
    }
}

void AllocationExplorerPane::AllocationTableChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);

    // Figure out the model index of the currently selected event
    QModelIndexList modelIndexList = selected.indexes();
    if (modelIndexList.size() != 0)
    {
        const QModelIndex index = modelIndexList.at(0);

        if (index.isValid())
        {
            const RmtVirtualAllocation* allocation =
                reinterpret_cast<RmtVirtualAllocation*>(model_->GetAllocationProxyModel()->GetData(index.row(), kVirtualAllocationColumnId));
            bool aliased = model_->GetAllocationBarModel()->SetSelectedResourceForAllocation(allocation, -1, kAllocationModelIndex);
            ui_->aliased_resource_checkbox_->setEnabled(aliased);

            Refresh();
        }
    }
}

void AllocationExplorerPane::ResourceTableDoubleClicked(const QModelIndex& index) const
{
    if (index.isValid() == true)
    {
        const RmtResourceIdentifier resource_identifier = model_->GetResourceProxyModel()->GetData(index.row(), kResourceColumnGlobalId);
        emit                        MessageManager::Get().ResourceSelected(resource_identifier);
        emit                        MessageManager::Get().NavigateToPane(rmv::kPaneSnapshotResourceDetails);
    }
}

void AllocationExplorerPane::ScrollToSelectedAllocation()
{
    QItemSelectionModel* selected_item = ui_->allocation_table_view_->selectionModel();
    if (selected_item->hasSelection())
    {
        QModelIndexList item_list = selected_item->selectedRows();
        if (item_list.size() > 0)
        {
            // get the model index of the name column since column 0 (compare ID) is hidden and scrollTo
            // doesn't appear to scroll on hidden columns
            QModelIndex model_index = model_->GetAllocationProxyModel()->index(item_list[0].row(), kResourceColumnName);
            ui_->allocation_table_view_->scrollTo(model_index, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}

void AllocationExplorerPane::ScrollToSelectedResource()
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
