//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Timeline pane.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_TIMELINE_PANE_H_
#define RMV_VIEWS_TIMELINE_TIMELINE_PANE_H_

#include "ui_timeline_pane.h"

#include "qt_common/custom_widgets/colored_legend_scene.h"
#include "qt_common/custom_widgets/scaled_table_view.h"
#include "qt_common/utils/zoom_icon_group_manager.h"

#include "rmt_data_set.h"

#include "models/timeline/timeline_model.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_snapshot_marker.h"
#include "views/custom_widgets/rmv_timeline_graph.h"
#include "views/timeline_colorizer.h"
#include "views/timeline/keyboard_zoom_shortcuts_timeline.h"
#include "util/definitions.h"
#include "util/thread_controller.h"
#include "util/widget_util.h"

class MainWindow;

/// Class declaration for the timeline pane
class TimelinePane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit TimelinePane(MainWindow* parent = nullptr);

    /// Destructor.
    virtual ~TimelinePane();

    // Overridden Qt events.

    /// Overridden show event. Fired when this pane is opened.
    /// \param event The event.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Key pressed event handler.
    /// \param event The event data for this event.
    virtual void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

    /// Key released event handler.
    /// \param event The event data for this event.
    virtual void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

    /// Create a context menu to add a new snapshot.
    /// \param event The event.
    virtual void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Refresh what's visible on the UI.
    void OnTraceLoad();

    // Handlers from BasePane

    /// Clean up.
    virtual void OnTraceClose() Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// Update time units.
    virtual void SwitchTimeUnits() Q_DECL_OVERRIDE;

    /// Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

public slots:
    /// Zoom into selection box.
    void ZoomInSelection();

    /// Reset view.
    void ZoomReset();

    /// Zoom in by 2x.
    void ZoomIn();

    /// Zoom out by 2x.
    void ZoomOut();

    /// Wrapper around zoom in function.
    /// \param zoom_rate The zoom rate (a value of 2 would be a x2 zoom in).
    /// \param use_mouse_pos If true, zoom around the mouse position, otherwise zoom
    ///  around the view center position.
    void ZoomInCustom(int zoom_rate, bool use_mouse_pos);

    /// Wrapper around zoom out function.
    /// \param zoom_rate The zoom rate (a value of 2 would be a x2 zoom out).
    /// \param use_mouse_pos If true, zoom around the mouse position, otherwise zoom
    ///  around the view center position.
    void ZoomOutCustom(int zoom_rate, bool use_mouse_pos);

    /// Update the duration label.
    /// \param duration The new duration.
    void UpdateSelectedDuration(uint64_t duration);

    /// Update the hover over clock label.
    /// \param clock The new timestamp in clocks.
    void UpdateHoverClock(uint64_t clock);

private slots:
    /// Slot to handle what happens when the 'filter by size' slider changes.
    /// \param min_value Minimum value of slider span.
    /// \param max_value Maximum value of slider span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

    /// Handle what happens when user changes the search filter.
    void SearchBoxChanged();

    /// Slot to compare 2 snapshots via the "compare snsapshots" button. The
    /// snapshots have already been selected in the snapshot table.
    void CompareSnapshots();

    /// Select a snapshot.
    /// \param snapshot_point The snapshot selected.
    void SelectSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Highlight an entry in the snapshot table.
    void TableSelectionChanged();

    /// Handle what happens when an item in the snapshot table is double-clicked on.
    /// \param index The model index of the item clicked.
    void TableDoubleClicked(const QModelIndex& index);

    /// Create a new snapshot.
    /// \param snapshot_time The time the snapshot was taken.
    void GenerateSnapshotAtTime(uint64_t snapshot_time);

    /// Set the zoom buttons to the correct configuration after a zoom in.
    /// \param zoom Flag indicating if further zoom possible, true if so.
    void UpdateZoomButtonsForZoomIn(bool zoom);

    /// Set the zoom buttons to the correct configuration after a zoom out.
    /// \param zoom Flag indicating if further zoom possible, true if so.
    void UpdateZoomButtonsForZoomOut(bool zoom);

    /// Set the zoom to selection icon to the correct state after a region selection.
    /// \param selectedRegion If true, a region is selected.
    void UpdateZoomButtonsForZoomToSelection(bool selected_region);

    /// Slot to handle what happens when the scroll bar on the timeline changes.
    void ScrollBarChanged();

    /// Slot to handle what happens when the timeline type changes via the combo box.
    void TimelineTypeChanged();

    /// Slot to handle what happens when the timeline worker thread has finished.
    void TimelineWorkerThreadFinished();

    /// Slot to handle what happens after the resource list table is sorted.
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedSnapshot();

private:
    /// Update the snapshot markers on the graph.
    void UpdateSnapshotMarkers();

    /// Add a new allocation graph to the timeline.
    /// \return pointer to a new snapshot graphics object.
    RMVTimelineGraph* AddTimelineGraph();

    /// Add a new snapshot to the timeline.
    /// \param snapshot_point The snapshot point to add.
    /// \return pointer to a new marker graphics object.
    RMVSnapshotMarker* AddSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Remove a snapshot from the timeline.
    /// \param snapshot_point The snapshot point.
    void RemoveSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Rename a snapshot from the timeline.
    /// \param snapshot_index The index of the snapshot to rename.
    void RenameSnapshotByIndex(int snapshot_index);

    /// Update the snapshot table display area. If no snapshots have been created, inform the user as such.
    void UpdateTableDisplay();

    /// Select rows in the snapshots table depending on which snapshot is currently selected.
    void SelectTableRows();

    /// Add the snapshot legends to the required scene.
    void AddSnapshotLegends();

    /// Update the right-click context menu on the timeline scrollbar. By default this is shown but
    /// it should be disabled if the view is fully zoomed out.
    /// \param shown Should the context menu be shown.
    void UpdateTimelineScrollbarContextMenu(bool shown);

    /// Helper function to set the maximum height of the table so it only contains rows with valid data.
    inline void SetMaximumSnapshotTableHeight()
    {
        ui_->snapshot_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->snapshot_table_view_, model_->RowCount()));
    }

    Ui::TimelinePane*              ui_;                       ///< Pointer to the Qt UI design.
    ZoomIconGroupManager*          zoom_icon_manager_;        ///< The object responsible for the zoom icon status.
    MainWindow*                    main_window_;              ///< Reference to the mainwindow (parent).
    rmv::TimelineModel*            model_;                    ///< Container class for the widget models.
    ColoredLegendScene*            snapshot_legends_;         ///< Snapshot legends above the timeline.
    KeyboardZoomShortcutsTimeline* keyboard_zoom_shortcuts_;  ///< Keyboard shortcut handler.
    TimelineColorizer*             colorizer_;                ///< The colorizer used by the 'timeline type' combo box.
    rmv::ThreadController*         thread_controller_;        ///< The thread for processing backend data.
};

#endif  // RMV_VIEWS_TIMELINE_TIMELINE_PANE_H_
