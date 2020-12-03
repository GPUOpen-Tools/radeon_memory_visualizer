//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RMV's carousel navigation button
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_NAV_BUTTON_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_NAV_BUTTON_H_

#include <QGraphicsObject>

/// Container class for the carousel's L/R nav buttons.
class RMVCarouselNavButton : public QGraphicsObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param width The width of the button.
    /// \param height The height of the button.
    /// \param left_direction If true, the button points to the left.
    RMVCarouselNavButton(int width, int height, bool leftDirection);

    /// Destructor.
    virtual ~RMVCarouselNavButton();

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
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// Mouse hover over event.
    /// \param event the QGraphicsSceneHoverEvent.
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse hover leave event.
    /// \param event the QGraphicsSceneHoverEvent.
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// Mouse press event.
    /// \param event the QGraphicsSceneMouseEvent.
    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Mouse release event.
    /// \param event the QGraphicsSceneMouseEvent.
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// Set dimensions of this widget.
    /// \param width The width.
    /// \param height The height.
    void UpdateDimensions(int width, int height);

signals:
    /// Signal fired when a button is pressed.
    /// \param left_direction Was the left button pressed.
    void PressedButton(bool left_direction);

private:
    /// Build a polygon that represents a triangle.
    /// \return The triangle as a QPolygonF type.
    static QPolygonF GetTriangle(int width, int height, bool left_direction, double scaling_factor);

    /// Get scaled height.
    /// \return The scaled height.
    int32_t ScaledHeight() const;

    /// Get scaled width.
    /// \return The scaled width.
    int32_t ScaledWidth() const;

    int  width_;           ///< Widget width.
    int  height_;          ///< Widget height.
    bool left_direction_;  ///< Whether it's a left arrow or not.
    bool hovered_;         ///< Is the mouse currently hovered over this widget.
    bool pressed_;         ///< Has the mouse clicked this widget.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_NAV_BUTTON_H_
