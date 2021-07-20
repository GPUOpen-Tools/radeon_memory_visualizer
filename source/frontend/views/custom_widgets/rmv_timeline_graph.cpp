//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a widget that shows how memory usage changes per
/// process over time.
//=============================================================================

#include "views/custom_widgets/rmv_timeline_graph.h"

#include <QPainter>

#include "models/timeline/timeline_model.h"

RMVTimelineGraph::RMVTimelineGraph(const RMVTimelineGraphConfig& config)
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

    // For each bucket, ask the model for a rectangle to draw.
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

                // Flip the y-coord so (0, 0) is at the bottom left and scale values up to fit the view.
                rect.setY(scaled_height - (y_pos * scaled_height));
                rect.setHeight(height * scaled_height);

                // Calculate the x offset and width based on the bucket number and number of buckets.
                qreal w = (qreal)ScaledWidth() / (qreal)num_buckets;
                rect.setX(w * (qreal)bucket_index);

                // Allow for rounding error on width since coords are floating point.
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
