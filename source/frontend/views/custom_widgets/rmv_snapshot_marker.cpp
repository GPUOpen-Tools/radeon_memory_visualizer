//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a snapshot marker
//=============================================================================

#include "views/custom_widgets/rmv_snapshot_marker.h"

#include <QPainter>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "models/message_manager.h"
#include "util/rmv_util.h"

RMVSnapshotMarker::RMVSnapshotMarker(const RMVSnapshotMarkerConfig& config)
    : config_(config)
    , state_(kSnapshotStateNone)
    , selected_(false)
    , hovered_(false)
{
    setAcceptHoverEvents(true);
}

RMVSnapshotMarker::~RMVSnapshotMarker()
{
}

QPolygonF RMVSnapshotMarker::GetTriangle(int length)
{
    QPolygonF triangle;
    triangle << QPoint(length / 2 * -1, 0) << QPoint(length / 2, 0) << QPoint(0, length);
    return triangle;
}

QRectF RMVSnapshotMarker::boundingRect() const
{
    return QRectF(static_cast<qreal>(config_.width) / 2.0 * -1.0, 0.0, config_.width, config_.height);
}

void RMVSnapshotMarker::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    QColor base_color = rmv_util::GetSnapshotStateColor(state_);

    if (state_ == kSnapshotStateNone)
    {
        //base_color = rmv_util::GetSnapshotTypeColor(config_.data.type);
    }

    const bool darken = (selected_) || ((hovered_) && (!selected_));

    if ((darken) && (state_ == kSnapshotStateNone))
    {
        base_color = base_color.darker(125);
    }

    painter->setBrush(base_color);

    if (darken)
    {
        QPen pen;
        pen.setWidth(ScalingManager::Get().Scaled(2));
        pen.setBrush(Qt::black);
        painter->setPen(pen);
        setZValue(0.99);
    }
    else
    {
        painter->setPen(Qt::NoPen);
        setZValue(0.98);
    }

    painter->setRenderHint(QPainter::Antialiasing);

    QPolygonF polygon = GetTriangle(config_.width);
    painter->drawPolygon(polygon);

    QFont font;
    font.setPixelSize(ScalingManager::Get().Scaled(10));
    painter->setFont(font);

    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setWidth(1);
    if (darken)
    {
        pen.setColor(Qt::black);
    }
    else
    {
        pen.setColor(base_color);
    }

    painter->setPen(pen);
    painter->drawLine(0, config_.width, 0, config_.height);

    pen.setStyle(Qt::SolidLine);
    pen.setBrush(rmv_util::GetTextColorForBackground(base_color));
    painter->setPen(pen);

    painter->setRenderHint(QPainter::TextAntialiasing);

    //painter->drawText(QPoint(QtCommon::QtUtil::GetPainterTextWidth(painter, QString::number(config_.data.id)) / 2 * -1, config_.width / 2 - 1),
    //                   QString::number(config_.data.id));
}

void RMVSnapshotMarker::UpdateDimensions(int width, int height)
{
    config_.width  = width - 2;
    config_.height = height - 2;
}

void RMVSnapshotMarker::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    hovered_ = true;

    setCursor(Qt::PointingHandCursor);

    update();
}

void RMVSnapshotMarker::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    hovered_ = false;

    update();
}

void RMVSnapshotMarker::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    hovered_ = false;

    // get the snapshot id
    if (config_.snapshot_point != nullptr)
    {
        emit MessageManager::Get().SelectSnapshot(config_.snapshot_point);
    }
}

void RMVSnapshotMarker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    hovered_ = false;

    if (config_.snapshot_point != nullptr)
    {
        emit MessageManager::Get().OpenSnapshot(config_.snapshot_point);
    }
}

void RMVSnapshotMarker::SetSelected(bool selected)
{
    selected_ = selected;
}

void RMVSnapshotMarker::SetState(SnapshotState state)
{
    state_ = state;
}

RmtSnapshotPoint* RMVSnapshotMarker::GetSnapshotPoint()
{
    return config_.snapshot_point;
}

SnapshotState RMVSnapshotMarker::GetState()
{
    return state_;
}
