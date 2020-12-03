//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for a widget that shows how memory usage changes per
/// process over time.
//=============================================================================

#include "views/custom_widgets/rmv_timeline_graph.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "qt_common/utils/scaling_manager.h"

#include "models/timeline/timeline_model.h"

// Slightly transparent white background color for the custom tooltip
static const QColor kTooltipBackgroundColor = QColor(255, 255, 255, 230);

static const double kTooltipTextOffset = 3.0;

// The position to place the tooltip to the right of the mouse so that the
// mouse cursor isn't obscured
static const qreal kMouseXOffset = 25.0;

RMVTimelineGraph::RMVTimelineGraph(const RMVTimelineGraphConfig& config)
    : mouse_hovering_(false)
    , tooltip_contents_(nullptr)
    , tooltip_background_(nullptr)
{
    setAcceptHoverEvents(true);

    config_ = config;
}

RMVTimelineGraph::~RMVTimelineGraph()
{
}

QRectF RMVTimelineGraph::boundingRect() const
{
    return QRectF(0, 0, ScaledWidth(), ScaledHeight());
}

void RMVTimelineGraph::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();

    qreal scaled_height = ScaledHeight();
    int   num_buckets   = config_.model_data->GetNumBuckets();

    // for each bucket, ask the model for a rectangle to draw.
    int  bucket_index = 0;
    bool success      = true;
    do
    {
        for (int bucket_group_index = 0; bucket_group_index < config_.model_data->GetNumBucketGroups(); bucket_group_index++)
        {
            QColor color  = config_.colorizer->GetColor(bucket_group_index);
            qreal  y_pos  = 0.0f;
            qreal  height = 0.0f;
            success       = config_.model_data->GetHistogramData(bucket_group_index, bucket_index, y_pos, height);
            if (success)
            {
                QRectF rect;

                // flip the y-coord so (0, 0) is at the bottom left and scale values up to fit the view
                rect.setY(scaled_height - (y_pos * scaled_height));
                rect.setHeight(height * scaled_height);

                // calculate the x offset and width based on the bucket number and number of buckets
                qreal w = (qreal)ScaledWidth() / (qreal)num_buckets;
                rect.setX(w * (qreal)bucket_index);

                // allow for rounding error on width since coords are floating point
                rect.setWidth(w + 0.5);

                painter->fillRect(rect, color);
            }
            else
            {
                break;
            }
        }
        bucket_index++;
    } while (success);

    painter->restore();
}

void RMVTimelineGraph::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    mouse_hovering_       = true;
    last_scene_hover_pos_ = event->scenePos();
    last_hover_pos_       = event->pos();
}

void RMVTimelineGraph::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    last_scene_hover_pos_ = event->scenePos();
    last_hover_pos_       = event->pos();
    UpdateToolTip();
}

void RMVTimelineGraph::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    mouse_hovering_ = false;

    if (tooltip_background_ != nullptr)
    {
        tooltip_background_->hide();
    }

    if (tooltip_contents_ != nullptr)
    {
        tooltip_contents_->hide();
    }
}

void RMVTimelineGraph::UpdateDimensions(int width, int height)
{
    config_.width  = width;
    config_.height = height;
}

int32_t RMVTimelineGraph::ScaledWidth() const
{
    return config_.width;
}

int32_t RMVTimelineGraph::ScaledHeight() const
{
    return config_.height;
}

void RMVTimelineGraph::UpdateToolTip()
{
    if (!tooltip_contents_ || !tooltip_background_)
    {
        tooltip_background_ = scene()->addRect(QRect(), QPen(), kTooltipBackgroundColor);
        tooltip_contents_   = new RMVTimelineTooltip();
        scene()->addItem(tooltip_contents_);
    }

    if (mouse_hovering_)
    {
        QString     str;
        const qreal view_width = ScaledWidth();
        Q_ASSERT(view_width > 0);
        qreal              x_pos = last_hover_pos_.x() / view_width;
        QList<TooltipInfo> tooltip_info;
        if (config_.model_data->GetTimelineTooltipInfo(x_pos, tooltip_info))
        {
            tooltip_background_->show();
            tooltip_contents_->show();

            const qreal view_height    = ScaledHeight();
            const qreal tooltip_width  = tooltip_background_->rect().width();
            const qreal tooltip_height = tooltip_background_->rect().height();

            // If the tooltip is to the right of the mouse, offset it slightly so it isn't
            // hidden under the mouse
            static const qreal mouse_x_offset = kMouseXOffset;
            QPointF            tooltip_offset = QPointF(mouse_x_offset, 0);

            // If the tooltip is going to run off the right of the scene, position it to the left
            // of the mouse
            if (last_hover_pos_.x() > (view_width - mouse_x_offset - tooltip_width))
            {
                tooltip_offset.setX(-tooltip_width);
            }

            // If the tooltip is going to run off the bottom of the scene, position it at the bottom of the scene
            // so all the tooltip is visible
            qreal max_y_pos = view_height - tooltip_height;
            if (last_hover_pos_.y() > max_y_pos)
            {
                tooltip_offset.setY(max_y_pos - last_hover_pos_.y());
            }

            // tooltip labels use scene coordinates
            const QPointF label_pos = last_scene_hover_pos_ + tooltip_offset;
            tooltip_contents_->setPos(label_pos);
            tooltip_contents_->SetData(tooltip_info);

            // Add a small margin so text is not overlapping the outline and adjust position accordingly
            QRectF rect   = tooltip_contents_->boundingRect();
            qreal  offset = 2.0 * kTooltipTextOffset;
            rect.setWidth(rect.width() + offset);
            rect.setHeight(rect.height() + offset);
            tooltip_background_->setPos(label_pos - QPointF(kTooltipTextOffset, kTooltipTextOffset));
            tooltip_background_->setRect(rect);
        }
        else
        {
            tooltip_background_->hide();
            tooltip_contents_->hide();
        }
    }
}
