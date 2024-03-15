//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Timeline pane.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_TIMELINE_PANE_H_
#define RMV_VIEWS_TIMELINE_TIMELINE_PANE_H_

#include "ui_timeline_pane.h"

#include "qt_common/custom_widgets/colored_legend_scene.h"
#include "qt_common/custom_widgets/scaled_table_view.h"
#include "qt_common/utils/zoom_icon_group_manager.h"

#include "models/timeline/timeline_colorizer.h"
#include "models/timeline/timeline_model.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_snapshot_marker.h"
#include "views/custom_widgets/rmv_timeline_graph.h"
#include "views/timeline/keyboard_zoom_shortcuts_timeline.h"
#include "util/definitions.h"
#include "util/thread_controller.h"
#include "util/widget_util.h"

/// @brief Class declaration for the timeline pane.
class TimelinePane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit TimelinePane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~TimelinePane();

    // Overridden Qt events.

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event The event.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Key pressed event handler.
    ///
    /// @param [in] event The event data for this event.
    virtual void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

    /// @brief Key released event handler.
    ///
    /// @param [in] event The event data for this event.
    virtual void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

    /// @brief Create a context menu to add a new snapshot.
    ///
    /// @param [in] event The event.
    virtual void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Refresh what's visible on the UI after a trace has loaded.
    void OnTraceLoad();

    // Handlers from BasePane

    /// @brief Clean up.
    virtual void OnTraceClose() Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update time units.
    virtual void SwitchTimeUnits() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

public slots:
    /// @brief Zoom into selection box.
    void ZoomInSelection();

    /// @brief Reset view.
    void ZoomReset();

    /// @brief Zoom in by 2x.
    void ZoomIn();

    /// @brief Zoom out by 2x.
    void ZoomOut();

    /// @brief Wrapper around zoom in function.
    ///
    /// @param [in] zoom_rate     The zoom rate (a value of 2 would be a x2 zoom in).
    /// @param [in] use_mouse_pos If true, zoom around the mouse position, otherwise zoom
    ///  around the view center position.
    void ZoomInCustom(int zoom_rate, bool use_mouse_pos);

    /// @brief Wrapper around zoom out function.
    ///
    /// @param [in] zoom_rate     The zoom rate (a value of 2 would be a x2 zoom out).
    /// @param [in] use_mouse_pos If true, zoom around the mouse position, otherwise zoom
    ///  around the view center position.
    void ZoomOutCustom(int zoom_rate, bool use_mouse_pos);

    /// @brief Update the duration label.
    ///
    /// @param [in] duration The new duration.
    void UpdateSelectedDuration(uint64_t duration);

    /// @brief Update the hover over clock label.
    ///
    /// @param [in] clock The new timestamp in clocks.
    void UpdateHoverClock(uint64_t clock);

private slots:
    /// @brief Slot to handle what happens when the 'filter by size' slider changes.
    ///
    /// @param [in] min_value Minimum value of slider span.
    /// @param [in] max_value Maximum value of slider span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

    /// @brief Handle what happens when user changes the search filter.
    void SearchBoxChanged();

    /// @brief Slot to compare 2 snapshots via the "compare snapshots" button.
    ///
    /// The snapshots have already been selected in the snapshot table.
    void CompareSnapshots();

    /// @brief Update the snapshot table if a snapshot marker is clicked on.
    ///
    /// @param [in] snapshot_point The snapshot selected.
    void UpdateSnapshotTable(const RmtSnapshotPoint* snapshot_point) const;

    /// @brief Highlight an entry in the snapshot table.
    void TableSelectionChanged();

    /// @brief Handle what happens when an item in the snapshot table is double-clicked on.
    ///
    /// @param [in] index The model index of the item clicked.
    void TableDoubleClicked(const QModelIndex& index);

    /// @brief Create a new snapshot.
    ///
    /// @param [in] snapshot_time The time the snapshot was taken.
    void GenerateSnapshotAtTime(uint64_t snapshot_time);

    /// @brief Set the zoom buttons to the correct configuration after a zoom in.
    ///
    /// @param [in] zoom Flag indicating if further zoom possible, true if so.
    void UpdateZoomButtonsForZoomIn(bool zoom);

    /// @brief Set the zoom buttons to the correct configuration after a zoom out.
    ///
    /// @param [in] zoom Flag indicating if further zoom possible, true if so.
    void UpdateZoomButtonsForZoomOut(bool zoom);

    /// @brief Set the zoom to selection icon to the correct state after a region selection.
    ///
    /// @param [in] selected_region If true, a region is selected.
    void UpdateZoomButtonsForZoomToSelection(bool selected_region);

    /// @brief Slot to handle what happens when the scroll bar on the timeline changes.
    void ScrollBarChanged();

    /// @brief Slot to handle what happens when the timeline type changes via the combo box.
    void TimelineTypeChanged();

    /// @brief Slot to handle what happens when the timeline worker thread has finished.
    void TimelineWorkerThreadFinished();

    /// @brief Slot to handle cancelling the worker thread.
    void TimelineWorkerThreadCancelled();

    /// @brief Slot to handle what happens after the resource list table is sorted.
    ///
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedSnapshot();

