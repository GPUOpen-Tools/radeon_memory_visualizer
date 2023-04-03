//=============================================================================
// Copyright (c) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the custom tooltips.
///
/// Comprises of a tooltip data item (either text or swatches) and a
/// background. Both of these objects are derived from QGraphicsItem so can be
/// added to any scene. This class takes care of making sure the tooltip is
/// placed around about the mouse position and clipping it to the view
/// rectangle.
///
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_TOOLTIP_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_TOOLTIP_H_

#include <QGraphicsItem>
#include <QGraphicsScene>

/// @brief Container class for a custom tooltip.
class RMVTooltip
{
public:
    /// @brief Constructor.
    explicit RMVTooltip();

    /// @brief Destructor.
    virtual ~RMVTooltip();

    /// @brief Create the tool tip.
    ///
    /// @param [in] scene        The graphics scene on which to place the tooltip.
    /// @param [in] color_swatch Is a color swatch needed for this tooltip.
    void CreateToolTip(QGraphicsScene* scene, bool color_swatch);

    /// @brief Update the tool tip.
    ///
    /// @param [in] mouse_pos   The mouse position in the parent view.
    /// @param [in] scene_pos   The mouse position in the scene.
    /// @param [in] view_width  The width of the parent view.
    /// @param [in] view_height The height of the parent view.
    void UpdateToolTip(const QPointF& mouse_pos, const QPointF& scene_pos, const qreal view_width, const qreal view_height);

    /// @brief Hide the tool tip.
    void HideToolTip();

    /// @brief Set the text for this tooltip.
    ///
    /// @param [in] text_string The text to display in the tooltip.
    void SetText(const QString& text_string);

    /// @brief Set the colors for this tooltip.
    ///
    /// @param [in] color_string The colors for the swatches for each line of text.
    void SetColors(const QString& color_string);

private:
    QGraphicsSimpleTextItem* tooltip_contents_;    ///< Contents of the custom tool tip implementation.
    QGraphicsRectItem*       tooltip_background_;  ///< Background rect of the custom tool tip implementation.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_TOOLTIP_H_
