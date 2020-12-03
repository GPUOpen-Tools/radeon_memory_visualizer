//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a memory block widget
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAMERA_SNAPSHOT_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAMERA_SNAPSHOT_WIDGET_H_

#include <QGraphicsObject>

#include "util/definitions.h"

/// The camera widget diameter.
static const qreal kCircleDiameter = 300.0;

/// Holds data for rendering of a camera widget.
struct RMVCameraSnapshotWidgetConfig
{
    RMVCameraSnapshotWidgetConfig()
        : width(0)
        , height(0)
        , margin(0)
        , interactive(false)
    {
    }

    int     width;          ///< Widget width.
    int     height;         ///< Widget height.
    int     margin;         ///< How far from the edge.
    QColor  base_color;     ///< Background color.
    QString snapshot_name;  ///< Name of the snapshot.
    bool    interactive;    ///< Can be clicked.
};

/// Container class for a camera widget which gets rendered when nothing has been compared yet.
class RMVCameraSnapshotWidget : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param config A configuration struct for this object.
    explicit RMVCameraSnapshotWidget(const RMVCameraSnapshotWidgetConfig& config);

    /// Destructor.
    virtual ~RMVCameraSnapshotWidget();

    /// Mouse hover over event.
    /// \param event the QGraphicsSceneHoverEvent.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse hover leave event.
    /// \param event the QGraphicsSceneHoverEvent.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse press event.
    /// \param event the QGraphicsSceneHoverEvent.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Implementation of Qt's bounding volume for this item.
    /// \return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// Implementation of Qt's bounding shape for this item.
    /// \return The item's QPainterPath.
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

    /// Implementation of Qt's paint for this item.
    /// \param painter The painter object to use.
    /// \param item Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// \param widget Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// Set max width.
    /// \param width the width.
    void UpdateDimensions(int width, int height);

    /// Update current snapshot name.
    /// \param name new name.
    void UpdateName(const QString& name);

    /// Update current base color.
    /// \param color new color.
    void UpdateBaseColor(const QColor& color);

signals:
    /// Signal emitted when a snapshot is clicked on.
    void Navigate();

private:
    /// Get scaled height.
    /// \return scaled height.
    int32_t ScaledHeight() const;

    /// Get scaled width.
    /// \return scaled width.
    int32_t ScaledWidth() const;

    /// Get scaled margin.
    /// \return scaled margin.
    int32_t ScaledMargin() const;

    RMVCameraSnapshotWidgetConfig config_;        ///< Description of this widget.
    QColor                        render_color_;  ///< Color to use when drawing the widget.
};
#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAMERA_SNAPSHOT_WIDGET_H_
