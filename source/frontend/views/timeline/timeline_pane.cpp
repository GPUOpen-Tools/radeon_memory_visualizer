//=============================================================================
// Copyright (c) 2018-2022 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of Timeline pane.
//=============================================================================

#include "views/timeline/timeline_pane.h"

#include <QScrollBar>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

#include "qt_common/custom_widgets/double_slider_widget.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "managers/message_manager.h"
#include "managers/pane_manager.h"
#include "managers/snapshot_manager.h"
#include "managers/trace_manager.h"
#include "models/proxy_models/snapshot_timeline_proxy_model.h"
#include "models/timeline/snapshot_item_model.h"
#include "models/timeline/timeline_model.h"
#include "settings/rmv_settings.h"
#include "util/time_util.h"

#ifndef _WIN32
#include <linux/safe_crt.h>
#endif

const static QString kRenameAction  = "Rename snapshot";
const static QString kDeleteAction  = "Delete snapshot";
const static QString kCompareAction = "Compare snapshots";

TimelinePane::TimelinePane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::TimelinePane)
    , thread_controller_(nullptr)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);

    // Fix up the ratios of the 2 splitter regions.
    ui_->splitter_->setStretchFactor(0, 5);
    ui_->splitter_->setStretchFactor(1, 4);

    // Initialize the snapshot legends.
    rmv::widget_util::InitGraphicsView(ui_->snapshot_legends_view_, rmv::kColoredLegendsHeight);
    rmv::widget_util::InitColorLegend(snapshot_legends_, ui_->snapshot_legends_view_);
    AddSnapshotLegends();

    model_ = new rmv::TimelineModel();

    model_->InitializeModel(ui_->snapshot_count_label_, rmv::kTimelineSnapshotCount, "text");
    model_->InitializeTableModel(ui_->snapshot_table_view_, 0, rmv::kSnapshotTimelineColumnCount);

    // Set default columns widths appropriately so that they can show the table contents.
    ui_->snapshot_table_view_->SetColumnPadding(0);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnID, 10);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnName, 11);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnTime, 10);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnVirtualAllocations, 12);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnResources, 9);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnAllocatedTotalVirtualMemory, 14);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnAllocatedBoundVirtualMemory, 14);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnAllocatedUnboundVirtualMemory, 16);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnCommittedLocal, 16);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnCommittedInvisible, 18);
    ui_->snapshot_table_view_->SetColumnWidthEms(rmv::kSnapshotTimelineColumnCommittedHost, 16);

    // Allow users to resize columns if desired.
    ui_->snapshot_table_view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    rmv::widget_util::UpdateTablePalette(ui_->snapshot_table_view_);

    ui_->snapshot_table_view_->horizontalHeader()->setSectionsClickable(true);
    ui_->snapshot_table_view_->setSortingEnabled(true);
    ui_->snapshot_table_view_->sortByColumn(rmv::kSnapshotTimelineColumnTime, Qt::AscendingOrder);
    ui_->snapshot_table_view_->setEditTriggers(QAbstractItemView::EditKeyPressed);

    // Hide columns that we are using for sorting.
    ui_->snapshot_table_view_->hideColumn(rmv::kSnapshotTimelineColumnID);

    // Hide the snapshot legends for now. Currently not used but maybe needed in future.
    ui_->snapshot_legends_controls_wrapper_->hide();

    // Initialize the timeline type combo box.
    colorizer_ = new rmv::TimelineColorizer();

    // Set up a list of required timeline modes, in order.
    // The list is terminated with -1.
    static const RmtDataTimelineType type_list[] = {kRmtDataTimelineTypeResourceUsageVirtualSize,
                                                    kRmtDataTimelineTypeResourceUsageCount,
                                                    kRmtDataTimelineTypeVirtualMemory,
                                                    kRmtDataTimelineTypeCommitted,
                                                    // kRmtDataTimelineTypeProcess,
                                                    RmtDataTimelineType(-1)};

    // Initialize the "color by" UI elements. Set up the combo box, legends and signals etc.
    colorizer_->Initialize(parent, ui_->timeline_type_combo_box_, ui_->timeline_legends_view_, type_list);
    connect(ui_->timeline_type_combo_box_, &ArrowIconComboBox::SelectionChanged, this, &TimelinePane::TimelineTypeChanged);

    model_->SetTimelineType(type_list[0]);

    // Allow multiple snapshots to be selected so they can be compared.
    ui_->snapshot_table_view_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Set up the zoom buttons.
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

    // Hide size slider for now.
    ui_->size_slider_->hide();
    ui_->size_slider_label_->hide();

    // Disable the compare button.
    ui_->compare_button_->setEnabled(false);

    keyboard_zoom_shortcuts_ = new rmv::KeyboardZoomShortcutsTimeline(this, ui_->timeline_view_->horizontalScrollBar(), ui_->timeline_view_);

    connect(keyboard_zoom_shortcuts_, &rmv::KeyboardZoomShortcutsTimeline::ZoomInSelectionSignal, this, &TimelinePane::ZoomInSelection);
    connect(keyboard_zoom_shortcuts_, &rmv::KeyboardZoomShortcutsTimeline::ResetViewSignal, this, &TimelinePane::ZoomReset);
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
    connect(&rmv::SnapshotManager::Get(), &rmv::SnapshotManager::SnapshotMarkerSelected, this, &TimelinePane::UpdateSnapshotTable);

    // Set up a connection between the timeline being sorted and making sure the selected event is visible.
    connect(model_->GetProxyModel(), &rmv::SnapshotTimelineProxyModel::layoutChanged, this, &TimelinePane::ScrollToSelectedSnapshot);
}

