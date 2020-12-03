//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of Timeline pane.
//=============================================================================

#include "views/timeline/timeline_pane.h"

#include <QScrollBar>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

#include "qt_common/custom_widgets/double_slider_widget.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_assert.h"
#include "rmt_data_snapshot.h"
#include "rmt_util.h"

#include "models/message_manager.h"
#include "models/proxy_models/snapshot_timeline_proxy_model.h"
#include "models/snapshot_manager.h"
#include "models/timeline/snapshot_item_model.h"
#include "models/timeline/timeline_model.h"
#include "models/trace_manager.h"
#include "settings/rmv_settings.h"
#include "util/time_util.h"
#include "views/main_window.h"
#include "views/pane_manager.h"

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

const static QString kRenameAction  = "Rename snapshot";
const static QString kDeleteAction  = "Delete snapshot";
const static QString kCompareAction = "Compare snapshots";

/// Worker class definition to do the processing of the timeline generation
/// on a separate thread.
class TimelineWorker : public rmv::BackgroundTask
{
public:
    /// Constructor.
    explicit TimelineWorker(rmv::TimelineModel* model, RmtDataTimelineType timeline_type)
        : BackgroundTask()
        , model_(model)
        , timeline_type_(timeline_type)
    {
    }

    /// Destructor.
    ~TimelineWorker()
    {
    }

    /// Worker thread function.
    virtual void ThreadFunc()
    {
        model_->GenerateTimeline(timeline_type_);
    }

private:
    rmv::TimelineModel* model_;          ///< Pointer to the model data
    RmtDataTimelineType timeline_type_;  ///< The timeline type
};

