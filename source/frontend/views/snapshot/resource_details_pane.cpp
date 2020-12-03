//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Resource details pane.
//=============================================================================

#include "views/snapshot/resource_details_pane.h"

#include <QScrollBar>
#include <QThread>

#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_print.h"

#include "models/message_manager.h"
#include "models/proxy_models/resource_details_proxy_model.h"
#include "util/widget_util.h"
#include "views/main_window.h"

static const int kDonutThickness = 20;
static const int kDonutDimension = 200;

/// Worker class definition to do the processing of the resource history data
/// on a separate thread.
class ResourceWorker : public rmv::BackgroundTask
{
public:
    /// Constructor
    ResourceWorker(rmv::ResourceDetailsModel* model, RmtResourceIdentifier resource_identifier)
        : BackgroundTask()
        , model_(model)
        , resource_identifier_(resource_identifier)
    {
    }

    /// Destructor
    ~ResourceWorker()
    {
    }

    /// Worker thread function
    virtual void ThreadFunc()
    {
        model_->GenerateResourceHistory(resource_identifier_);
    }

private:
    rmv::ResourceDetailsModel* model_;                ///< Pointer to the model data
    RmtResourceIdentifier      resource_identifier_;  ///< The selected resource identifier
};

ResourceDetailsPane::ResourceDetailsPane(MainWindow* parent)
    : BasePane(parent)
    , ui_(new Ui::ResourceDetailsPane)
    , main_window_(parent)
    , resource_identifier_(0)
    , thread_controller_(nullptr)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    model_ = new rmv::ResourceDetailsModel();

    model_->InitializeModel(ui_->label_title_, rmv::kResourceDetailsResourceName, "text");

    model_->InitializeModel(ui_->content_base_address_, rmv::kResourceDetailsAllocationBaseAddress, "text");
    model_->InitializeModel(ui_->content_offset_, rmv::kResourceDetailsAllocationOffset, "text");
    model_->InitializeModel(ui_->content_resource_address_, rmv::kResourceDetailsBaseAddress, "text");
    model_->InitializeModel(ui_->content_size_, rmv::kResourceDetailsSize, "text");
    model_->InitializeModel(ui_->content_type_, rmv::kResourceDetailsType, "text");
    model_->InitializeModel(ui_->content_preferred_heap_, rmv::kResourceDetailsHeap, "text");
    model_->InitializeModel(ui_->content_fully_mapped_, rmv::kResourceDetailsFullyMapped, "text");
    model_->InitializeModel(ui_->content_unmapped_percentage_, rmv::kResourceDetailsUnmappedPercentage, "text");
    model_->InitializeModel(ui_->content_create_time_, rmv::kResourceDetailsCreateTime, "text");
    model_->InitializeModel(ui_->content_bind_time_, rmv::kResourceDetailsBindTime, "text");
    model_->InitializeModel(ui_->content_commit_type_, rmv::kResourceDetailsCommitTime, "text");
    model_->InitializeModel(ui_->content_owner_type_, rmv::kResourceDetailsOwnerTime, "text");
    model_->InitializeModel(ui_->content_flags_, rmv::kResourceDetailsFlags, "text");

    model_->InitializePropertiesTableModel(ui_->resource_properties_table_view_, 0, 2);

    // The properties table is a small one, so resize this based on contents of every row (ie: setting precision to -1),
    // and add a 20 pixel padding after cell contents.
    ui_->resource_properties_table_view_->SetColumnPadding(20);
    ui_->resource_properties_table_view_->horizontalHeader()->setResizeContentsPrecision(-1);
    ui_->resource_properties_table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui_->resource_properties_table_view_->horizontalHeader()->setSectionsClickable(true);
    ui_->resource_properties_table_view_->horizontalHeader()->setStretchLastSection(true);
    ui_->resource_properties_table_view_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui_->resource_properties_table_view_->verticalHeader()->setResizeContentsPrecision(1);

    model_->InitializeTimelineTableModel(ui_->resource_timeline_table_view_, 0, kResourceHistoryCount);

    // The Resource timeline table has lots of horizontal space,
    // so these column widths are a bit wider than the actual table contents.
    ui_->resource_timeline_table_view_->SetColumnPadding(0);
    ui_->resource_timeline_table_view_->SetColumnWidthEms(0, 6);
    ui_->resource_timeline_table_view_->SetColumnWidthEms(1, 30);
    ui_->resource_timeline_table_view_->SetColumnWidthEms(2, 8);

    // Stil let the user resize the columns if desired.
    ui_->resource_timeline_table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);

    // set up the residency legends
    rmv::widget_util::InitGraphicsView(ui_->legends_view_local_, rmv::kColoredLegendsHeight);
    rmv::widget_util::InitGraphicsView(ui_->legends_view_invisible_, rmv::kColoredLegendsHeight);
    rmv::widget_util::InitGraphicsView(ui_->legends_view_system_, rmv::kColoredLegendsHeight);
    rmv::widget_util::InitGraphicsView(ui_->legends_view_unmapped_, rmv::kColoredLegendsHeight);

    rmv::widget_util::InitColorLegend(legends_scene_heaps_[kResidencyLocal], ui_->legends_view_local_);
    rmv::widget_util::InitColorLegend(legends_scene_heaps_[kResidencyInvisible], ui_->legends_view_invisible_);
    rmv::widget_util::InitColorLegend(legends_scene_heaps_[kResidencySystem], ui_->legends_view_system_);
    rmv::widget_util::InitColorLegend(legends_scene_heaps_[kResidencyUnmapped], ui_->legends_view_unmapped_);

    ui_->resource_timeline_->Initialize(model_);

    // set up the residency donut widget
    ui_->residency_donut_->setFixedWidth(ScalingManager::Get().Scaled(kDonutDimension));
    ui_->residency_donut_->setFixedHeight(ScalingManager::Get().Scaled(kDonutDimension));
    ui_->residency_donut_->SetArcWidth(ScalingManager::Get().Scaled(kDonutThickness));
    ui_->residency_donut_->SetNumSegments(kRmtHeapTypeSystem + 2);

    ui_->resource_timeline_table_view_->setFrameStyle(QFrame::StyledPanel);
    ui_->resource_timeline_table_view_->horizontalHeader()->setSectionsClickable(true);
    ui_->resource_timeline_table_view_->horizontalHeader()->setResizeContentsPrecision(32);

    // Allow the resource_timeline_table_view_ to resize the rows based on the size of the first column.
    ui_->resource_timeline_table_view_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui_->resource_timeline_table_view_->verticalHeader()->setResizeContentsPrecision(1);

    // hide the details column until it's hooked up
    ui_->resource_timeline_table_view_->hideColumn(kResourceHistoryDetails);

    // hide the 'owner type' and 'flags' in the public build
    ui_->label_owner_type_->hide();
    ui_->content_owner_type_->hide();
    ui_->label_flags_->hide();
    ui_->content_flags_->hide();

    // add a delegate to the resource timeline table to allow custom painting
    legend_delegate_ = new RMVResourceEventDelegate(nullptr, model_);
    ui_->resource_timeline_table_view_->setItemDelegateForColumn(kResourceHistoryLegend, legend_delegate_);

    // intercept the ResourceSelected signal so the chosen resource can be set up. This signal is sent
    // before the pane navigation
    connect(&MessageManager::Get(), &MessageManager::ResourceSelected, this, &ResourceDetailsPane::SelectResource);

    connect(ui_->resource_timeline_, &RMVResourceTimeline::TimelineSelected, this, &ResourceDetailsPane::TimelineSelected);

    // click on the table to update the selected icon on the timeline, then call update via a lambda which will cause a repaint
    connect(ui_->resource_timeline_table_view_, &QTableView::clicked, model_, &rmv::ResourceDetailsModel::TimelineEventSelected);
    connect(ui_->resource_timeline_table_view_, &QTableView::clicked, [=]() { ui_->resource_timeline_->update(); });

    // resource base address should navigate to allocation explorer
    ui_->content_base_address_->setCursor(Qt::PointingHandCursor);
    connect(ui_->content_base_address_, &QPushButton::clicked, [=]() { emit MessageManager::Get().NavigateToPane(rmv::kPaneSnapshotAllocationExplorer); });

    // set up a connection between the timeline being sorted and making sure the selected event is visible
    connect(model_->GetTimelineProxyModel(), &rmv::ResourceDetailsProxyModel::layoutChanged, this, &ResourceDetailsPane::ScrollToSelectedEvent);

    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &ResourceDetailsPane::OnScaleFactorChanged);
}

