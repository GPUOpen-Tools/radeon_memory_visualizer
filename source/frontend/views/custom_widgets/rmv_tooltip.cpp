//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the custom tooltips.
///
/// Comprises of a tooltip data item (either text or swatches) and a
/// background. Both of these objects are derived from QGraphicsItem so can be
/// added to any scene. This class takes care of making sure the tooltip is
/// placed around about the mouse position and clipping it to the view
/// rectangle.
///
//=============================================================================

#include "views/custom_widgets/rmv_tooltip.h"

#include "views/custom_widgets/rmv_color_swatch_tooltip_item.h"

// Slightly transparent white background color for the custom tooltip.
static const QColor kTooltipBackgroundColor = QColor(255, 255, 255, 230);

// The width of the border around the text in the tooltip.
static const double kTooltipBorderWidth = 3.0;

// The default font size with no DPI scaling.
static const int kDefaultFontSize = 11;

// The position to place the tooltip to the right of the mouse so that the
// mouse cursor isn't obscured.
static const qreal kMouseXOffset = 25.0;

RMVTooltip::RMVTooltip()
    : tooltip_contents_(nullptr)
    , tooltip_background_(nullptr)
{
}

RMVTooltip::~RMVTooltip()
{
}

void RMVTooltip::HideToolTip()
{
    if (tooltip_contents_)
    {
        tooltip_contents_->hide();
    }
    if (tooltip_background_)
    {
        tooltip_background_->hide();
    }
}

void RMVTooltip::CreateToolTip(QGraphicsScene* scene, bool color_swatch)
{
    // Note: The scene takes ownership of these objects so no need to delete.
    tooltip_background_ = scene->addRect(QRect(), QPen(), kTooltipBackgroundColor);
    if (color_swatch)
    {
        tooltip_contents_ = new RMVColorSwatchTooltipItem();
    }
    else
    {
        tooltip_contents_ = new QGraphicsSimpleTextItem();
    }
    scene->addItem(tooltip_contents_);

    // Make sure the tooltip is on top of everything else in the scene.
    tooltip_background_->setZValue(1.0);
    tooltip_contents_->setZValue(1.0);

    // Don't scale the tooltip's border.
    auto pen = tooltip_background_->pen();
    pen.setCosmetic(true);
    tooltip_background_->setPen(pen);
}

void RMVTooltip::SetText(const QString& text_string)
{
    QFont font = tooltip_contents_->font();
    font.setPixelSize(kDefaultFontSize);
    tooltip_contents_->setFont(font);
    tooltip_contents_->setText(text_string);

    // The method setText() will update the bounding rectangle for tooltip_contents, so update
    // the background rectangle. Add a small margin so text is not overlapping the outline.
    QRectF      rect   = tooltip_contents_->boundingRect();
    const qreal offset = 2.0 * kTooltipBorderWidth;
    rect.setWidth(rect.width() + offset);
    rect.setHeight(rect.height() + offset);
    tooltip_background_->setRect(rect);
}

void RMVTooltip::SetColors(const QString& color_string)
{
    // Split up the color string and set colors via the custom data method in the base class.
    QStringList color_list = color_string.split("\n");
    int         num_rows   = color_list.size();
    for (auto index = 0; index < num_rows; index++)
    {
        tooltip_contents_->setData(index, color_list[index]);
    }
}

void RMVTooltip::UpdateToolTip(const QPointF& mouse_pos, const QPointF& scene_pos, const qreal view_width, const qreal view_height)
{
    Q_ASSERT(tooltip_contents_);
    Q_ASSERT(tooltip_background_);
    Q_ASSERT(view_width > 0);
    Q_ASSERT(view_height > 0);

    tooltip_background_->show();
    tooltip_contents_->show();

    const qreal tooltip_width  = tooltip_background_->rect().width();
    const qreal tooltip_height = tooltip_background_->rect().height();

    // If the tooltip is to the right of the mouse, offset it slightly so it isn't
    // hidden under the mouse.
    static const qreal mouse_x_offset = kMouseXOffset;
    QPointF            tooltip_offset = QPointF(mouse_x_offset, 0);

    // If the tooltip is going to run off the right of the scene, position it to the left
    // of the mouse.
    if (mouse_pos.x() > (view_width - mouse_x_offset - tooltip_width))
    {
        tooltip_offset.setX(-tooltip_width);
    }

    // If the tooltip is going to run off the bottom of the scene, position it at the bottom of the scene
    // so all the tooltip is visible.
    qreal max_y_pos = view_height - tooltip_height;
    if (mouse_pos.y() > max_y_pos)
    {
        tooltip_offset.setY(max_y_pos - mouse_pos.y());
    }

    // Tooltip labels use scene coordinates.
    const QPointF label_pos = scene_pos + tooltip_offset;
    tooltip_contents_->setPos(label_pos + QPointF(kTooltipBorderWidth, kTooltipBorderWidth));
    tooltip_background_->setPos(label_pos);
}