TimelinePane::TimelinePane(MainWindow* parent)
    : BasePane(parent)
    , ui_(new Ui::TimelinePane)
    , main_window_(parent)
    , thread_controller_(nullptr)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    // fix up the ratios of the 2 splitter regions
    ui_->splitter_->setStretchFactor(0, 5);
    ui_->splitter_->setStretchFactor(1, 4);

    // initialize the snapshot legends
    rmv::widget_util::InitGraphicsView(ui_->snapshot_legends_view_, rmv::kColoredLegendsHeight);
    rmv::widget_util::InitColorLegend(snapshot_legends_, ui_->snapshot_legends_view_);
    AddSnapshotLegends();

    model_ = new rmv::TimelineModel();

    model_->InitializeModel(ui_->snapshot_count_label_, rmv::kTimelineSnapshotCount, "text");
    model_->InitializeTableModel(ui_->snapshot_table_view_, 0, kSnapshotTimelineColumnCount);

    // Set default columns widths appropriately so that they can show the table contents.
    ui_->snapshot_table_view_->SetColumnPadding(0);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnID, 10);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnName, 11);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnTime, 10);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnVirtualAllocations, 12);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnResources, 9);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnAllocatedTotalVirtualMemory, 14);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnAllocatedBoundVirtualMemory, 14);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnAllocatedUnboundVirtualMemory, 16);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnCommittedLocal, 16);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnCommittedInvisible, 18);
    ui_->snapshot_table_view_->SetColumnWidthEms(kSnapshotTimelineColumnCommittedHost, 16);

    // Allow users to resize columns if desired.
    ui_->snapshot_table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    rmv::widget_util::UpdateTablePalette(ui_->snapshot_table_view_);

    ui_->snapshot_table_view_->horizontalHeader()->setSectionsClickable(true);
    ui_->snapshot_table_view_->setSortingEnabled(true);
    ui_->snapshot_table_view_->sortByColumn(kSnapshotTimelineColumnTime, Qt::AscendingOrder);
    ui_->snapshot_table_view_->setEditTriggers(QAbstractItemView::EditKeyPressed);

    // hide columns that we are using for sorting
    ui_->snapshot_table_view_->hideColumn(kSnapshotTimelineColumnID);

    // hide the snapshot legends for now. Currently not used but maybe needed in future
    ui_->snapshot_legends_controls_wrapper_->hide();

    // initialize the timeline type combo box
    colorizer_ = new TimelineColorizer();

    // Set up a list of required timeline modes, in order.
    // The list is terminated with -1
    static const RmtDataTimelineType type_list[] = {kRmtDataTimelineTypeResourceUsageVirtualSize,
                                                    kRmtDataTimelineTypeResourceUsageCount,
                                                    kRmtDataTimelineTypeVirtualMemory,
                                                    kRmtDataTimelineTypeCommitted,
                                                    // kRmtDataTimelineTypeProcess,
                                                    RmtDataTimelineType(-1)};

    // Initialize the "color by" UI elements. Set up the combo box, legends and signals etc
    colorizer_->Initialize(parent, ui_->timeline_type_combo_box_, ui_->timeline_legends_view_, type_list);
    connect(ui_->timeline_type_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &TimelinePane::TimelineTypeChanged);

    model_->SetTimelineType(type_list[0]);

    // allow multiple snapshots to be selected so they can be compared.
    ui_->snapshot_table_view_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Set up the zoom buttons
    ZoomIconManagerConfiguration zoom_config        = {};
    zoom_config.zoom_in_button                      = ui_->zoom_in_button_;
    zoom_config.zoom_in_resource_enabled            = rmv::resource::kZoomInEnabled;
    zoom_config.zoom_in_resource_disabled           = rmv::resource::kZoomInDisabled;
    zoom_config.zoom_out_button                     = ui_->zoom_out_button_;
    zoom_config.zoom_out_resource_enabled           = rmv::resource::kZoomOutEnabled;
    zoom_config.zoom_out_resource_disabled          = rmv::resource::kZoomOutDisabled;
    zoom_config.zoom_reset_button                   = ui_->zoom_reset_button_;
    zoom_config.zoom_reset_resource_enabled         = rmv::resource::kZoomResetEnabled;
    zoom_config.zoom_reset_resource_disabled        = rmv::resource::kZoomResetDisabled;
    zoom_config.zoom_to_selection_button            = ui_->zoom_to_selection_button_;
    zoom_config.zoom_to_selection_resource_enabled  = rmv::resource::kZoomToSelectionEnabled;
    zoom_config.zoom_to_selection_resource_disabled = rmv::resource::kZoomToSelectionDisabled;

    zoom_icon_manager_ = new ZoomIconGroupManager(zoom_config);

    rmv::widget_util::InitCommonFilteringComponents(ui_->search_box_, ui_->size_slider_);

    // hide size slider for now
    ui_->size_slider_->hide();
    ui_->size_slider_label_->hide();

    // disable the compare button
    ui_->compare_button_->setEnabled(false);

    keyboard_zoom_shortcuts_ = new KeyboardZoomShortcutsTimeline(this, ui_->timeline_view_->horizontalScrollBar(), ui_->timeline_view_);

    connect(keyboard_zoom_shortcuts_, &KeyboardZoomShortcutsTimeline::ZoomInSelectionSignal, this, &TimelinePane::ZoomInSelection);
    connect(keyboard_zoom_shortcuts_, &KeyboardZoomShortcutsTimeline::ResetViewSignal, this, &TimelinePane::ZoomReset);
    connect(ui_->size_slider_, &DoubleSliderWidget::SpanChanged, this, &TimelinePane::FilterBySizeSliderChanged);
    connect(ui_->search_box_, &QLineEdit::textChanged, this, &TimelinePane::SearchBoxChanged);
    connect(ui_->zoom_to_selection_button_, &QPushButton::pressed, this, &TimelinePane::ZoomInSelection);
    connect(ui_->zoom_reset_button_, &QPushButton::pressed, this, &TimelinePane::ZoomReset);
    connect(ui_->zoom_in_button_, &QPushButton::pressed, this, &TimelinePane::ZoomIn);
    connect(ui_->zoom_out_button_, &QPushButton::pressed, this, &TimelinePane::ZoomOut);
    connect(ui_->timeline_view_, &RMVSnapshotTimeline::UpdateSelectedDuration, this, &TimelinePane::UpdateSelectedDuration);
    connect(ui_->timeline_view_, &RMVSnapshotTimeline::UpdateHoverClock, this, &TimelinePane::UpdateHoverClock);
    connect(ui_->snapshot_table_view_, &RMVSnapshotTableView::SelectionChanged, this, &TimelinePane::TableSelectionChanged);
    connect(ui_->snapshot_table_view_, &RMVSnapshotTableView::doubleClicked, this, &TimelinePane::TableDoubleClicked);
    connect(ui_->timeline_view_, &RMVSnapshotTimeline::GenerateSnapshotAtTime, this, &TimelinePane::GenerateSnapshotAtTime);
    connect(ui_->timeline_view_, &RMVSnapshotTimeline::UpdateZoomButtonsForZoomIn, this, &TimelinePane::UpdateZoomButtonsForZoomIn);
    connect(ui_->timeline_view_, &RMVSnapshotTimeline::UpdateZoomButtonsForZoomOut, this, &TimelinePane::UpdateZoomButtonsForZoomOut);
    connect(ui_->timeline_view_, &RMVSnapshotTimeline::UpdateZoomButtonsForZoomToSelection, this, &TimelinePane::UpdateZoomButtonsForZoomToSelection);
    connect(ui_->timeline_view_->horizontalScrollBar(), &QScrollBar::valueChanged, this, &TimelinePane::ScrollBarChanged);
    connect(ui_->compare_button_, &QPushButton::pressed, this, &TimelinePane::CompareSnapshots);
    connect(&MessageManager::Get(), &MessageManager::SelectSnapshot, this, &TimelinePane::SelectSnapshot);

    // set up a connection between the timeline being sorted and making sure the selected event is visible
    connect(model_->GetProxyModel(), &rmv::SnapshotTimelineProxyModel::layoutChanged, this, &TimelinePane::ScrollToSelectedSnapshot);
}