ResourceDetailsPane::~ResourceDetailsPane()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &ResourceDetailsPane::OnScaleFactorChanged);

    delete legend_delegate_;
    delete model_;
    delete ui_;
}

void ResourceDetailsPane::OnScaleFactorChanged()
{
    // Resize the donut
    ui_->residency_donut_->setFixedSize(ScalingManager::Get().Scaled(kDonutDimension), ScalingManager::Get().Scaled(kDonutDimension));
    ui_->residency_donut_->SetArcWidth(ScalingManager::Get().Scaled(kDonutThickness));
}

void ResourceDetailsPane::showEvent(QShowEvent* event)
{
    if (model_->IsResourceValid(resource_identifier_) == true)
    {
        // enable the active resource history page in the stacked widget
        ui_->resource_valid_switch_->setCurrentIndex(0);

        // start the processing thread and pass in the worker object. The thread controller will take ownership
        // of the worker and delete it once it's complete
        thread_controller_ = new rmv::ThreadController(main_window_, this, new ResourceWorker(model_, resource_identifier_));

        // when the worker thread has finished, a signal will be emitted. Wait for the signal here and update
        // the UI with the newly acquired data from the worker thread
        connect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &ResourceDetailsPane::Refresh);
    }
    else
    {
        // enable the invalid resource history page in the stacked widget
        ui_->resource_valid_switch_->setCurrentIndex(1);
    }

    ResizeItems();
    QWidget::showEvent(event);
}

