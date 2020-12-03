//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for the resource event icons. This is a helper class to draw
/// icons common to the resource timeline and the resource event table in the
/// resource details pane
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_EVENT_ICONS_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_EVENT_ICONS_H_

#include <QPainter>
#include <QColor>

#include "models/snapshot/resource_details_model.h"

namespace rmv
{
    class ResourceEventIcons
    {
    public:
        /// Constructor.
        ResourceEventIcons();

        /// Destructor.
        ~ResourceEventIcons();

        /// Draw a resource icon.
        /// \param painter Pointer to the Qt painter object.
        /// \param x_pos The left-most position of where to start drawing the icon.
        /// \param y_pos The mid point on the y-axis of where to start drawing the icon.
        /// \param icon_size The size of the icon, in pixels, unscaled.
        /// \param color The icon color.
        /// \param shape The icon shape.
        void DrawIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color, ResourceIconShape shape) const;

    private:
        /// Draw a circle icon.
        /// \param painter Pointer to the Qt painter object.
        /// \param x_pos The left-most position of where to start drawing the icon.
        /// \param y_pos The mid point on the y-axis of where to start drawing the icon.
        /// \param icon_size The size of the icon, in pixels, unscaled.
        /// \param color The icon color.
        void DrawCircleIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const;

        /// Draw a triangle.
        /// \param painter Pointer to the Qt painter object.
        /// \param x_pos The left-most position of where to start drawing the icon.
        /// \param y_pos The mid point on the y-axis of where to start drawing the icon.
        /// \param icon_size The size of the icon, in pixels, unscaled.
        /// \param color The icon color.
        /// \param inverted Should the triangle be inverted.
        void DrawTriangleIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color, bool inverted) const;

        /// Draw a square.
        /// \param painter Pointer to the Qt painter object.
        /// \param x_pos The left-most position of where to start drawing the icon.
        /// \param y_pos The mid point on the y-axis of where to start drawing the icon.
        /// \param icon_size The size of the icon, in pixels, unscaled.
        /// \param color The icon color.
        void DrawSquareIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const;

        /// Draw a cross icon.
        /// \param painter Pointer to the Qt painter object.
        /// \param x_pos The left-most position of where to start drawing the icon.
        /// \param y_pos The mid point on the y-axis of where to start drawing the icon.
        /// \param icon_size The size of the icon, in pixels, unscaled.
        /// \param color The icon color.
        void DrawCrossIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const;

        /// Draw a cross.
        /// \param painter Pointer to the Qt painter object.
        /// \param x_pos The left-most position of where to start drawing the icon.
        /// \param y_pos The mid point on the y-axis of where to start drawing the icon.
        /// \param icon_size The size of the icon, in pixels, unscaled.
        void DrawCross(QPainter* painter, int x_pos, int y_pos, int icon_size) const;
    };
}  // namespace rmv

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_EVENT_ICONS_H_