TimelinePane::~TimelinePane()
{
    delete zoom_icon_manager_;
    delete colorizer_;
}

void TimelinePane::showEvent(QShowEvent* event)
{
    SwitchTimeUnits();
    ui_->compare_button_->setEnabled(false);
    SelectTableRows();
    QWidget::showEvent(event);
}

void TimelinePane::SelectTableRows()
{
    ui_->snapshot_table_view_->setSelectionMode(QAbstractItemView::MultiSelection);
    const RmtSnapshotPoint* snapshot_point = SnapshotManager::Get().GetSelectedSnapshotPoint();
    if (snapshot_point != nullptr)
    {
        const QModelIndex& index = model_->GetProxyModel()->FindModelIndex(((uintptr_t)snapshot_point), kSnapshotTimelineColumnID);
        if (index.isValid())
        {
            ui_->snapshot_table_view_->selectRow(index.row());
        }
    }
    ui_->snapshot_table_view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void TimelinePane::OnTraceLoad()
{
    model_->Update();

    // Add memory allocation widget. Qt will take ownership of the created timeline so
    // it will get deleted when it's removed from the scene.
    AddTimelineGraph();

    UpdateSnapshotMarkers();

    model_->ValidateTimeUnits();

    ui_->timeline_view_->SetMaxClock(model_->GetMaxTimestamp());
    SwitchTimeUnits();

    ui_->timeline_wrapper_->show();
    ui_->snapshot_table_view_->showColumn(kSnapshotTimelineColumnTime);

    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
    colorizer_->UpdateLegends();

    UpdateTableDisplay();
}

void TimelinePane::UpdateSnapshotMarkers()
{
    TraceManager& trace_manager = TraceManager::Get();
    if (trace_manager.DataSetValid() == false)
    {
        return;
    }

    ui_->timeline_view_->ClearSnapshotMarkers();

    // add snapshot widgets
    RmtDataSet* data_set = trace_manager.GetDataSet();
    for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < data_set->snapshot_count; current_snapshot_point_index++)
    {
        RmtSnapshotPoint*  current_snapshot_point = &data_set->snapshots[current_snapshot_point_index];
        RMVSnapshotMarker* marker                 = AddSnapshot(current_snapshot_point);
        RMT_UNUSED(marker);
    }
    const RmtSnapshotPoint* snapshot_point = SnapshotManager::Get().GetSelectedSnapshotPoint();
    ui_->timeline_view_->SelectSnapshot(snapshot_point);
}

