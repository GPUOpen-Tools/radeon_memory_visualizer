//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a camera snapshot widget.
//=============================================================================

#include "views/custom_widgets/rmv_camera_snapshot_widget.h"

#include <QPainter>

#include "qt_common/utils/qt_util.h"

#include "managers/message_manager.h"
#include "managers/trace_manager.h"

RMVCameraSnapshotWidget::RMVCameraSnapshotWidget(const RMVCameraSnapshotWidgetConfig& config)
    : config_(config)
    , render_color_(config_.base_color)
{
    setAcceptHoverEvents(true);
}

RMVCameraSnapshotWidget::~RMVCameraSnapshotWidget()
{
}

QRectF RMVCameraSnapshotWidget::boundingRect() const
{
    return QRectF(ScaledMargin(), ScaledMargin(), ScaledWidth(), ScaledHeight());
}

QPainterPath RMVCameraSnapshotWidget::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void RMVCameraSnapshotWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(render_color_);
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);

    painter->setRenderHint(QPainter::Antialiasing);

    const int camera_width  = 120;
    const int camera_height = 80;

    const int circle_diameter = kCircleDiameter - ScaledMargin() * 2;

    painter->drawEllipse(ScaledMargin(), ScaledMargin(), ScaledWidth(), ScaledHeight());
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter->setBrush(Qt::white);
    painter->drawRoundedRect(
        QRect(ScaledMargin() + circle_diameter / 2 - camera_width / 2, ScaledMargin() + circle_diameter / 2 - camera_height / 2, camera_width, camera_height),
        10,
        10);

    painter->setBrush(Qt::white);
    painter->drawRect(ScaledMargin() + circle_diameter / 2 - camera_width / 2 + 15,
                      ScaledMargin() + circle_diameter / 2 - camera_height / 2 - 5,
                      20,
                      20);

    painter->setBrush(render_color_);
    painter->drawRect(ScaledMargin() + circle_diameter / 2 + 30,
                      ScaledMargin() + circle_diameter / 2 - 30,
                      20,
                      10);

    const int lens_diameter_0 = 50;
    painter->setBrush(render_color_);
    painter->drawEllipse(ScaledMargin() + circle_diameter / 2 - lens_diameter_0 / 2,
                         ScaledMargin() + circle_diameter / 2 - lens_diameter_0 / 2 + 3,
                         lens_diameter_0,
                         lens_diameter_0);

    const int lens_diameter_1 = 40;
    painter->setBrush(Qt::white);
    painter->drawEllipse(ScaledMargin() + circle_diameter / 2 - lens_diameter_1 / 2,
                         ScaledMargin() + circle_diameter / 2 - lens_diameter_1 / 2 + 3,
                         lens_diameter_1,
                         lens_diameter_1);

    const int lens_diameter_2 = 30;
    painter->setBrush(render_color_);
    painter->drawEllipse(ScaledMargin() + circle_diameter / 2 - lens_diameter_2 / 2,
                         ScaledMargin() + circle_diameter / 2 - lens_diameter_2 / 2 + 3,
                         lens_diameter_2,
                         lens_diameter_2);

    painter->setPen(Qt::white);
    const int snapshot_name_length = QtCommon::QtUtils::GetPainterTextWidth(painter, config_.snapshot_name);
    painter->drawText(
        ScaledMargin() + circle_diameter / 2 - snapshot_name_length / 2, ScaledMargin() + circle_diameter / 2 + circle_diameter / 4, config_.snapshot_name);
}

void RMVCameraSnapshotWidget::UpdateDimensions(int width, int height)
{
    config_.width  = width - 2;
    config_.height = height - 2;
}

void RMVCameraSnapshotWidget::UpdateName(const QString& name)
{
    config_.snapshot_name = name;

    update();
}

void RMVCameraSnapshotWidget::UpdateBaseColor(const QColor& color)
{
    config_.base_color = color;

    render_color_ = color;

    update();
}

void RMVCameraSnapshotWidget::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    if (config_.interactive)
    {
        setCursor(Qt::PointingHandCursor);

        render_color_ = config_.base_color.darker(125);
    }

    update();
}

void RMVCameraSnapshotWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);

    if (config_.interactive)
    {
        render_color_ = config_.base_color;
    }

    update();
}

void RMVCameraSnapshotWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    if (config_.interactive)
    {
        if (rmv::TraceManager::Get().DataSetValid())
        {
            emit Navigate();
        }
    }
}

int32_t RMVCameraSnapshotWidget::ScaledHeight() const
{
    return config_.height;
}

int32_t RMVCameraSnapshotWidget::ScaledWidth() const
{
    return config_.width;
}

int32_t RMVCameraSnapshotWidget::ScaledMargin() const
{
    return config_.margin;
}