//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's snapshot timeline visualization.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TIMELINE_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TIMELINE_H_

#include "qt_common/custom_widgets/timeline_view.h"

#include "rmt_data_set.h"
#include "rmt_data_snapshot.h"

#include "models/timeline/timeline_model.h"
#include "util/definitions.h"
#include "views/custom_widgets/rmv_snapshot_marker.h"
#include "views/custom_widgets/rmv_timeline_graph.h"

/// Holds and controls the entire queue timings visualization.
class RMVSnapshotTimeline : public TimelineView
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit RMVSnapshotTimeline(QWidget* parent);

    /// Destructor.
    virtual ~RMVSnapshotTimeline();

    /// Handle resizing.
    /// \param event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Handle a mouse press event.
    /// \param event The mouse event.
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// Handle a mouse move event.
    /// \param event The move event.
    virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    /// Capture a mouse wheel event. This allows the user to zoom in and out if
    /// the control key is also pressed.
    /// \param event The wheel event.
    virtual void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

    /// Update objects inside the timeline.
    virtual void UpdateContent() Q_DECL_OVERRIDE;

    /// Create a context menu to add a new snapshot.
    /// \param event The context menu event.
    virtual void contextMenuEvent(QContextMenuEvent* event) Q_DECL_OVERRIDE;

    /// Add a new snapshot.
    /// \param snapshot_point The new snapshot point.
    RMVSnapshotMarker* AddSnapshot(RmtSnapshotPoint* snapshot_point);

    /// Select a snapshot.
    /// \param snapshot_point The snapshot point to select.
    void SelectSnapshot(const RmtSnapshotPoint* snapshot_point);

    /// Add a new timeline graph. This is the representation of different processes
    /// and how their memory usage varies over time. Data is displayed as a slabbed
    /// graph.
    /// \param timeline_model Pointer to the timeline model.
    /// \param colorizer Pointer to the colorizer object.
    RMVTimelineGraph* AddTimelineGraph(rmv::TimelineModel* timeline_model, TimelineColorizer* colorizer);

    /// Clear out scene content.
    void Clear();

    /// Clear the snapshot markers.
    void ClearSnapshotMarkers();

    /// Update the ruler time units.
    /// \param time_units The new time unit to use.
    /// \param time_to_clock_ratio The ratio of time units to clock units. Used to convert from time to
    ///  clocks and vice versa.
    void UpdateTimeUnits(TimeUnitType time_unit, double time_to_clock_ratio);

signals:
    /// Signal for when the selected region in the timeline needs updating.
    /// \param duration The duration.
    void UpdateSelectedDuration(uint64_t duration);

    /// Signal for when the clock needs updating when moving the mouse.
    /// \param clock The time in clocks on the timeline the mouse is over.
    void UpdateHoverClock(uint64_t clock);

    /// Signal for when a snapshot is to be generated.
    /// \param time The time, in clocks, where the snapshot is to be generated.
    void GenerateSnapshotAtTime(uint64_t time);

    /// Signal for when the zoom in button needs updating. Typically called wnen
    /// selecting a region on the timeline or using the zoom buttons.
    /// \param zoom If true, zoom in is allowed.
    void UpdateZoomButtonsForZoomIn(bool zoom);

    /// Signal for when the zoom out button needs updating. Typically called wnen
    /// selecting a region on the timeline or using the zoom buttons.
    /// \param zoom If true, zoom out is allowed.
    void UpdateZoomButtonsForZoomOut(bool zoom);

    /// Signal for when the zoom to selection button needs updating. Typically called wnen
    /// selecting a region on the timeline.
    /// \param zoom If true, zoom in is allowed.
    void UpdateZoomButtonsForZoomToSelection(bool selected_region);

private:
    /// Update marker locations. Uses a dynamic cast to decide whether it's using
    /// the correct type.
    void UpdateSnapshotMarkers();
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TIMELINE_H_