void TimelinePane::OnTraceClose()
{
    // reset the timeline type combo back to default
    int row_index = 0;
    ui_->timeline_type_combo_box_->SetSelectedRow(row_index);
    RmtDataTimelineType new_timeline_type = colorizer_->ApplyColorMode(row_index);
    model_->SetTimelineType(new_timeline_type);

    ui_->timeline_view_->Clear();
}

void TimelinePane::UpdateSelectedDuration(uint64_t duration)
{
    QString text = "-";

    if (duration != 0)
    {
        text = rmv::time_util::ClockToTimeUnit(duration);
    }

    ui_->selection_clock_label_->setText(text);
}

void TimelinePane::UpdateHoverClock(uint64_t clock)
{
    ui_->hover_clock_label_->setText(rmv::time_util::ClockToTimeUnit(clock));
}

void TimelinePane::Reset()
{
    model_->ResetModelValues();
    SnapshotManager::Get().SetSelectedSnapshotPoint(nullptr);

    ZoomReset();

    ui_->size_slider_->SetLowerValue(0);
    ui_->size_slider_->SetUpperValue(rmv::kSizeSliderRange);
    ui_->search_box_->setText("");
}

void TimelinePane::SwitchTimeUnits()
{
    double ratio = rmv::time_util::TimeToClockRatio();
    ui_->timeline_view_->UpdateTimeUnits(RMVSettings::Get().GetUnits(), ratio);
    model_->Update();
}

void TimelinePane::ChangeColoring()
{
    snapshot_legends_->Clear();
    AddSnapshotLegends();
}

void TimelinePane::ZoomInSelection()
{
    const bool zoom = ui_->timeline_view_->ZoomInSelection();

    if (ui_->timeline_view_->RegionSelected())
    {
        UpdateZoomButtonsForZoomIn(zoom);
    }
}

void TimelinePane::ZoomReset()
{
    zoom_icon_manager_->ZoomReset();
    UpdateTimelineScrollbarContextMenu(false);

    ui_->selection_clock_label_->setText("-");

    ui_->timeline_view_->ZoomReset();
    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
}

void TimelinePane::ZoomIn()
{
    const bool zoom = ui_->timeline_view_->ZoomIn(2, false);
    UpdateZoomButtonsForZoomIn(zoom);
}

void TimelinePane::ZoomOut()
{
    const bool zoom = ui_->timeline_view_->ZoomOut(2, false);
    UpdateZoomButtonsForZoomOut(zoom);
}

void TimelinePane::ZoomInCustom(int zoom_rate, bool use_mouse_pos)
{
    const bool zoom = ui_->timeline_view_->ZoomIn(zoom_rate, use_mouse_pos);
    UpdateZoomButtonsForZoomIn(zoom);
}

void TimelinePane::ZoomOutCustom(int zoom_rate, bool use_mouse_pos)
{
    const bool zoom = ui_->timeline_view_->ZoomOut(zoom_rate, use_mouse_pos);
    UpdateZoomButtonsForZoomOut(zoom);
}

void TimelinePane::UpdateZoomButtonsForZoomIn(bool zoom)
{
    zoom_icon_manager_->ZoomIn(zoom);
    UpdateTimelineScrollbarContextMenu(true);

    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
}

void TimelinePane::UpdateZoomButtonsForZoomOut(bool zoom)
{
    zoom_icon_manager_->ZoomOut(zoom);
    UpdateTimelineScrollbarContextMenu(zoom);

    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
}

