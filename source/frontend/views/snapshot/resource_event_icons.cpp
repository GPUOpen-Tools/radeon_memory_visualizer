//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the resource event icons.
//=============================================================================

#include "views/snapshot/resource_event_icons.h"

#include <QPainterPath>

#include "qt_common/utils/scaling_manager.h"

namespace rmv
{
    ResourceEventIcons::ResourceEventIcons()
    {
    }

    ResourceEventIcons::~ResourceEventIcons()
    {
    }

    void ResourceEventIcons::DrawIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color, ResourceIconShape shape) const
    {
        switch (shape)
        {
        case rmv::kIconShapeCircle:
            DrawCircleIcon(painter, x_pos, y_pos, icon_size, color);
            break;

        case rmv::kIconShapeTriangle:
            DrawTriangleIcon(painter, x_pos, y_pos, icon_size, color, false);
            break;

        case rmv::kIconShapeInvertedTriangle:
            DrawTriangleIcon(painter, x_pos, y_pos, icon_size, color, true);
            break;

        case rmv::kIconShapeSquare:
            DrawSquareIcon(painter, x_pos, y_pos, icon_size, color);
            break;

        case rmv::kIconShapeCross:
            DrawCrossIcon(painter, x_pos, y_pos, icon_size, color);
            break;

        default:
            break;
        }
    }

    void ResourceEventIcons::DrawCircleIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawEllipse(QRect(x_pos, y_pos - (icon_size / 2), icon_size, icon_size));
    }

    void ResourceEventIcons::DrawTriangleIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color, bool inverted) const
    {
        QPainterPath path;

        int tip;
        int base;
        if (!inverted)
        {
            tip  = y_pos - (icon_size / 2);
            base = tip + icon_size;
        }
        else
        {
            tip  = y_pos + (icon_size / 2);
            base = tip - icon_size;
        }

        path.moveTo(x_pos, base);
        path.lineTo(static_cast<qreal>(x_pos) + (static_cast<qreal>(icon_size) / 2.0), tip);
        path.lineTo(static_cast<qreal>(x_pos) + static_cast<qreal>(icon_size), base);
        path.lineTo(x_pos, base);

        painter->setPen(Qt::NoPen);
        painter->fillPath(path, QBrush(color));
    }

    void ResourceEventIcons::DrawSquareIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawRect(x_pos, y_pos - (icon_size / 2), icon_size, icon_size);
    }

    void ResourceEventIcons::DrawCrossIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const
    {
        static const int kPenSize        = icon_size / 4;
        static const int kScaledIconSize = (icon_size - kPenSize);
        static const int kScaledOffset   = (kPenSize / 2);

        painter->setPen(QPen(color, kPenSize));
        DrawCross(painter, x_pos + kScaledOffset, y_pos, kScaledIconSize);
    }

    void ResourceEventIcons::DrawCross(QPainter* painter, int x_pos, int y_pos, int icon_size) const
    {
        painter->drawLine(x_pos, y_pos - (icon_size / 2), x_pos + icon_size, y_pos + (icon_size / 2));
        painter->drawLine(x_pos + icon_size, y_pos - (icon_size / 2), x_pos, y_pos + (icon_size / 2));
    }

}  // namespace rmv
