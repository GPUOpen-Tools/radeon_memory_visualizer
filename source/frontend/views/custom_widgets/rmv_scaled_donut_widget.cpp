//=============================================================================
// Copyright (c) 2017-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of a donut widget.
//=============================================================================

// use definition of PI from math.h
#define _USE_MATH_DEFINES

#include "views/custom_widgets/rmv_scaled_donut_widget.h"

#include <math.h>
#include <QStylePainter>
#include <QQueue>

#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/scaling_manager.h"

#include "rmt_assert.h"

// The arc width is the ratio of the arc width in pixels to the width of the
// donut widget. The larger this number, the thicker the arc.
static const qreal kArcWidthScale = 0.0921;

// Font size scalings, based on the widget width. The larger the number, the
// smaller the font.
static const int kDonutValuePixelFontSize = 36;
static const int kDonutTextPixelFontSize  = 14;

RMVScaledDonutWidget::RMVScaledDonutWidget(QWidget* parent)
    : QWidget(parent)
    , width_(0)
    , height_(0)
    , num_segments_(0)
    , arc_width_(0)
{
    AdjustSize();
}

RMVScaledDonutWidget::~RMVScaledDonutWidget()
{
}

int RMVScaledDonutWidget::heightForWidth(int width) const
{
    return width;
}

void RMVScaledDonutWidget::AdjustSize()
{
    width_ = geometry().width();
    setMaximumHeight(width_);  // Force proportional aspect ratio.
    height_    = geometry().height();
    arc_width_ = width_ * kArcWidthScale;

    // Scale font sizes in multiples of 2, incase the odd font size doesn't exist.
    // Note: Font point size is adjusted based on the width of widget and the base font point size (before DPI scaling is applied).
    // The size of the widget should be increased based on the DPI scaling.
    //qreal dpiScale   = ScalingManager::Get().GetScaleFactor();
}

void RMVScaledDonutWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF bound_rect = QRectF(0, 0, width_, height_);

    int w = bound_rect.width();
    int h = bound_rect.height();

    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate the total range of the values. This is used to calculate how wide
    // each segment should be.
    qreal range = 0.0;

    for (unsigned int loop = 0; loop < num_segments_; loop++)
    {
        range += slices_[loop].value;
    }

    // Calculate the draw rectangle. Take into account the width of the pen and subtract this
    // from the rectangle bounds.
    QRectF rect(arc_width_ / 2.0, arc_width_ / 2.0, w - arc_width_, h - arc_width_);

    QFont font;
    font.setFamily(font.defaultFamily());
    // Iterate over all segments and draw each one.
    // Set start to 6 o'clock position, clockwise (default is 3 o'clock, so add 90 degrees counterclockwise).
    // Angles are specified in 1/16 of a degree, negative angles are counterclockwise.
    int start_pos = -90 * 16;

    QQueue<QPoint> labelPositions;
    const uint32_t segment_count = RMT_MINIMUM((uint32_t)slices_.count(), num_segments_);
    for (uint32_t loop = 0; loop < segment_count; loop++)
    {
        // Create the pen and set up the color for this slice.
        QPen pen(slices_[loop].fill_color, arc_width_, Qt::SolidLine);
        pen.setCapStyle(Qt::FlatCap);

        // Calculate the arc angle for this slice.
        qreal angle = (360.0 * 16.0 * slices_[loop].value) / range;

        // Draw the arc.
        painter.setPen(pen);
        painter.drawArc(rect, start_pos, static_cast<int>(angle));

        // Figure out where to draw the text on the arc.
        qreal text_angle = angle / 2.0;
        text_angle += start_pos;

        // Convert to radians.
        text_angle *= M_PI / (180.0 * 16.0);

        // Calculate text position.
        int   radius = rect.width() / 2;
        qreal x_pos  = radius + (radius * cos(text_angle));
        qreal y_pos  = radius - (radius * sin(text_angle));

        // Take into account the donut draw rectangle and the bounding rectangle of the font.
        QRect textRect = painter.boundingRect(QRect(0, 0, 0, 0), Qt::AlignLeft, slices_[loop].slice_text);
        x_pos += rect.x() - (textRect.width() / 2);
        y_pos += rect.y() + (textRect.height() / 2);

        // Save label positions and render later once all arc sections have been drawn.
        labelPositions.enqueue(QPoint(x_pos, y_pos));

        // Set the start position of the next arc.
        start_pos += angle;
    }

    // Draw the text labels on the arcs.
    painter.setPen(Qt::white);
    for (unsigned int loop = 0; loop < segment_count; loop++)
    {
        const QPoint labelPos = labelPositions.dequeue();
        painter.drawText(labelPos.x(), labelPos.y(), slices_[loop].slice_text);
    }

    font.setPixelSize(kDonutValuePixelFontSize);
    painter.setFont(font);
    painter.setPen(Qt::black);

    int text_width = QtCommon::QtUtils::GetPainterTextWidth(&painter, text_line_one_);

    int x_pos = (w - text_width) / 2;
    int y_pos = (h * 52) / 100;
    painter.drawText(x_pos, y_pos, text_line_one_);

    // Draw the description text.
    font.setPixelSize(kDonutTextPixelFontSize);
    painter.setFont(font);
    text_width = QtCommon::QtUtils::GetPainterTextWidth(&painter, text_line_two_);
    x_pos      = (w - text_width) / 2;
    painter.drawText(x_pos, (h * 66) / 100, text_line_two_);
}

void RMVScaledDonutWidget::SetNumSegments(unsigned int num_segments)
{
    if (num_segments_ != num_segments)
    {
        slices_.resize(num_segments);
        num_segments_ = num_segments;
    }
}

void RMVScaledDonutWidget::SetIndexValue(unsigned int index, qreal value)
{
    if (index < num_segments_)
    {
        slices_[index].value = value;
    }
}

void RMVScaledDonutWidget::SetIndexColor(unsigned int index, const QColor& fill_color)
{
    if (index < num_segments_)
    {
        slices_[index].fill_color = fill_color;
    }
}

void RMVScaledDonutWidget::SetIndexText(unsigned int index, const QString& text)
{
    if (index < num_segments_)
    {
        slices_[index].slice_text = text;
    }
}

void RMVScaledDonutWidget::SetArcWidth(qreal arc_width)
{
    arc_width_ = arc_width;
}

void RMVScaledDonutWidget::SetTextLineOne(const QString& text)
{
    text_line_one_ = text;
}

void RMVScaledDonutWidget::SetTextLineTwo(const QString& text)
{
    text_line_two_ = text;
}

void RMVScaledDonutWidget::SetBackgroundColor(const QColor& color)
{
    background_color_ = color;
}

void RMVScaledDonutWidget::resizeEvent(QResizeEvent* event)
{
    AdjustSize();
    QWidget::resizeEvent(event);
}