void TimelinePane::UpdateTimelineScrollbarContextMenu(bool shown)
{
    // hide the right-click context menu on the scrollbar if fully zoomed out.
    QScrollBar* scroll_bar = ui_->timeline_view_->horizontalScrollBar();
    if (scroll_bar != nullptr)
    {
        if (shown == true)
        {
            scroll_bar->setContextMenuPolicy(Qt::DefaultContextMenu);
        }
        else
        {
            scroll_bar->setContextMenuPolicy(Qt::NoContextMenu);
        }
    }
}

void TimelinePane::UpdateZoomButtonsForZoomToSelection(bool selected_region)
{
    zoom_icon_manager_->ZoomToSelection(selected_region);
}

void TimelinePane::ScrollBarChanged()
{
    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
}

void TimelinePane::FilterBySizeSliderChanged(int min_value, int max_value)
{
    model_->FilterBySizeChanged(min_value, max_value);
}

void TimelinePane::SearchBoxChanged()
{
    model_->SearchBoxChanged(ui_->search_box_->text());
    SetMaximumSnapshotTableHeight();
}

void TimelinePane::CompareSnapshots()
{
    QItemSelectionModel* pItemSelectionModel = ui_->snapshot_table_view_->selectionModel();
    QModelIndexList      selected_rows       = pItemSelectionModel->selectedRows();

    if (selected_rows.count() == 2)
    {
        RmtSnapshotPoint* snapshot_point_base =
            reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(selected_rows[kSnapshotCompareBase].row(), kSnapshotTimelineColumnID));
        RmtSnapshotPoint* snapshot_point_diff =
            reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(selected_rows[kSnapshotCompareDiff].row(), kSnapshotTimelineColumnID));

        emit MessageManager::Get().CompareSnapshot(snapshot_point_base, snapshot_point_diff);
    }
}

void TimelinePane::SelectSnapshot(RmtSnapshotPoint* snapshot_point)
{
    ui_->timeline_view_->SelectSnapshot(snapshot_point);

    const QModelIndex& index = model_->GetProxyModel()->FindModelIndex(((uintptr_t)snapshot_point), kSnapshotTimelineColumnID);

    if (index.isValid() == true)
    {
        ui_->snapshot_table_view_->selectRow(index.row());

        QItemSelectionModel* selected_item = ui_->snapshot_table_view_->selectionModel();
        if (selected_item->hasSelection())
        {
            QModelIndexList selected_rows = selected_item->selectedRows();
            if (selected_rows.size() > 0)
            {
                ui_->compare_button_->setEnabled(selected_rows.count() == 2);
            }
        }

        //ui_->snapshot_table_view_->setFocus(Qt::FocusReason::OtherFocusReason);
    }
    SnapshotManager::Get().SetSelectedSnapshotPoint(snapshot_point);
}

void TimelinePane::TableSelectionChanged()
{
    const QItemSelectionModel* selection_model = ui_->snapshot_table_view_->selectionModel();
    const QModelIndexList&     selected_rows   = selection_model->selectedRows();
    const QModelIndex          current_index   = selection_model->currentIndex();
    bool                       is_selected     = selection_model->isSelected(current_index);

    if (is_selected == true)
    {
        if (current_index.isValid())
        {
            RmtSnapshotPoint* selected_snapshot = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(current_index.row(), kSnapshotTimelineColumnID));
            SelectSnapshot(selected_snapshot);
        }
    }
    else
    {
        SnapshotManager::Get().SetSelectedSnapshotPoint(nullptr);
        ui_->timeline_view_->SelectSnapshot(nullptr);
    }

    // Enable the compare button if 2 entries in the table are selected
    ui_->compare_button_->setEnabled(selected_rows.count() == 2);
}

void TimelinePane::TableDoubleClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        RmtSnapshotPoint* snapshot_point = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(index.row(), kSnapshotTimelineColumnID));

        emit MessageManager::Get().OpenSnapshot(snapshot_point);
    }
}

void TimelinePane::GenerateSnapshotAtTime(uint64_t snapshot_time)
{
    RmtSnapshotPoint* snapshot_point = model_->AddSnapshot(snapshot_time);
    if (snapshot_point != nullptr)
    {
        ui_->timeline_view_->AddSnapshot(snapshot_point);
        UpdateTableDisplay();
    }
}

