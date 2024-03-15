//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a resource timeline widget.
//=============================================================================

#include "views/custom_widgets/rmv_resource_timeline.h"

#include <math.h>
#include <QPainter>
#include <QQueue>
#include <QMouseEvent>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "views/delegates/rmv_resource_event_delegate.h"

// Some defaults.
static const int32_t kDefaultTimelineWidthHint  = 100;
static const int32_t kDefaultTimelineHeightHint = RMVResourceEventDelegate::kIconDefaultSizeHint;

RMVResourceTimeline::RMVResourceTimeline(QWidget* parent)
    : QWidget(parent)
    , model_(nullptr)
{
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QWidget::updateGeometry);
}

RMVResourceTimeline::~RMVResourceTimeline()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QWidget::updateGeometry);
}

void RMVResourceTimeline::Initialize(rmv::ResourceDetailsModel* model)
{
    model_ = model;
}

QSize RMVResourceTimeline::sizeHint() const
{
    return ScalingManager::Get().Scaled(QSize(kDefaultTimelineWidthHint, kDefaultTimelineHeightHint));
}

void RMVResourceTimeline::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    const QRect& r     = rect();
    int          mid_y = r.y() + (r.height() / 2);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::black, 1));
    painter.drawLine(r.left(), mid_y, r.right(), mid_y);

    // Draw the resource events. Get the data from the model.
    int                icon_size = r.height() * RMVResourceEventDelegate::kIconSizeFactor;
    int                width     = r.width() - icon_size;
    int                index     = 0;
    rmv::ResourceEvent resource_event;
    bool               success = true;
    do
    {
        success = model_->GetEventData(index, width, resource_event);
        if (success)
        {
            int left_pos = r.left() + resource_event.timestamp;
            event_icons_.DrawIcon(&painter, left_pos, mid_y, icon_size, resource_event.color, resource_event.shape);
        }
        index++;
    } while (success);
}

void RMVResourceTimeline::mousePressEvent(QMouseEvent* event)
{
    double icon_size = this->height() * RMVResourceEventDelegate::kIconSizeFactor;
    int    x_pos     = event->pos().x();
    double width     = this->width() - icon_size;

    double logical_position = (double)x_pos / width;
    double tolerance        = icon_size / width;

    emit TimelineSelected(logical_position, tolerance);
}
