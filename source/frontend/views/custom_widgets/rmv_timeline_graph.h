//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a widget that shows how memory usage changes per
/// process over time.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_GRAPH_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_GRAPH_H_

#include <QGraphicsObject>

#include "models/timeline/timeline_model.h"
#include "util/definitions.h"
#include "views/custom_widgets/rmv_timeline_tooltip.h"
#include "views/timeline_colorizer.h"

/// Describes the data needed for the timeline.
struct RMVTimelineGraphConfig
{
    int                 width;       ///< Widget width.
    int                 height;      ///< Widget height.
    rmv::TimelineModel* model_data;  ///< Pointer to the timeline model.
    TimelineColorizer*  colorizer;   ///< Pointer to the timeline colorizer.
};

/// Container class for a widget which shows how memory allocations change per process
/// over time.
class RMVTimelineGraph : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config The configuration for this widget.
    explicit RMVTimelineGraph(const RMVTimelineGraphConfig& config);

    /// Destructor.
    virtual ~RMVTimelineGraph();

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const override;

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param option Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    /// Overridden hoverEnterEvent to handle what happens when the user moves the
    /// mouse into this graphics object.
    /// \param event The data associated with this event.
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    /// Overridden hoverMoveEvent to handle what happens when the user moves the
    /// mouse in this graphics object. Shows a tooltip of information about the current
    /// timeline data.
    /// \param event The data associated with this event.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    /// Overridden hoverLeaveEvent to handle what happens when the user moves the
    /// mouse off this graphics object.
    /// \param event The data associated with this event.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    /// Update the dimensions of this widget. This widget is such that it is the same
    /// size as the view and placed in the scene where it is always visible.
    /// \param width The new width.
    /// \param height The new height.
    void UpdateDimensions(int width, int height);

private slots:
    /// Update the tool tip.
    void UpdateToolTip();

private:
    /// Get scaled width.
    /// \return scaled width.
    int32_t ScaledWidth() const;

    /// Get scaled height.
    /// \return scaled height.
    int32_t ScaledHeight() const;

    RMVTimelineGraphConfig config_;                ///< Description of this widget.
    bool                   mouse_hovering_;        ///< Tracks if mouse is hovering over the widget.
    QPointF                last_scene_hover_pos_;  ///< Keeps track of the last mouse scene hover position.
    QPointF                last_hover_pos_;        ///< Keeps track of the last mouse hover position (view coords).
    RMVTimelineTooltip*    tooltip_contents_;      ///< Contents of the custom tool tip implementation.
    QGraphicsRectItem*     tooltip_background_;    ///< Background rect of the custom tool tip implementation.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TIMELINE_GRAPH_H_