RMVTimelineGraph* TimelinePane::AddTimelineGraph()
{
    RMVTimelineGraph* new_object = ui_->timeline_view_->AddTimelineGraph(model_, colorizer_);
    return new_object;
}

RMVSnapshotMarker* TimelinePane::AddSnapshot(RmtSnapshotPoint* snapshot_point)
{
    if (snapshot_point == nullptr)
    {
        return nullptr;
    }

    RMVSnapshotMarker* new_marker = ui_->timeline_view_->AddSnapshot(snapshot_point);
    return new_marker;
}

void TimelinePane::RemoveSnapshot(RmtSnapshotPoint* snapshot_point)
{
    const TraceManager& trace_manager = TraceManager::Get();
    if (trace_manager.DataSetValid() == false)
    {
        return;
    }

    // if the snapshot point has a cached snapshot (i.e.: there's a chance its open, then look at closing it)
    if (snapshot_point->cached_snapshot)
    {
        // if we're about to remove the snapshot that's open, then signal to everyone its about to vanish.
        const RmtDataSnapshot* open_snapshot = trace_manager.GetOpenSnapshot();
        if (open_snapshot == snapshot_point->cached_snapshot)
        {
            if (open_snapshot != nullptr)
            {
                emit MessageManager::Get().OpenSnapshot(nullptr);
            }
        }

        if (trace_manager.GetComparedSnapshot(kSnapshotCompareBase) == snapshot_point->cached_snapshot ||
            trace_manager.GetComparedSnapshot(kSnapshotCompareDiff) == snapshot_point->cached_snapshot)
        {
            if (trace_manager.GetComparedSnapshot(kSnapshotCompareBase) != nullptr || trace_manager.GetComparedSnapshot(kSnapshotCompareDiff) != nullptr)
            {
                emit MessageManager::Get().CompareSnapshot(nullptr, nullptr);
            }
        }
    }

    // deselect the selected snapshot if it's being removed
    if (snapshot_point == SnapshotManager::Get().GetSelectedSnapshotPoint())
    {
        SnapshotManager::Get().SetSelectedSnapshotPoint(nullptr);
    }

    model_->RemoveSnapshot(snapshot_point);

    UpdateSnapshotMarkers();
    UpdateTableDisplay();
}

void TimelinePane::RenameSnapshotByIndex(int32_t snapshot_index)
{
    QModelIndex model_index = ui_->snapshot_table_view_->model()->index(snapshot_index, kSnapshotTimelineColumnName, QModelIndex());
    ui_->snapshot_table_view_->edit(model_index);
}

void TimelinePane::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();

    if (ui_->timeline_view_->GetResetState() == false)
    {
        if (keyboard_zoom_shortcuts_->KeyPressed(key, event->isAutoRepeat()) == false)
        {
            QWidget::keyPressEvent(event);
        }
    }
}

void TimelinePane::keyReleaseEvent(QKeyEvent* event)
{
    const int key = event->key();

    if (keyboard_zoom_shortcuts_->KeyReleased(key, event->isAutoRepeat()) == false)
    {
        QWidget::keyReleaseEvent(event);
    }
}

void TimelinePane::resizeEvent(QResizeEvent* event)
{
    if (model_ != nullptr)
    {
        model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
    }
    QWidget::resizeEvent(event);
}

