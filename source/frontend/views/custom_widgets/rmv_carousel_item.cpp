//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the carousel item widget base class.
//=============================================================================

#include "views/custom_widgets/rmv_carousel_item.h"

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "util/rmv_util.h"
#include "util/string_util.h"

static const int kSummaryTextOffset = 15;

RMVCarouselItem::RMVCarouselItem(const RMVCarouselConfig& config)
    : config_(config)
{
    config_.width  = kCarouselItemWidth;
    config_.height = kCarouselItemHeight;
}

RMVCarouselItem::~RMVCarouselItem()
{
}

QRectF RMVCarouselItem::boundingRect() const
{
    return QRectF(0, 0, ScalingManager::Get().Scaled(config_.width), ScalingManager::Get().Scaled(config_.height));
}

void RMVCarouselItem::UpdateDimensions(int width, int height)
{
    config_.width  = width;
    config_.height = height;
}

void RMVCarouselItem::DrawCarouselBaseComponents(QPainter* painter, const QString& title) const
{
    int widget_width  = config_.width;
    int widget_height = config_.height;

    widget_width  = ScalingManager::Get().Scaled(widget_width);
    widget_height = ScalingManager::Get().Scaled(widget_height);

    painter->setPen(Qt::NoPen);
    painter->setBrush(kCarouselBaseColor);
    painter->drawRect(0, 0, widget_width, widget_height);

    QFont font;
    font.setPixelSize(ScalingManager::Get().Scaled(15));
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(
        ScalingManager::Get().Scaled(10), ScalingManager::Get().Scaled(20), title + ((config_.data_type == kCarouselDataTypeDelta) ? " delta" : ""));
}

void RMVCarouselItem::DrawHorizontalBarComponent(QPainter*      painter,
                                                 const QString& bar_title,
                                                 uint32_t       x_pos,
                                                 uint32_t       y_pos,
                                                 uint32_t       bar_length,
                                                 uint32_t       bar_width,
                                                 int64_t        value,
                                                 int64_t        max,
                                                 bool           show_summary) const
{
    DrawColoredHorizontalBarComponent(painter, bar_title, kDefaultCarouselBarColor, x_pos, y_pos, bar_length, bar_width, value, max, show_summary);
}

void RMVCarouselItem::DrawColoredHorizontalBarComponent(QPainter*      painter,
                                                        const QString& bar_title,
                                                        const QColor&  bar_color,
                                                        uint32_t       x_pos,
                                                        uint32_t       y_pos,
                                                        uint32_t       bar_length,
                                                        uint32_t       bar_width,
                                                        int64_t        value,
                                                        int64_t        max,
                                                        bool           show_summary) const
{
    QColor fill_color = bar_color;
    QColor text_color = Qt::black;

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
        max       = abs(max);
        bar_scale = 0.5;
        origin    = ScalingManager::Get().Scaled(int(bar_length / 2));
    }

    QFont font = painter->font();

    x_pos      = ScalingManager::Get().Scaled((int)x_pos);
    y_pos      = ScalingManager::Get().Scaled((int)y_pos);
    bar_length = ScalingManager::Get().Scaled((int)bar_length);
    bar_width  = ScalingManager::Get().Scaled((int)bar_width);

    if (bar_title.isEmpty() == false)
    {
        int title_font_size = ScalingManager::Get().Scaled(12);
        font.setBold(false);
        font.setPixelSize(title_font_size);
        painter->setPen(Qt::black);
        painter->setFont(font);
        painter->drawText(x_pos, y_pos - title_font_size, bar_title);
    }

    // The length of the bar that contains data.
    uint32_t filled_bar_length = max != 0 ? (value * bar_length) / max : bar_length;
    filled_bar_length          = std::min<uint32_t>(filled_bar_length, bar_length);

    // Make sure bar is within bounds.
    if (filled_bar_length > bar_length)
    {
        filled_bar_length = bar_length;
    }

    // Take into account scale. The bar is scaled by 0.5 in delta mode since values
    // go positive or negative.
    filled_bar_length *= bar_scale;

    // Make sure at least 1 pixel row is drawn.
    if (filled_bar_length < 1.0)
    {
        filled_bar_length = 1.0;
    }

    // Move the origin if the bar is negative.
    if (negative)
    {
        origin -= filled_bar_length;
    }

    // Paint the bar background in white.
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawRect(x_pos, y_pos, bar_length, bar_width);

    // Paint the bar.
    if (max != 0)
    {
        painter->setBrush(fill_color);
        painter->drawRect(x_pos + origin, y_pos, filled_bar_length, bar_width);

        if (show_summary)
        {
            font.setBold(true);
            font.setPixelSize(ScalingManager::Get().Scaled(10));
            painter->setFont(font);
            painter->setPen(text_color);

            QString allocated_description =
                rmv::string_util::LocalizedValueMemory(value, false, false) + " out of " + rmv::string_util::LocalizedValueMemory(max, false, false);
            int allocated_length = QtCommon::QtUtils::GetTextWidth(font, allocated_description);

            painter->drawText(
                x_pos + bar_length - allocated_length, y_pos + bar_width + ScalingManager::Get().Scaled(kSummaryTextOffset), allocated_description);
        }
    }
}