void ResourceDetailsPane::hideEvent(QHideEvent* event)
{
    if (model_->IsResourceValid(resource_identifier_) == true && thread_controller_ != nullptr)
    {
        disconnect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &ResourceDetailsPane::Refresh);
        thread_controller_->deleteLater();
        thread_controller_ = nullptr;
    }
    QWidget::hideEvent(event);
}

void ResourceDetailsPane::Refresh()
{
    // be sure to make sure the worker thread has populated the resource history table before
    // updating the model.
    if (thread_controller_ != nullptr && thread_controller_->Finished())
    {
        // Prior to doing a table update, disable sorting since Qt is super slow about it
        ui_->resource_timeline_table_view_->setSortingEnabled(false);

        int32_t num_properties = model_->Update(resource_identifier_);
        if (num_properties == 0)
        {
            ui_->resource_properties_valid_switch_->setCurrentIndex(0);
        }
        else
        {
            ui_->resource_properties_valid_switch_->setCurrentIndex(1);
        }

        ui_->resource_timeline_table_view_->setSortingEnabled(true);
        ui_->resource_timeline_table_view_->sortByColumn(kResourceHistoryTime, Qt::AscendingOrder);
        rmv::widget_util::SetWidgetBackgroundColor(ui_->residency_donut_, Qt::white);

        int     value;
        QString name;
        QColor  color;

        RmtResourceBackingStorage heapTypes[kResidencyCount] = {(RmtResourceBackingStorage)kRmtHeapTypeLocal,
                                                                (RmtResourceBackingStorage)kRmtHeapTypeInvisible,
                                                                (RmtResourceBackingStorage)kRmtHeapTypeSystem,
                                                                kRmtResourceBackingStorageUnmapped};

        for (int i = 0; i < kResidencyCount; i++)
        {
            RMT_ASSERT(legends_scene_heaps_[i] != nullptr);
            legends_scene_heaps_[i]->Clear();

            value = 0;
            if (model_->GetResidencyData(resource_identifier_, heapTypes[i], value, name, color) == true)
            {
                legends_scene_heaps_[i]->AddColorLegendItem(color, name + QString(" (%1%)").arg(value));
                ui_->residency_donut_->SetIndexValue(i, value);
                ui_->residency_donut_->SetIndexColor(i, color);
            }
        }

        // set the view sizes to match the scene sizes so the legends appear left-justified
        ui_->legends_view_local_->setFixedSize(legends_scene_heaps_[kResidencyLocal]->itemsBoundingRect().size().toSize());
        ui_->legends_view_invisible_->setFixedSize(legends_scene_heaps_[kResidencyInvisible]->itemsBoundingRect().size().toSize());
        ui_->legends_view_system_->setFixedSize(legends_scene_heaps_[kResidencySystem]->itemsBoundingRect().size().toSize());
        ui_->legends_view_unmapped_->setFixedSize(legends_scene_heaps_[kResidencyUnmapped]->itemsBoundingRect().size().toSize());

        bool baseAddressValid = model_->IsResourceBaseAddressValid(resource_identifier_);
        ui_->content_base_address_->setEnabled(baseAddressValid);
        if (baseAddressValid == true)
        {
            ui_->content_base_address_->setStyleSheet(kLinkButtonStylesheet);
        }
        else
        {
            ui_->content_base_address_->setStyleSheet("QPushButton { color : lightGray; border: none; text-align: left}");
        }

        // Show the warning message if the memory isn't all in the preferred heap
        if (model_->PhysicalMemoryInPreferredHeap(resource_identifier_) == true)
        {
            ui_->warning_widget_->hide();
        }
        else
        {
            ui_->warning_widget_->show();
        }
    }

    // Make sure the properties table can display the contents.
    ui_->resource_properties_table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    SetMaximumTimelineTableHeight();
}

void ResourceDetailsPane::SwitchTimeUnits()
{
    if (resource_identifier_ != 0)
    {
        model_->Update(resource_identifier_);
    }
}

void ResourceDetailsPane::ChangeColoring()
{
    Refresh();
}

void ResourceDetailsPane::ResizeItems()
{
    Refresh();
}

void ResourceDetailsPane::resizeEvent(QResizeEvent* event)
{
    ResizeItems();
    QWidget::resizeEvent(event);
}

void ResourceDetailsPane::SelectResource(RmtResourceIdentifier resource_identifier)
{
    resource_identifier_ = resource_identifier;
}

void ResourceDetailsPane::TimelineSelected(double logical_position, double icon_size)
{
    int row = model_->GetEventRowFromTimeline(logical_position, icon_size);
    if (row != -1)
    {
        ui_->resource_timeline_table_view_->selectRow(row);
    }
    else
    {
        ui_->resource_timeline_table_view_->clearSelection();
    }
    ui_->resource_timeline_->update();
}

void ResourceDetailsPane::ScrollToSelectedEvent()
{
    QItemSelectionModel* selected_item = ui_->resource_timeline_table_view_->selectionModel();
    if (selected_item->hasSelection())
    {
        QModelIndexList item_list = selected_item->selectedRows();
        if (item_list.size() > 0)
        {
            QModelIndex model_index = item_list[0];
            ui_->resource_timeline_table_view_->scrollTo(model_index, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}
