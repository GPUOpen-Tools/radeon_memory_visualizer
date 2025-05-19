//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the resource event icons.
///
/// This is a helper class to draw icons common to the resource timeline and
/// the resource event table in the resource details pane.
///
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_EVENT_ICONS_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_EVENT_ICONS_H_

#include <QColor>
#include <QPainter>

#include "models/snapshot/resource_details_model.h"

namespace rmv
{
    /// @brief Class declaration.
    class ResourceEventIcons
    {
    public:
        /// @brief Constructor.
        ResourceEventIcons();

        /// @brief Destructor.
        ~ResourceEventIcons();

        /// @brief Draw a resource icon.
        ///
        /// @param [in] painter   Pointer to the Qt painter object.
        /// @param [in] x_pos     The left-most position of where to start drawing the icon.
        /// @param [in] y_pos     The mid point on the y-axis of where to start drawing the icon.
        /// @param [in] icon_size The size of the icon, in pixels, unscaled.
        /// @param [in] color     The icon color.
        /// @param [in] shape     The icon shape.
        void DrawIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color, ResourceIconShape shape) const;

    private:
        /// @brief Draw a circle icon.
        ///
        /// @param [in] painter   Pointer to the Qt painter object.
        /// @param [in] x_pos     The left-most position of where to start drawing the icon.
        /// @param [in] y_pos     The mid point on the y-axis of where to start drawing the icon.
        /// @param [in] icon_size The size of the icon, in pixels, unscaled.
        /// @param [in] color     The icon color.
        void DrawCircleIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const;

        /// @brief Draw a triangle.
        ///
        /// @param [in] painter   Pointer to the Qt painter object.
        /// @param [in] x_pos     The left-most position of where to start drawing the icon.
        /// @param [in] y_pos     The mid point on the y-axis of where to start drawing the icon.
        /// @param [in] icon_size The size of the icon, in pixels, unscaled.
        /// @param [in] color     The icon color.
        /// @param [in] inverted  Should the triangle be inverted.
        void DrawTriangleIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color, bool inverted) const;

        /// @brief Draw a square.
        ///
        /// @param [in] painter   Pointer to the Qt painter object.
        /// @param [in] x_pos     The left-most position of where to start drawing the icon.
        /// @param [in] y_pos     The mid point on the y-axis of where to start drawing the icon.
        /// @param [in] icon_size The size of the icon, in pixels, unscaled.
        /// @param [in] color     The icon color.
        void DrawSquareIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const;

        /// @brief Draw a cross icon.
        ///
        /// @param [in] painter   Pointer to the Qt painter object.
        /// @param [in] x_pos     The left-most position of where to start drawing the icon.
        /// @param [in] y_pos     The mid point on the y-axis of where to start drawing the icon.
        /// @param [in] icon_size The size of the icon, in pixels, unscaled.
        /// @param [in] color     The icon color.
        void DrawCrossIcon(QPainter* painter, int x_pos, int y_pos, int icon_size, QColor color) const;

        /// @brief Draw a cross.
        ///
        /// @param [in] painter   Pointer to the Qt painter object.
        /// @param [in] x_pos     The left-most position of where to start drawing the icon.
        /// @param [in] y_pos     The mid point on the y-axis of where to start drawing the icon.
        /// @param [in] icon_size The size of the icon, in pixels, unscaled.
        void DrawCross(QPainter* painter, int x_pos, int y_pos, int icon_size) const;
    };
}  // namespace rmv

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_EVENT_ICONS_H_
