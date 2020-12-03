//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a helper class for custom button rendering for the
/// themes and colors pane.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_THEMES_AND_COLORS_ITEM_BUTTON_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_THEMES_AND_COLORS_ITEM_BUTTON_H_

#include "qt_common/custom_widgets/scaled_push_button.h"

/// Helper class for custom button rendering.
class ThemesAndColorsItemButton : public ScaledPushButton
{
public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit ThemesAndColorsItemButton(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~ThemesAndColorsItemButton();

    /// Themes and colors button color setter - stores color and font color values
    /// for use in custom drawing.
    /// \param color The color to set.
    void SetColor(const QColor& color);

    /// Themes and colors button paint event - overrides button draw function to
    /// implement custom drawing functionality.
    /// \param event Qt paint event.
    void paintEvent(QPaintEvent* event) override;

private:
    QColor button_color_;  ///< Color of this button.
    QColor font_color_;    ///< Font color of this button.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_THEMES_AND_COLORS_ITEM_BUTTON_H_
