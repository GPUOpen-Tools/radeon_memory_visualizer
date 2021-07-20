//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a snapshot marker.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_MARKER_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_MARKER_H_

#include <QGraphicsObject>

#include "rmt_data_set.h"

#include "util/rmv_util.h"

/// @brief Describes the little triangle+line indicating a snapshot on the timeline.
struct RMVSnapshotMarkerConfig
{
    int               width;           ///< Widget width.
    int               height;          ///< Widget height.
    RmtSnapshotPoint* snapshot_point;  ///< Snapshot point.
};

/// @brief Container class for a widget which shows when a snapshot lives on a timeline.
class RMVSnapshotMarker : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVSnapshotMarker(const RMVSnapshotMarkerConfig& config);

    /// @brief Destructor.
    virtual ~RMVSnapshotMarker();

    /// @brief Build a polygon that represents a triangle.
    ///
    /// @param [in] length The triangle line length, in pixels.
    ///
    /// @return The triangle polygon.
    static QPolygonF GetTriangle(int length);

    /// @brief Mouse hover over event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse hover leave event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse press event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse double click event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] item    Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Update the widget dimensions.
    ///
    /// @param [in] width  The width.
    /// @param [in] height The height.
    void UpdateDimensions(int width, int height);

    /// @brief Set the selected state of this snapshot marker.
    ///
    /// @param selected Whether this snapshot is selected or not.
    void SetSelected(bool selected);

    /// @brief Set the state of this snapshot marker.
    ///
    /// @param [in] state The state of this snapshot marker.
    void SetState(SnapshotState state);

    /// @brief Get the snapshot point for this snapshot marker.
    ///
    /// @return The snapshot point.
    RmtSnapshotPoint* GetSnapshotPoint() const;

    /// @brief Get the state of this snapshot marker.
    ///
    /// @return The state.
    SnapshotState GetState() const;

private:
    RMVSnapshotMarkerConfig config_;    ///< Description of this widget.
    SnapshotState           state_;     ///< Set the state of the snapshot.
    bool                    selected_;  ///< Is this snapshot marker selected.
    bool                    hovered_;   ///< Is this snapshot marker hovered over?
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_MARKER_H_
