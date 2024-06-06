//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the carousel resource types widget.
//=============================================================================

#include "views/custom_widgets/rmv_carousel_resource_types.h"

#include <QPainter>

#include "qt_common/utils/qt_util.h"

#include "rmt_print.h"

#include "models/colorizer.h"
#include "util/rmv_util.h"
#include "util/string_util.h"

static const int kLegendStartOffset  = 44;
static const int kLegendHeight       = 26;
static const int kEdgeMargin         = 5;
static const int kIconWidth          = 20;
static const int kNumResourcesToShow = 6;
static const int kTextGap            = 5;  // The gap between the text descriptions.

RMVCarouselResourceTypes::RMVCarouselResourceTypes(const RMVCarouselConfig& config)
    : RMVCarouselItem(config)
    , data_{}
{
}

RMVCarouselResourceTypes::~RMVCarouselResourceTypes()
{
}

void RMVCarouselResourceTypes::DrawCarouselMemoryUsageLegend(QPainter*      painter,
                                                             uint32_t       y_offset,
                                                             const QString& resource_name,
                                                             const QColor&  resource_color,
                                                             int32_t        usage_amount)
{
    static const int kFontPixelSize = 12;
    static const int kTextOffset    = (kIconWidth + kFontPixelSize) / 2;

    y_offset               = (int)y_offset;
    uint32_t margin_length = kEdgeMargin;

    QFont font = painter->font();
    font.setBold(false);
    font.setPixelSize(kFontPixelSize);

    painter->setFont(font);
    painter->setPen(Qt::NoPen);
    painter->setBrush(resource_color);
    painter->drawRect(margin_length, y_offset, kIconWidth, kIconWidth);

    const int text_pos_x        = (2 * margin_length) + kIconWidth;
    QString   usage_description = resource_name;
    if (usage_description.at(usage_description.size() - 1) != 's')
    {
        usage_description += 's';
    }

    painter->setPen(Qt::black);
    painter->drawText(text_pos_x, y_offset + kTextOffset, usage_description);

    QString amount = "(";

    if (config_.data_type == kCarouselDataTypeDelta)
    {
        if (usage_amount > 0)
        {
            painter->setPen(rmv_util::GetDeltaChangeColor(kDeltaChangeIncrease));
            amount += "+";
        }
        else if (usage_amount < 0)
        {
            painter->setPen(rmv_util::GetDeltaChangeColor(kDeltaChangeDecrease));
        }
        else
        {
            painter->setPen(rmv_util::GetDeltaChangeColor(kDeltaChangeNone));
        }
    }

    amount += rmv::string_util::LocalizedValue(usage_amount) + ")";

    const int description_length = QtCommon::QtUtils::GetPainterTextWidth(painter, usage_description);

    painter->drawText(text_pos_x + description_length + kTextGap, y_offset + kTextOffset, amount);
}

void RMVCarouselResourceTypes::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    DrawCarouselBaseComponents(painter, "Resource types");

    int bar_offset = (config_.width / 2);
    int bar_length = (config_.width / 2) - kEdgeMargin;
    int y_pos      = 0;

    // Display the most abundant resource types.
    for (int i = 0; i < kNumResourcesToShow; i++)
    {
        y_pos                           = kLegendStartOffset + (i * kLegendHeight);
        RmtResourceUsageType usage_type = data_.usage_map[i].usage_type;

        DrawCarouselMemoryUsageLegend(painter,
                                      y_pos,
                                      RmtGetResourceUsageTypeNameFromResourceUsageType(usage_type),
                                      rmv::Colorizer::GetResourceUsageColor(usage_type),
                                      data_.usage_map[i].usage_amount);
        DrawHorizontalBarComponent(painter, "", bar_offset, y_pos, bar_length, kIconWidth, data_.usage_map[i].usage_amount, data_.usage_maximum, false);
    }

    // Total up all the other resources.
    int32_t other_amount = 0;
    for (int i = kNumResourcesToShow; i < kRmtResourceUsageTypeCount; i++)
    {
        other_amount += data_.usage_map[i].usage_amount;
    }

    // Show the other resources.
    y_pos = kLegendStartOffset + (kNumResourcesToShow * kLegendHeight);
    DrawCarouselMemoryUsageLegend(painter, y_pos, "Other", rmv::Colorizer::GetResourceUsageColor(kRmtResourceUsageTypeFree), other_amount);
    DrawHorizontalBarComponent(painter, "", bar_offset, y_pos, bar_length, kIconWidth, other_amount, data_.usage_maximum, false);
}

void RMVCarouselResourceTypes::SetData(const rmv::RMVCarouselData& data)
{
    data_ = data.resource_types_data;
    update();
}
