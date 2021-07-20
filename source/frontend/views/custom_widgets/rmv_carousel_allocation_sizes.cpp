//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the carousel allocation sizes widget.
//=============================================================================

#include "views/custom_widgets/rmv_carousel_allocation_sizes.h"

#include <QPainter>

#include "qt_common/utils/scaling_manager.h"

#include "util/rmv_util.h"

static const int kBarTopOffset = 45;
static const int kBarHeight    = 180;

RMVCarouselAllocationSizes::RMVCarouselAllocationSizes(const RMVCarouselConfig& config)
    : RMVCarouselItem(config)
    , data_{}
{
}

RMVCarouselAllocationSizes::~RMVCarouselAllocationSizes()
{
}

void RMVCarouselAllocationSizes::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    DrawCarouselBaseComponents(painter, "Virtual memory allocation size");

    const QString size_strings[] = {"0MiB", "1MiB", "2MiB", "4MiB", "8MiB", "16MiB", "32MiB", "64MiB", "128MiB", "256MiB", "512MiB", "1GiB"};

    int gap       = 5;
    int bar_width = config_.width / (rmv::kNumAllocationSizeBuckets + 3);
    int x_offset  = (config_.width - (((rmv::kNumAllocationSizeBuckets - 1) * gap) + (rmv::kNumAllocationSizeBuckets * bar_width))) / 2;

    for (int index = 0; index < rmv::kNumAllocationSizeBuckets; index++)
    {
        int x_pos = ScalingManager::Get().Scaled((index * (gap + bar_width)) + x_offset);
        DrawAllocationBar(painter, bar_width, x_pos, data_.buckets[index], size_strings[index]);
    }
}

void RMVCarouselAllocationSizes::DrawAllocationBar(QPainter* painter, int bar_width, int x_pos, int32_t value, const QString& label_string)
{
    QColor text_color = Qt::black;
    QColor fill_color = kDefaultCarouselBarColor;

    int   origin    = 0;
    float bar_scale = 1.0;
    bool  negative  = false;

    if (config_.data_type == kCarouselDataTypeDelta)
    {
        if (value > 0)
        {
            fill_color = rmv_util::GetDeltaChangeColor(kDeltaChangeIncrease);
        }
        else if (value < 0)
        {
            fill_color = rmv_util::GetDeltaChangeColor(kDeltaChangeDecrease);
            negative   = true;
        }
        else
        {
            fill_color = rmv_util::GetDeltaChangeColor(kDeltaChangeNone);
        }
        text_color = fill_color;

        value     = abs(value);
        bar_scale = 0.5;
        origin    = ScalingManager::Get().Scaled(kBarHeight / 2);
    }

    // Calculate the positioning of the bars given a spacing between the bars and the width of each bar.
    int bar_top    = ScalingManager::Get().Scaled(kBarTopOffset);
    int bar_bottom = ScalingManager::Get().Scaled(kBarTopOffset + kBarHeight);
    int bar_height = ScalingManager::Get().Scaled(kBarHeight);
    int font_size  = ScalingManager::Get().Scaled(9);
    int bar_length = ScalingManager::Get().Scaled(bar_width);

    // Draw the bar.
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRect(x_pos, bar_top, bar_length, bar_height);

    // Draw the data in the bar if data is valid.
    painter->setBrush(fill_color);
    if (data_.num_allocations > 0)
    {
        float height = (value * bar_height) / data_.num_allocations;
        height *= bar_scale;
        height = std::max<float>(height, 1.0);
        if (!negative)
        {
            origin += height;
        }
        painter->drawRect(x_pos, bar_bottom - origin, bar_length, height);
    }

    // Set up the text drawing.
    QFont font = painter->font();
    font.setBold(false);
    font.setPixelSize(font_size);
    painter->setPen(Qt::black);
    painter->setFont(font);
    QFontMetricsF scaled_font_metrics = ScalingManager::Get().ScaledFontMetrics(font);

    // Draw the text label under the bar.
    qreal text_width    = scaled_font_metrics.width(label_string);
    int   text_x_offset = x_pos - (text_width / 2);
    int   text_y_offset = bar_top + bar_height + font_size;
    painter->drawText(text_x_offset, text_y_offset, label_string);

    // Draw the value string above the bar, centered.
    QString value_string = QString::number(value);
    text_width           = scaled_font_metrics.width(value_string);
    text_x_offset        = x_pos + ((bar_width - text_width) / 2);
    text_y_offset        = bar_top - (font_size / 2);
    painter->setPen(text_color);
    painter->drawText(text_x_offset, text_y_offset, value_string);
}

void RMVCarouselAllocationSizes::SetData(const rmv::RMVCarouselData& data)
{
    data_ = data.allocation_sizes_data;
    update();
}
