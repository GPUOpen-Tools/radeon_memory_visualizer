//=============================================================================
// Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a color picker button.
//=============================================================================

#include "views/custom_widgets/rmv_color_picker_button.h"

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>

#include "qt_common/utils/scaling_manager.h"

static const int kDefaultButtonDimension = 60;

RMVColorPickerButton::RMVColorPickerButton(QWidget* parent)
    : QPushButton(parent)
{
    connect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QPushButton::updateGeometry);
}

RMVColorPickerButton::~RMVColorPickerButton()
{
    disconnect(&ScalingManager::Get(), &ScalingManager::ScaleFactorChanged, this, &QPushButton::updateGeometry);
}

void RMVColorPickerButton::SetColor(const QColor& color)
{
    button_color_ = color;
}

int RMVColorPickerButton::heightForWidth(int width) const
{
    return width;
}

QSize RMVColorPickerButton::sizeHint() const
{
    return ScalingManager::Get().Scaled(minimumSizeHint());
}

QSize RMVColorPickerButton::minimumSizeHint() const
{
    return QSize(kDefaultButtonDimension, kDefaultButtonDimension);
}

void RMVColorPickerButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    ScalingManager& sm = ScalingManager::Get();
    QPainter        painter(this);

    const int pos_adj       = sm.Scaled(1);
    const int size_adj      = pos_adj * 2;
    const int outline_width = sm.Scaled(2);

    // Rectangle used for drawing button and its border.
    QRect r1(pos_adj, pos_adj, this->size().width() - size_adj, this->size().height() - size_adj);

    const int left     = r1.left() + outline_width;
    const int right    = left + r1.width() - (outline_width * 2);
    const int top      = r1.top() + outline_width;
    const int bottom   = top + r1.height() - (outline_width * 2);
    const int center_x = (left + right) / 2;
    const int center_y = (top + bottom) / 2;

    if (this->isChecked() || this->underMouse())
    {
        // Fill rect with black to form border.
        painter.fillRect(r1, QBrush(Qt::black));

        QPolygon polygon;
        QPolygon polygon2;

        // Determine polygon shapes based on button status.
        if (this->isChecked())
        {
            polygon << QPoint(center_x, top) << QPoint(right, top) << QPoint(right, bottom) << QPoint(left, bottom) << QPoint(left, center_y);

            polygon2 << QPoint(center_x, top) << QPoint(right - 1, top) << QPoint(right - 1, bottom - 1) << QPoint(left, bottom - 1) << QPoint(left, center_y);
        }
        else if (this->underMouse())
        {
            polygon << QPoint(left, top) << QPoint(right, top) << QPoint(right, bottom) << QPoint(left, bottom);

            polygon2 << QPoint(left, top) << QPoint(right - 1, top) << QPoint(right - 1, bottom - 1) << QPoint(left, bottom - 1);
        }

        // Draw colored polygon.
        QPainterPath path;
        path.addPolygon(polygon);
        painter.fillPath(path, QBrush(button_color_));

        // Draw white interior border.
        painter.setPen(QPen(Qt::white, 1));
        painter.drawPolygon(polygon2);
    }
    else
    {
        // Fill rect with black to form border.
        painter.fillRect(r1, button_color_);
    }
}
