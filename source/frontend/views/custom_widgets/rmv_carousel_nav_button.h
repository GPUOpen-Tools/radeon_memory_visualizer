//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the carousel navigation button.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_NAV_BUTTON_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_NAV_BUTTON_H_

#include <QGraphicsObject>

/// @brief Container class for the carousel's L/R nav buttons.
class RMVCarouselNavButton : public QGraphicsObject
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] width          The width of the button.
    /// @param [in] height         The height of the button.
    /// @param [in] left_direction If true, the button points to the left.
    RMVCarouselNavButton(int width, int height, bool left_direction);

    /// @brief Destructor.
    virtual ~RMVCarouselNavButton();

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
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) Q_DECL_OVERRIDE;

    /// @brief Mouse hover over event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse hover leave event.
    ///
    /// @param [in] event The QGraphicsSceneHoverEvent event object.
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse press event.
    ///
    /// @param [in] event The QGraphicsSceneMouseEvent event object.
    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Mouse release event.
    ///
    /// @param [in] event The QGraphicsSceneMouseEvent event object.
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    /// @brief Set dimensions of this widget.
    ///
    /// @param [in] width  The width.
    /// @param [in] height The height.
    void UpdateDimensions(int width, int height);

signals:
    /// @brief Signal fired when a button is pressed.
    ///
    /// @param [in] left_direction Was the left button pressed.
    void PressedButton(bool left_direction);

private:
    /// @brief Build a polygon that represents a triangle.
    ///
    /// @param [in] width           The width.
    /// @param [in] height          The height.
    /// @param [in] left_direction  If true, create a left-direction arrow, otherwise right-direction.
    /// @param [in] scaling_factor  The scaling factor.
    ///
    /// @return The triangle as a QPolygonF type.
    static QPolygonF GetTriangle(int width, int height, bool left_direction, double scaling_factor);

    /// @brief Get scaled height.
    ///
    /// @return The scaled height.
    int32_t ScaledHeight() const;

    /// @brief Get scaled width.
    ///
    /// @return The scaled width.
    int32_t ScaledWidth() const;

    int  width_;           ///< Widget width.
    int  height_;          ///< Widget height.
    bool left_direction_;  ///< Whether it's a left arrow or not.
    bool hovered_;         ///< Is the mouse currently hovered over this widget.
    bool pressed_;         ///< Has the mouse clicked this widget.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CAROUSEL_NAV_BUTTON_H_
