//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the resource event delegate.
//=============================================================================

#include "views/delegates/rmv_resource_event_delegate.h"

#include <QPainter>

const static QColor kTableSelectionColor(0, 120, 215);

// Default width and height of the icon sizeHint.
// They are the same value since it draws within a square; this includes
// a small padding around the actual icon.
const double RMVResourceEventDelegate::kIconDefaultSizeHint = 24;

// The icon size factor (percentage) relative to the height of the available rect. If the icon is to be 70%
// the height of the rect, a value of 0.7 should be used. This allows for some space above/below the icon
// in the rect.
const double RMVResourceEventDelegate::kIconSizeFactor = 0.7;

RMVResourceEventDelegate::RMVResourceEventDelegate(QWidget* parent, rmv::ResourceDetailsModel* model)
    : QStyledItemDelegate(parent)
    , model_(model)
{
}

RMVResourceEventDelegate::~RMVResourceEventDelegate()
{
}

QSize RMVResourceEventDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size_hint;

    if (index.column() == rmv::kResourceHistoryColumnLegend)
    {
        int scaled_dimension = kIconDefaultSizeHint;
        size_hint.setWidth(scaled_dimension);
        size_hint.setHeight(scaled_dimension);
    }
    else
    {
        size_hint = QStyledItemDelegate::sizeHint(option, index);
    }

    return size_hint;
}

void RMVResourceEventDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column() == rmv::kResourceHistoryColumnLegend)
    {
        const RmtResourceHistoryEventType event_type = (RmtResourceHistoryEventType)index.data().toInt();
        QColor                            color      = model_->GetColorFromEventType(event_type, false);

        // Draw cell background color.
        if (option.state & QStyle::State_Selected)
        {
            // Draw selection highlight.
            painter->fillRect(option.rect, QBrush(kTableSelectionColor));
            color = Qt::white;
        }

        painter->setRenderHint(QPainter::Antialiasing);

        rmv::ResourceIconShape shape = model_->GetShapeFromEventType(event_type);

        int    mid_y     = (option.rect.top() + option.rect.bottom()) / 2;
        double icon_size = option.rect.height() * kIconSizeFactor;

        // Calculate offset for the icon in the table.
        double x_offset = (option.rect.width() - icon_size) / 2.0;
        x_offset        = std::min<int>(x_offset, icon_size / 2);
        x_offset += option.rect.x();

        event_icons_.DrawIcon(painter, x_offset, mid_y, icon_size, color, shape);
    }
}