void TimelinePane::contextMenuEvent(QContextMenuEvent* event)
{
    // Check that there are exactly two selected objects -- offer to compare them.
    // Else offer to remove the snapshot.
    QItemSelectionModel* item_selection_model = ui_->snapshot_table_view_->selectionModel();

    if (!item_selection_model->hasSelection())
    {
        return;
    }

    // Get the number of rows in the table selected
    QModelIndexList selected_rows = item_selection_model->selectedRows();

    if (selected_rows.count() == 1)
    {
        // if 1 row selected, allow user to rename or delete a snapshot
        QMenu   menu;
        QAction rename_action(kRenameAction);
        QAction delete_action(kDeleteAction);

        menu.addAction(&rename_action);
        menu.addAction(&delete_action);

        QAction* pAction = menu.exec(event->globalPos());

        if (pAction != nullptr)
        {
            QString selection_text = pAction->text();
            if (selection_text.compare(kDeleteAction) == 0)
            {
                RmtSnapshotPoint* snapshot_point = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(selected_rows[0].row(), kSnapshotTimelineColumnID));
                RemoveSnapshot(snapshot_point);
            }
            else if (selection_text.compare(kRenameAction) == 0)
            {
                const int32_t snapshot_id = selected_rows[0].row();
                RenameSnapshotByIndex(snapshot_id);
            }
        }
        return;
    }

    if (selected_rows.count() == 2)
    {
        // if 2 rows selected, allow user to compare snapshots
        QAction action(kCompareAction);

        QMenu menu;
        menu.addAction(&action);

        QAction* pAction = menu.exec(event->globalPos());

        if (pAction != nullptr)
        {
            RmtSnapshotPoint* snapshot_point_base =
                reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(selected_rows[kSnapshotCompareBase].row(), kSnapshotTimelineColumnID));
            RmtSnapshotPoint* snapshot_point_diff =
                reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(selected_rows[kSnapshotCompareDiff].row(), kSnapshotTimelineColumnID));
            emit MessageManager::Get().CompareSnapshot(snapshot_point_base, snapshot_point_diff);
        }

        return;
    }
}

void TimelinePane::TimelineTypeChanged()
{
    int index = ui_->timeline_type_combo_box_->CurrentRow();
    if (index >= 0)
    {
        RmtDataTimelineType new_timeline_type = colorizer_->ApplyColorMode(index);
        model_->SetTimelineType(new_timeline_type);

        // start the processing thread and pass in the worker object. The thread controller will take ownership
        // of the worker and delete it once it's complete
        thread_controller_ = new rmv::ThreadController(main_window_, ui_->timeline_view_, new TimelineWorker(model_, new_timeline_type));

        // when the worker thread has finished, a signal will be emitted. Wait for the signal here and update
        // the UI with the newly acquired data from the worker thread
        connect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &TimelinePane::TimelineWorkerThreadFinished);
    }
}

void TimelinePane::TimelineWorkerThreadFinished()
{
    disconnect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &TimelinePane::TimelineWorkerThreadFinished);
    thread_controller_->deleteLater();
    thread_controller_ = nullptr;

    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
    ui_->timeline_view_->viewport()->update();
    colorizer_->UpdateLegends();
}

void TimelinePane::ScrollToSelectedSnapshot()
{
    QItemSelectionModel* selected_item = ui_->snapshot_table_view_->selectionModel();
    if (selected_item->hasSelection())
    {
        QModelIndexList item_list = selected_item->selectedRows();
        if (item_list.size() > 0)
        {
            // get the model index of the name column since column 0 (ID) is hidden and scrollTo
            // doesn't appear to scroll on hidden columns
            QModelIndex model_index = model_->GetProxyModel()->index(item_list[0].row(), kSnapshotTimelineColumnName);
            ui_->snapshot_table_view_->scrollTo(model_index, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}

void TimelinePane::UpdateTableDisplay()
{
    int index = (model_->RowCount() == 0) ? 0 : 1;
    ui_->snapshot_table_valid_switch_->setCurrentIndex(index);

    SetMaximumSnapshotTableHeight();
}

void TimelinePane::AddSnapshotLegends()
{
    // Commented out for now but kept for reference as may be used later.
    //snapshot_legends_->AddColorLegendItem(GetSnapshotTypeColor(RMV_SNAPSHOT_TYPE_LIVE), "Live snapshot");
    //snapshot_legends_->AddColorLegendItem(GetSnapshotTypeColor(RMV_SNAPSHOT_TYPE_GENERATED), "Generated snapshot");
}
