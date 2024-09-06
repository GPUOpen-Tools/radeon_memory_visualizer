//=============================================================================
// Copyright (c) 2017-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the snapshot timeline visualization.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TIMELINE_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TIMELINE_H_

#include "qt_common/custom_widgets/timeline_view.h"

#include "models/timeline/timeline_model.h"
#include "util/definitions.h"
#include "views/custom_widgets/rmv_snapshot_marker.h"
#include "views/custom_widgets/rmv_timeline_graph.h"
#include "views/custom_widgets/rmv_tooltip.h"

/// @brief Holds and controls the entire queue timings visualization.
class RMVSnapshotTimeline : public TimelineView
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVSnapshotTimeline(QWidget* parent);

    /// @brief Destructor.
    virtual ~RMVSnapshotTimeline();

    /// @brief Handle resizing.
    ///
    /// @param [in] event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Handle a mouse press event.
    ///
    /// @param [in] event The mouse event.
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Handle a mouse move event.
    ///
    /// @param [in] event The move event.
    virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Event triggered when the mouse is no longer over the view.
    ///
    /// @param [in] event Pointer to the event object.
    virtual void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;

    /// @brief Capture a mouse wheel event.
    ///
    /// This allows the user to zoom in and out if the control key is also pressed.
    ///
    /// @param [in] event The wheel event.
    virtual void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

    /// @brief Update objects inside the timeline.
    virtual void UpdateContent() Q_DECL_OVERRIDE;

    /// @brief Create a context menu to add a new snapshot.
    ///
    /// @param [in] event The context menu event.
    virtual void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;

    /// @brief Add a new snapshot.
    ///
    /// @param [in] snapshot_point The new snapshot point.
    ///
    /// @return The new snapshot marker created.
    RMVSnapshotMarker* AddSnapshot(RmtSnapshotPoint* snapshot_point);

    /// @brief Select a snapshot.
    ///
    /// @param [in] snapshot_point The snapshot point to select.
    void SelectSnapshot(const RmtSnapshotPoint* snapshot_point);

    /// @brief Add a new timeline graph.
    ///
    /// This is the representation of different processes and how their memory usage varies over time.
    /// Data is displayed as a slabbed graph.
    ///
    /// @param [in] timeline_model Pointer to the timeline model.
    /// @param [in] colorizer      Pointer to the colorizer object.
    ///
    /// @return The new timeline graph created.
    RMVTimelineGraph* AddTimelineGraph(rmv::TimelineModel* timeline_model, rmv::TimelineColorizer* colorizer);

    /// @brief Clear out scene content.
    void Clear();

    /// @brief Clear the snapshot markers.
    void ClearSnapshotMarkers();

    /// @brief Update the ruler time units.
    ///
    /// @param [in] time_unit           The new time unit to use.
    /// @param [in] time_to_clock_ratio The ratio of time units to clock units. Used to convert from time to
    ///  clocks and vice versa.
    void UpdateTimeUnits(TimeUnitType time_unit, double time_to_clock_ratio);

signals:
    /// @brief Signal for when the selected region in the timeline needs updating.
    ///
    /// @param [in] duration The duration.
    void UpdateSelectedDuration(uint64_t duration);

    /// @brief Signal for when the clock needs updating when moving the mouse.
    ///
    /// @param [in] clock The time in clocks on the timeline the mouse is over.
    void UpdateHoverClock(uint64_t clock);

    /// @brief Signal for when a snapshot is to be generated.
    ///
    /// @param [in] time The time, in clocks, where the snapshot is to be generated.
    void GenerateSnapshotAtTime(uint64_t time);

    /// @brief Signal for when the zoom in button needs updating.
    ///
    /// Typically called wnen selecting a region on the timeline or using the zoom buttons.
    ///
    /// @param [in] zoom If true, zoom in is allowed.
    void UpdateZoomButtonsForZoomIn(bool zoom);

    /// @brief Signal for when the zoom out button needs updating.
    ///
    /// Typically called wnen selecting a region on the timeline or using the zoom buttons.
    ///
    /// @param [in] zoom If true, zoom out is allowed.
    void UpdateZoomButtonsForZoomOut(bool zoom);

    /// @brief Signal for when the zoom to selection button needs updating.
    ///
    /// Typically called wnen selecting a region on the timeline.
    ///
    /// @param [in] zoom If true, zoom in is allowed.
    void UpdateZoomButtonsForZoomToSelection(bool selected_region);

private:
    /// @brief Update marker locations.
    ///
    /// Uses a dynamic cast to decide whether it's using the correct type.
    void UpdateSnapshotMarkers();

    /// @brief Update the tool tip.
    ///
    /// Make sure the tool tip contains the correct data for what is currently under the mouse position.
    ///
    /// @param [in] mouse_pos The mouse position in the parent view.
    void UpdateToolTip(const QPointF& mouse_pos);

    /// @brief Update the widget based on the color theme.
    void OnColorThemeUpdated();

    RMVTimelineGraph*   timeline_graph_;    ///< The timeline graph.
    RMVTooltip          timeline_tooltip_;  ///< The tooltip on the timeline.
    rmv::TimelineModel* timeline_model_;    ///< Pointer to the timeline model.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TIMELINE_H_
