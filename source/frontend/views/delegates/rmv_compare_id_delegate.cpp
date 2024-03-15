//=============================================================================
// Copyright (c) 2017-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of RMVCompareIdDelegate.
//=============================================================================

#include "views/delegates/rmv_compare_id_delegate.h"

#include <QStyleOptionViewItem>
#include <QPainter>
#include <QModelIndex>
#include <QBrush>
#include <QColor>
#include <QPen>

#include "qt_common/utils/scaling_manager.h"

#include "models/resource_item_model.h"
#include "util/rmv_util.h"

// Desired size of the margin in pixels.
const qreal kMarginHint = 5;

// Desired diameter of the circle in pixels.
const qreal kDiameterHint = 20;

// Desired height is based on two margins and the diameter of the circle.
const qreal kHeightHint = kMarginHint + kDiameterHint + kMarginHint;

// Desired width is based on two side-by-side circles with a margin before, after, and in between the circles.
const qreal kWidthHint = kMarginHint + kDiameterHint + kMarginHint + kDiameterHint + kMarginHint;

RMVCompareIdDelegate::RMVCompareIdDelegate(QWidget* parent)
    : QItemDelegate(parent)
{
    CalculateCheckmarkGeometry(kHeightHint);
}

RMVCompareIdDelegate::~RMVCompareIdDelegate()
{
}

QSize RMVCompareIdDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return DefaultSizeHint();
}

QSize RMVCompareIdDelegate::DefaultSizeHint() const
{
    const int scaled_width_hint  = ScalingManager::Get().Scaled(kWidthHint);
    const int scaled_height_hint = ScalingManager::Get().Scaled(kHeightHint);

    return QSize(scaled_width_hint, scaled_height_hint);
}

void RMVCompareIdDelegate::CalculateCheckmarkGeometry(const int height)
{
    checkmark_geometry_.clear();

    int margin;
    int diameter;
    HeightToMarginAndDiameter(height, margin, diameter);

    // Calculate offsets of checkmark points.
    const qreal x_4_offset  = (4.0 / kDiameterHint) * diameter;
    const qreal x_12_offset = (12.0 / kDiameterHint) * diameter;
    const qreal x_10_offset = (10.0 / kDiameterHint) * diameter;
    const qreal x_2_offset  = (2.0 / kDiameterHint) * diameter;
    const qreal x_0_offset  = 0;

    const qreal y_12_offset = (12.0 / kDiameterHint) * diameter;
    const qreal y_4_offset  = (4.0 / kDiameterHint) * diameter;
    const qreal y_2_offset  = (2.0 / kDiameterHint) * diameter;
    const qreal y_9_offset  = (9.0 / kDiameterHint) * diameter;
    const qreal y_6_offset  = (6.0 / kDiameterHint) * diameter;
    const qreal y_8_offset  = (8.0 / kDiameterHint) * diameter;

    // Define the checkmark points.
    checkmark_geometry_ << QPoint(x_4_offset, y_12_offset);
    checkmark_geometry_ << QPoint(x_12_offset, y_4_offset);
    checkmark_geometry_ << QPoint(x_10_offset, y_2_offset);
    checkmark_geometry_ << QPoint(x_4_offset, y_9_offset);
    checkmark_geometry_ << QPoint(x_2_offset, y_6_offset);
    checkmark_geometry_ << QPoint(x_0_offset, y_8_offset);
}

void RMVCompareIdDelegate::DrawCircleCheckmark(QPainter* painter, const QColor& color, int x_pos, int y_pos, int diameter) const
{
    painter->setBrush(color);
    painter->drawEllipse(x_pos, y_pos, diameter, diameter);

    // Calculate offset percentages and actual positions based on ideal sizes (aka hints) and actual diameter.
    const qreal y_offset = (4.0 / kDiameterHint) * diameter;
    const qreal x_offset = (4.0 / kDiameterHint) * diameter;

    const int y_base = y_pos + y_offset;
    const int x_base = x_pos + x_offset;

    // Translate to new position for this checkmark.
    painter->translate(x_base, y_base);

    painter->setBrush(Qt::white);
    painter->drawPolygon(checkmark_geometry_);

    // Restore painter position.
    painter->translate(-x_base, -y_base);
}

void RMVCompareIdDelegate::HeightToMarginAndDiameter(const int height, int& margin, int& diameter) const
{
    // Calculate ratio of margin based on desired size.
    qreal hint_ratio = kMarginHint / kHeightHint;

    // Margin is defined by the ratio.
    margin = (qreal)height * hint_ratio;

    // Diameter is defined by whatever space is remaining.
    diameter = height - 2 * margin;
}

void RMVCompareIdDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column() == rmv::kResourceColumnCompareId)
    {
        QItemDelegate::drawBackground(painter, option, index);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);

        const QColor open_color     = rmv_util::GetSnapshotStateColor(kSnapshotStateViewed);
        const QColor compared_color = rmv_util::GetSnapshotStateColor(kSnapshotStateCompared);

        // Now calculate margin size based on the rect that was actually given for this item.
        int margin;
        int diameter;
        HeightToMarginAndDiameter(option.rect.height(), margin, diameter);

        // Other measurements are based on margin and diameter.
        const int compared_offset = margin + diameter + margin;
        const int color_index     = index.data().toInt();
        const int y_offset        = option.rect.y() + margin;

        if (color_index == rmv::kSnapshotCompareIdCommon)
        {
            DrawCircleCheckmark(painter, open_color, margin, y_offset, diameter);
            DrawCircleCheckmark(painter, compared_color, compared_offset, y_offset, diameter);
        }
        else if (color_index == rmv::kSnapshotCompareIdOpen)
        {
            DrawCircleCheckmark(painter, open_color, margin, y_offset, diameter);
        }
        else if (color_index == rmv::kSnapshotCompareIdCompared)
        {
            DrawCircleCheckmark(painter, compared_color, compared_offset, y_offset, diameter);
        }
    }
}
