//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the geometry settings.
///
/// Leverages Qt's saveGeometry/restoreGeometry methods to persist a widget's
/// position, size and state. The data is saved in the RMV settings file as a
/// hex string.
///
//=============================================================================

#include "settings/rmv_geometry_settings.h"

#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QWindow>
#include <QtCore>

#include "rmt_assert.h"

namespace rmv
{
    RMVGeometrySettings::RMVGeometrySettings()
    {
    }

    RMVGeometrySettings::~RMVGeometrySettings()
    {
    }

    void RMVGeometrySettings::Save(QWidget* widget)
    {
        QByteArray  array;
        QDataStream stream(&array, QIODevice::WriteOnly | QIODevice::ReadOnly);
        if (widget != nullptr)
        {
            array = widget->saveGeometry();
            QString geometry_data(array.toHex());
            RMVSettings::Get().SetStringValue(kSettingMainWindowGeometryData, geometry_data);
            RMVSettings::Get().SaveSettings();
        }
    }

    bool RMVGeometrySettings::Restore(QWidget* widget)
    {
        QString     geometry_data = RMVSettings::Get().GetStringValue(kSettingMainWindowGeometryData);
        QByteArray  array(QByteArray::fromHex(geometry_data.toLocal8Bit()));
        QDataStream stream(&array, QIODevice::WriteOnly | QIODevice::ReadOnly);
        if (widget != nullptr)
        {
            bool result = widget->restoreGeometry(array);
            Adjust(widget);
            return result;
        }
        return false;
    }

    void RMVGeometrySettings::Adjust(QWidget* widget)
    {
        QRect  widget_rect(widget->geometry());
        QRect  current_screen_rect(QGuiApplication::primaryScreen()->availableGeometry());
        QPoint current_pos = QPoint(widget_rect.x(), widget_rect.y());
        for (QScreen* screen : QGuiApplication::screens())
        {
            QRect screen_rect = screen->availableGeometry();
            if (screen_rect.contains(current_pos))
            {
                current_screen_rect = screen_rect;
                break;
            }
        }

        QRect adjusted_rect(widget_rect);
        if (widget_rect.width() > current_screen_rect.width())
        {
            adjusted_rect.setWidth(current_screen_rect.width());
        }

        if (widget_rect.right() > current_screen_rect.right())
        {
            adjusted_rect.moveRight(current_screen_rect.right());
        }

        if (widget_rect.left() < current_screen_rect.left())
        {
            adjusted_rect.moveLeft(current_screen_rect.left());
        }

        if (widget_rect.height() > current_screen_rect.height())
        {
            adjusted_rect.setHeight(current_screen_rect.height());
        }

        if (widget_rect.bottom() > current_screen_rect.bottom())
        {
            adjusted_rect.moveBottom(current_screen_rect.bottom());
        }

        if (widget_rect.top() < current_screen_rect.top())
        {
            adjusted_rect.moveTop(current_screen_rect.top());
        }

        if (widget_rect != adjusted_rect)
        {
            int titlebar_height = qApp->style()->pixelMetric(QStyle::PM_TitleBarHeight);
            int frame_thickness = qApp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            adjusted_rect.adjust(frame_thickness, frame_thickness + titlebar_height, -frame_thickness, -frame_thickness);
            widget->resize(adjusted_rect.size());
            widget->move(adjusted_rect.topLeft());
            RMVGeometrySettings::Save(widget);
        }
    }
}  // namespace rmv
