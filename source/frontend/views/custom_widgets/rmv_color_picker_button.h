//=============================================================================
// Copyright (c) 2021-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a color picker button.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_BUTTON_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_BUTTON_H_

#include <QColor>
#include <QPushButton>

#include "qt_common/utils/color_palette.h"

/// @brief Helper class for color picker. Allows custom button painting.
class RMVColorPickerButton : public QPushButton
{
public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVColorPickerButton(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVColorPickerButton();

    /// @brief Set the color of the button.
    ///
    /// @param [in] color The button color.
    void SetColor(const QColor& color);

    /// @brief Provides the desired height for the specified width which will keep the button square.
    ///
    /// @param [in] width The pixel width of this widget.
    ///
    /// @return The desired pixel height of this width.
    virtual int heightForWidth(int width) const Q_DECL_OVERRIDE;

    /// @brief Size hint, which is the scaled default button dimensions.
    ///
    /// @return The scaled size hint.
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

    /// @brief Minimum size hint, which is the unscaled default button dimensions.
    ///
    /// @return The mimium size hint.
    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;

protected:
    /// @brief Picker button paint event.
    ///
    /// Overrides button draw function to implement custom drawing functionality.
    ///
    /// @param [in] event Qt paint event.
    virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

private:
    QColor button_color_;  ///< Color of this button.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_BUTTON_H_
