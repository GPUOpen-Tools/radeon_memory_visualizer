//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RMV's carousel navigation button
//=============================================================================

#include "views/custom_widgets/rmv_carousel_nav_button.h"

#include <QCursor>
#include <QPainter>

#include "qt_common/utils/scaling_manager.h"

static const QColor kDefaultColor = QColor(210, 210, 210);
static const QColor kHoveredColor = QColor(180, 180, 180);
static const QColor kPressedColor = QColor(150, 150, 150);

RMVCarouselNavButton::RMVCarouselNavButton(int width, int height, bool left_direction)
    : width_(width)
    , height_(height)
    , left_direction_(left_direction)
    , hovered_(false)
    , pressed_(false)
{
    setAcceptHoverEvents(true);
}

RMVCarouselNavButton::~RMVCarouselNavButton()
{
}

QRectF RMVCarouselNavButton::boundingRect() const
{
    return QRectF(0, 0, width_, height_);
}

QPolygonF RMVCarouselNavButton::GetTriangle(int width, int height, bool left_direction, double scaling_factor)
{
    const int triangle_height = 100 * scaling_factor;
    const int center_pos_y    = height / 2;

    QPolygonF triangle;
    if (left_direction)
    {
        triangle << QPoint(width, center_pos_y - triangle_height / 2) << QPoint(width, center_pos_y + triangle_height / 2) << QPoint(0, center_pos_y);
    }
    else
    {
        triangle << QPoint(0, center_pos_y - triangle_height / 2) << QPoint(0, center_pos_y + triangle_height / 2) << QPoint(width, center_pos_y);
    }

    return triangle;
}

QPainterPath RMVCarouselNavButton::shape() const
{
    QPainterPath path;

    path.addPolygon(GetTriangle(width_, height_, left_direction_, ScalingManager::Get().Scaled(1.0)));

    return path;
}

void RMVCarouselNavButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    QColor arrow_color = kDefaultColor;
    if (pressed_)
    {
        arrow_color = kPressedColor;
    }
    else if (hovered_)
    {
        arrow_color = kHoveredColor;
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(arrow_color);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawPolygon(GetTriangle(width_, height_, left_direction_, ScalingManager::Get().Scaled(1.0)));
}

void RMVCarouselNavButton::UpdateDimensions(int width, int height)
{
    width_  = width;
    height_ = height;
}

void RMVCarouselNavButton::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    setCursor(Qt::PointingHandCursor);

    hovered_ = true;

    update();
}

void RMVCarouselNavButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    hovered_ = false;

    update();
}

void RMVCarouselNavButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    pressed_ = true;

    emit PressedButton(left_direction_);

    update();
}

void RMVCarouselNavButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    pressed_ = false;

    update();
}

int32_t RMVCarouselNavButton::ScaledHeight() const
{
    return ScalingManager::Get().Scaled(height_);
}

int32_t RMVCarouselNavButton::ScaledWidth() const
{
    return ScalingManager::Get().Scaled(width_);
}
