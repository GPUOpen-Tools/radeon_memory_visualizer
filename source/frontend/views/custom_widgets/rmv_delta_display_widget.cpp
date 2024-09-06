//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a delta display widget.
//=============================================================================

#include "views/custom_widgets/rmv_delta_display_widget.h"

#include <QPainter>
#include <QWidget>

#include "qt_common/utils/qt_util.h"

#include "util/rmv_util.h"
#include "util/string_util.h"

RMVDeltaDisplayWidget::RMVDeltaDisplayWidget(const RMVDeltaDisplayWidgetConfig& config)
    : config_(config)
{
}

RMVDeltaDisplayWidget::~RMVDeltaDisplayWidget()
{
}

QRectF RMVDeltaDisplayWidget::boundingRect() const
{
    return QRectF(0, 0, config_.width, config_.height);
}

void RMVDeltaDisplayWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    painter->setFont(config_.font);

    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(Qt::NoPen);

    int x_pos = 5;

    if (config_.graphic == true)
    {
        if (config_.type == kDeltaValueTypeString)
        {
            painter->setBrush(config_.custom_color);
            painter->drawEllipse(QRect(0, 0, 20, 20));
        }
        else
        {
            if (config_.value_num > 0)
            {
                painter->setBrush(rmv_util::GetDeltaChangeColor(kDeltaChangeIncrease));

                QPolygonF polygon;
                polygon << QPoint(0, 20) << QPoint(10, 0)
                        << QPoint(20, 20);
                painter->drawPolygon(polygon);
            }
            else if (config_.value_num < 0)
            {
                painter->setBrush(rmv_util::GetDeltaChangeColor(kDeltaChangeDecrease));

                QPolygonF polygon;
                polygon << QPoint(0, 0) << QPoint(10, 20)
                        << QPoint(20, 0);
                painter->drawPolygon(polygon);
            }
            else
            {
                painter->setBrush(rmv_util::GetDeltaChangeColor(kDeltaChangeNone));
                painter->drawEllipse(QRect(0, 0, 20, 20));
            }
        }

        x_pos += config_.height;
    }

    QString text = "N/A";

    if (config_.type == kDeltaValueTypeString)
    {
        text = config_.value_string;
    }
    else if (config_.type == kDeltaValueTypeValue)
    {
        text = rmv::string_util::LocalizedValue(config_.value_num);
    }
    else if (config_.type == kDeltaValueTypeValueLabeled)
    {
        text = rmv::string_util::LocalizedValueMemory(config_.value_num, false, false);
    }

    painter->setPen(QtCommon::QtUtils::ColorTheme::Get().GetCurrentThemeColors().graphics_scene_text_color);
    painter->drawText(x_pos, 15, text);
}

void RMVDeltaDisplayWidget::UpdateDimensions(int width, int height)
{
    config_.width  = width;
    config_.height = height;

    update();
}

void RMVDeltaDisplayWidget::UpdateDataType(DeltaValueType type)
{
    config_.type = type;

    update();
}

void RMVDeltaDisplayWidget::UpdateDataValueNum(int64_t value)
{
    config_.value_num = value;

    update();
}

void RMVDeltaDisplayWidget::UpdateDataValueString(const QString& string)
{
    config_.value_string = string;

    update();
}

void RMVDeltaDisplayWidget::UpdateDataCustomColor(const QColor& color)
{
    config_.custom_color = color;

    update();
}

void RMVDeltaDisplayWidget::UpdateDataGraphic(bool graphic)
{
    config_.graphic = graphic;

    update();
}