TimelinePane::~TimelinePane()
{
    delete zoom_icon_manager_;
    delete colorizer_;
    delete model_;
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
    ui_->snapshot_table_view_->clearSelection();

    // Temporarily set multi-selection on the table so multiple rows can be selected.
    // MultiSelection will toggle the selected row and leave all other rows unchanged.
    // ExtendedSelection will deselect the last row selected before selecting the new row
    // (unless ctrl or shift are pressed).
    ui_->snapshot_table_view_->setSelectionMode(QAbstractItemView::MultiSelection);

    // Cache the snapshot points since selectRow() will alter their values in the snapshot manager.
    const RmtSnapshotPoint* snapshot_point      = rmv::SnapshotManager::Get().GetSelectedSnapshotPoint();
    const RmtSnapshotPoint* diff_snapshot_point = rmv::SnapshotManager::Get().GetSelectedCompareSnapshotPointDiff();

    // Do the diff snapshot point first if valid, since the last selected snapshot will be the one used for single snapshot mode.
    if (diff_snapshot_point != nullptr)
    {
        const QModelIndex& index = model_->GetProxyModel()->FindModelIndex(((uintptr_t)diff_snapshot_point), rmv::kSnapshotTimelineColumnID);
        if (index.isValid())
        {
            ui_->snapshot_table_view_->selectRow(index.row());
        }
    }

    if (snapshot_point != nullptr && snapshot_point != diff_snapshot_point)
    {
        const QModelIndex& index = model_->GetProxyModel()->FindModelIndex(((uintptr_t)snapshot_point), rmv::kSnapshotTimelineColumnID);
        if (index.isValid())
        {
            ui_->snapshot_table_view_->selectRow(index.row());
        }
    }

    // Restore table selection mode.
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
    ui_->snapshot_table_view_->showColumn(rmv::kSnapshotTimelineColumnTime);

    model_->UpdateMemoryGraph(ui_->timeline_view_->ViewableStartClk(), ui_->timeline_view_->ViewableEndClk());
    colorizer_->UpdateLegends();

    UpdateTableDisplay();
}

void TimelinePane::UpdateSnapshotMarkers()
{
    rmv::TraceManager& trace_manager = rmv::TraceManager::Get();
    if (trace_manager.DataSetValid() == false)
    {
        return;
    }

    ui_->timeline_view_->ClearSnapshotMarkers();

    // Add snapshot widgets.
    RmtDataSet* data_set = trace_manager.GetDataSet();
    for (int32_t current_snapshot_point_index = 0; current_snapshot_point_index < data_set->snapshot_count; current_snapshot_point_index++)
    {
        RmtSnapshotPoint*  current_snapshot_point = &data_set->snapshots[current_snapshot_point_index];
        RMVSnapshotMarker* marker                 = AddSnapshot(current_snapshot_point);
        RMT_UNUSED(marker);
    }
    const RmtSnapshotPoint* snapshot_point = rmv::SnapshotManager::Get().GetSelectedSnapshotPoint();
    ui_->timeline_view_->SelectSnapshot(snapshot_point);
}

void TimelinePane::OnTraceClose()
{
    // Reset the timeline type combo back to default.
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
    rmv::SnapshotManager::Get().SetSelectedSnapshotPoint(nullptr);
    rmv::SnapshotManager::Get().SetSelectedCompareSnapshotPoints(nullptr, nullptr);

    ZoomReset();

    ui_->size_slider_->SetLowerValue(0);
    ui_->size_slider_->SetUpperValue(rmv::kSizeSliderRange);
    ui_->search_box_->setText("");
}