private:
    /// @brief Update the snapshot markers on the graph.
    void UpdateSnapshotMarkers();

    /// @brief Add a new allocation graph to the timeline.
    ///
    /// @return pointer to a new timeline graphics object.
    RMVTimelineGraph* AddTimelineGraph();

    /// @brief Add a new snapshot to the timeline.
    ///
    /// @param [in] snapshot_point The snapshot point to add.
    ///
    /// @return pointer to a new marker graphics object.
    RMVSnapshotMarker* AddSnapshot(RmtSnapshotPoint* snapshot_point);

    /// @brief Remove a snapshot from the timeline.
    ///
    /// @param [in] snapshot_point The snapshot point.
    void RemoveSnapshot(RmtSnapshotPoint* snapshot_point);

    /// @brief Rename a snapshot from the timeline.
    ///
    /// @param [in] snapshot_index The index of the snapshot to rename.
    void RenameSnapshotByIndex(int snapshot_index);

    /// @brief Update the snapshot table display area. If no snapshots have been created, inform the user as such.
    void UpdateTableDisplay();

    /// @brief Select rows in the snapshots table depending on which snapshot is currently selected.
    void SelectTableRows();

    /// @brief Add the snapshot legends to the required scene.
    void AddSnapshotLegends();

    /// @brief Update the right-click context menu on the timeline scrollbar.
    ///
    /// By default this is shown but it should be disabled if the view is fully zoomed out.
    ///
    /// @param [in] shown Should the context menu be shown.
    void UpdateTimelineScrollbarContextMenu(bool shown);

    /// @brief Update the label used to display the selection duration and mouse hover position on the timeline.
    void UpdateClockAndSelectionLabel();

    /// @brief Helper function to set the maximum height of the table so it only contains rows with valid data.
    inline void SetMaximumSnapshotTableHeight()
    {
        ui_->snapshot_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->snapshot_table_view_, model_->RowCount()));
    }

    Ui::TimelinePane*                   ui_;                            ///< Pointer to the Qt UI design.
    ZoomIconGroupManager*               zoom_icon_manager_;             ///< The object responsible for the zoom icon status.
    rmv::TimelineModel*                 model_;                         ///< Container class for the widget models.
    ColoredLegendScene*                 snapshot_legends_;              ///< Snapshot legends above the timeline.
    rmv::KeyboardZoomShortcutsTimeline* keyboard_zoom_shortcuts_;       ///< Keyboard shortcut handler.
    rmv::TimelineColorizer*             colorizer_;                     ///< The colorizer used by the 'timeline type' combo box.
    rmv::ThreadController*              thread_controller_;             ///< The thread for processing backend data.
    uint64_t                            hover_clock_;                   ///< The mouse position in clocks on the timeline.
    uint64_t                            selection_duration_in_clocks_;  ///< The duration of the timeline selection in clocks.
};

#endif  // RMV_VIEWS_TIMELINE_TIMELINE_PANE_H_
