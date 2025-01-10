//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a camera snapshot widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAMERA_SNAPSHOT_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAMERA_SNAPSHOT_WIDGET_H_

#include <QGraphicsObject>

#include "util/definitions.h"

/// The camera widget diameter.
static const qreal kCircleDiameter = 300.0;

/// @brief Holds data for rendering of a camera widget.
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

/// @brief Container class for a camera widget which gets rendered when nothing has been compared yet.
class RMVCameraSnapshotWidget : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] config A configuration struct for this object.
    explicit RMVCameraSnapshotWidget(const RMVCameraSnapshotWidgetConfig& config);

    /// Destructor.
    virtual ~RMVCameraSnapshotWidget();

    /// @brief Mouse hover over event.
    ///
    /// @param [in] event the QGraphicsSceneHoverEvent event object.
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse hover leave event.
    ///
    /// @param [in] event the QGraphicsSceneHoverEvent event object.
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse press event.
    ///
    /// @param [in] event the QGraphicsSceneMouseEvent event object.
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's bounding volume for this item.
    ///
    /// @return The item's bounding rectangle.
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's bounding shape for this item.
    ///
    /// @return The item's QPainterPath.
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this item.
    ///
    /// @param [in] painter The painter object to use.
    /// @param [in] item    Provides style options for the item, such as its state, exposed area and its level-of-detail hints.
    /// @param [in] widget  Points to the widget that is being painted on if specified.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Update the widget dimensions.
    ///
    /// @param [in] width  The new width.
    /// @param [in] height The new height.
    void UpdateDimensions(int width, int height);

    /// @brief Update current snapshot name.
    ///
    /// @param [in] name The new name.
    void UpdateName(const QString& name);

    /// @brief Update current base color.
    ///
    /// @param [in] color The new color.
    void UpdateBaseColor(const QColor& color);

signals:
    /// @brief Signal emitted when a snapshot is clicked on.
    void Navigate();

private:
    /// @brief Get scaled height.
    ///
    /// @return The scaled height.
    int32_t ScaledHeight() const;

    /// @brief Get scaled width.
    ///
    /// @return The scaled width.
    int32_t ScaledWidth() const;

    /// @brief Get scaled margin.
    ///
    /// @return The scaled margin.
    int32_t ScaledMargin() const;

    RMVCameraSnapshotWidgetConfig config_;        ///< Description of this widget.
    QColor                        render_color_;  ///< Color to use when drawing the widget.
};
#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAMERA_SNAPSHOT_WIDGET_H_