void TimelinePane::SwitchTimeUnits()
{
    double ratio = rmv::time_util::TimeToClockRatio();
    ui_->timeline_view_->UpdateTimeUnits(rmv::RMVSettings::Get().GetUnits(), ratio);
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
    // Hide the right-click context menu on the scrollbar if fully zoomed out.
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
    const QItemSelectionModel* selection_model = ui_->snapshot_table_view_->selectionModel();
    const QModelIndexList      selected_rows   = selection_model->selectedRows();

    if (selected_rows.count() == 2)
    {
        emit rmv::SnapshotManager::Get().CompareSnapshotsOpened();
    }
}

void TimelinePane::UpdateSnapshotTable(const RmtSnapshotPoint* snapshot_point) const
{
    const QModelIndex& selected_index = model_->GetProxyModel()->FindModelIndex(((uintptr_t)snapshot_point), rmv::kSnapshotTimelineColumnID);

    if (selected_index.isValid() == true)
    {
        ui_->snapshot_table_view_->selectRow(selected_index.row());
    }
}

void TimelinePane::TableSelectionChanged()
{
    const QItemSelectionModel* selection_model = ui_->snapshot_table_view_->selectionModel();
    const QModelIndex          current_index   = selection_model->currentIndex();
    bool                       is_selected     = selection_model->isSelected(current_index);

    RmtSnapshotPoint* selected_snapshot = nullptr;
    if (is_selected == true)
    {
        if (current_index.isValid())
        {
            selected_snapshot = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(current_index.row(), rmv::kSnapshotTimelineColumnID));
        }
    }

    // If no snapshot is selected, this could have been caused by the user deselecting a snapshot, leaving 2 or less snapshots
    // selected. In this case, pick the topmost snapshot to use as the snapshot that is currently highlighted.
    if (selected_snapshot == nullptr)
    {
        const QModelIndexList& selected_rows = selection_model->selectedRows();
        const int              count         = selected_rows.count();
        if (count > 0)
        {
            int base_row      = selected_rows[0].row();
            selected_snapshot = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(base_row, rmv::kSnapshotTimelineColumnID));
        }
    }

    ui_->timeline_view_->SelectSnapshot(selected_snapshot);

    // Assign the selected rows in the table to selected snapshots in the snapshot manager.
    if (selection_model->hasSelection())
    {
        const QModelIndexList& selected_rows = selection_model->selectedRows();
        const int              count         = selected_rows.count();

        if (count == 2)
        {
            // Make sure the row for the base snapshot is the entry that is selected.
            const QModelIndex& selected_index = model_->GetProxyModel()->FindModelIndex(((uintptr_t)selected_snapshot), rmv::kSnapshotTimelineColumnID);

            int row0     = selected_rows[0].row();
            int row1     = selected_rows[1].row();
            int base_row = selected_index.row();
            int diff_row = (base_row == row0) ? row1 : row0;
            Q_ASSERT(base_row != diff_row);

            // Enable comparing of snapshots.
            RmtSnapshotPoint* snapshot_point_base = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(base_row, rmv::kSnapshotTimelineColumnID));
            RmtSnapshotPoint* snapshot_point_diff = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(diff_row, rmv::kSnapshotTimelineColumnID));
            rmv::SnapshotManager::Get().SetSelectedCompareSnapshotPoints(snapshot_point_base, snapshot_point_diff);
            ui_->compare_button_->setEnabled(true);
        }
        else
        {
            // Comparing snapshots not valid since 2 entries in the table are not selected.
            rmv::SnapshotManager::Get().SetSelectedCompareSnapshotPoints(nullptr, nullptr);
            ui_->compare_button_->setEnabled(false);
        }
    }

    rmv::SnapshotManager::Get().SetSelectedSnapshotPoint(selected_snapshot);
}

void TimelinePane::TableDoubleClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        RmtSnapshotPoint* snapshot_point = reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(index.row(), rmv::kSnapshotTimelineColumnID));
        if (snapshot_point)
        {
            rmv::SnapshotManager::Get().SetSelectedSnapshotPoint(snapshot_point);
            emit rmv::SnapshotManager::Get().SnapshotOpened();
        }
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
    return ui_->timeline_view_->AddTimelineGraph(model_, colorizer_);
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
    const rmv::TraceManager& trace_manager = rmv::TraceManager::Get();
    if (trace_manager.DataSetValid() == false)
    {
        return;
    }

    rmv::SnapshotManager::Get().RemoveSnapshot(snapshot_point);

    model_->RemoveSnapshot(snapshot_point);

    emit rmv::MessageManager::Get().TitleBarChanged();
    UpdateSnapshotMarkers();
    UpdateTableDisplay();
}

void TimelinePane::RenameSnapshotByIndex(int32_t snapshot_index)
{
    QModelIndex model_index = ui_->snapshot_table_view_->model()->index(snapshot_index, rmv::kSnapshotTimelineColumnName, QModelIndex());
    ui_->snapshot_table_view_->edit(model_index);
    emit rmv::MessageManager::Get().TitleBarChanged();
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
    UpdateTableDisplay();
    QWidget::resizeEvent(event);
}

void TimelinePane::contextMenuEvent(QContextMenuEvent* event)
{
    // Check that there are exactly two selected objects -- offer to compare them.
    // Else offer to remove the snapshot.
    const QItemSelectionModel* selection_model = ui_->snapshot_table_view_->selectionModel();

    if (!selection_model->hasSelection())
    {
        return;
    }

    // Get the number of rows in the table selected.
    QModelIndexList selected_rows = selection_model->selectedRows();

    if (selected_rows.count() == 1)
    {
        // If 1 row selected, allow user to rename or delete a snapshot.
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
                RmtSnapshotPoint* snapshot_point =
                    reinterpret_cast<RmtSnapshotPoint*>(model_->GetProxyData(selected_rows[0].row(), rmv::kSnapshotTimelineColumnID));
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
        // If 2 rows selected, allow user to compare snapshots.
        QAction action(kCompareAction);

        QMenu menu;
        menu.addAction(&action);

        // Make sure the table is up to date. In the case where 3 snapshots are chosen, then 1 is deselected,
        // there won't be a selected table entry. If there are 2 snapshots selected in the table, these will be set up
        // for comparison.
        TableSelectionChanged();

        QAction* pAction = menu.exec(event->globalPos());

        if (pAction != nullptr)
        {
            emit rmv::SnapshotManager::Get().CompareSnapshotsOpened();
        }

        return;
    }
}

void TimelinePane::TimelineTypeChanged()
{
    const rmv::TraceManager& trace_manager = rmv::TraceManager::Get();
    if (trace_manager.DataSetValid())
    {
        const int index = ui_->timeline_type_combo_box_->CurrentRow();
        if (index >= 0)
        {
            RmtDataTimelineType new_timeline_type = colorizer_->ApplyColorMode(index);
            model_->SetTimelineType(new_timeline_type);

            // Start the processing thread and pass in the worker object. The thread controller will take ownership
            // of the worker and delete it once it's complete.
            thread_controller_ = new rmv::ThreadController(ui_->timeline_view_, model_->CreateWorkerThread(new_timeline_type));

            // When the worker thread has finished, a signal will be emitted. Wait for the signal here and update
            // the UI with the newly acquired data from the worker thread.
            connect(thread_controller_, &rmv::ThreadController::ThreadFinished, this, &TimelinePane::TimelineWorkerThreadFinished);
        }
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
            // Get the model index of the name column since column 0 (ID) is hidden and scrollTo
            // doesn't appear to scroll on hidden columns.
            QModelIndex model_index = model_->GetProxyModel()->index(item_list[0].row(), rmv::kSnapshotTimelineColumnName);
            ui_->snapshot_table_view_->scrollTo(model_index, QAbstractItemView::ScrollHint::PositionAtTop);
        }
    }
}

void TimelinePane::UpdateTableDisplay()
{
    int index = (model_->RowCount() == 0) ? 0 : 1;
    ui_->snapshot_table_valid_switch_->setCurrentIndex(index);
    ui_->snapshot_table_view_->setFocus();
    const QAbstractItemModel* source_model = model_->GetProxyModel()->sourceModel();
    if (source_model != nullptr)
    {
        // Find the index of the added snapshot and select it in the table.
        QModelIndex selection_index = source_model->index(model_->RowCount() - 1, 0);
        ui_->snapshot_table_view_->selectRow(model_->GetProxyModel()->mapFromSource(selection_index).row());
    }
    SetMaximumSnapshotTableHeight();
}

void TimelinePane::AddSnapshotLegends()
{
    // Commented out for now but kept for reference as may be used later.
    //snapshot_legends_->AddColorLegendItem(GetSnapshotTypeColor(RMV_SNAPSHOT_TYPE_LIVE), "Live snapshot");
    //snapshot_legends_->AddColorLegendItem(GetSnapshotTypeColor(RMV_SNAPSHOT_TYPE_GENERATED), "Generated snapshot");
}
