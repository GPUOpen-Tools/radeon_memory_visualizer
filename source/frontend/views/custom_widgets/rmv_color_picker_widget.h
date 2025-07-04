//=============================================================================
// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a color picker widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_WIDGET_H_

#include <QButtonGroup>
#include <QColor>
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>

#include "qt_common/utils/color_palette.h"

/// @brief Support for a color picker widget.
class RMVColorPickerWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The color picker widget's parent.
    explicit RMVColorPickerWidget(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVColorPickerWidget();

    /// @brief Set the number of button rows/columns.
    ///
    /// @param [in] rows    The button row count.
    /// @param [in] columns The button column count.
    void SetRowAndColumnCount(unsigned int rows, unsigned int columns);

    /// @brief Get the currently selected color.
    ///
    /// @return The currently selected color on the picker.
    QColor GetSelectedColor();

    /// @brief Get the palette id of the currently selected color.
    ///
    /// @return The palette id of the currently selected color on the picker.
    int GetSelectedPaletteId();

    /// @brief Get the color palette used by this color picker.
    ///
    /// @return The color palette currently in use by this picker.
    ColorPalette GetPalette();

    /// @brief Set the selected color on the picker given a palette id.
    ///
    /// @param [in] id The palette id of the color to select.
    void Select(int id);

    /// @brief Set the palette for this picker to use.
    ///
    /// @param [in] palette The ColorPalette object this picker will use.
    void SetPalette(const ColorPalette& palette);

signals:
    /// @brief Signals that a color has been selected.
    ///
    /// @param [in] palette_id The id of the color that has been changed in the palette.
    /// @param [in] color      The color that corresponds to the id.
    void ColorSelected(int palette_id, QColor color);

    /// @brief Signals that the palette has changed.
    ///
    /// @param [in] palette The new palette.
    void PaletteChanged(const ColorPalette& palette);

private slots:
    /// @brief Slot that is called when one of the buttons is clicked.
    ///
    /// @param [in] button_id The id of the button that is clicked.
    void ButtonClicked(int button_id);

private:
    /// @brief Generate and arrange the collection of buttons that make up this color picker.
    void GenerateButtons();

    /// @brief Correctly set the color of all the buttons using colors from the palette.
    void SetButtonColors();

    ColorPalette palette_;            ///< Color palette used by this picker.
    QGridLayout  grid_layout_;        ///< Grid layout used to layout button array.
    QButtonGroup button_group_;       ///< Button group used to group all color buttons together.
    int          grid_row_count_;     ///< Grid row count.
    int          grid_column_count_;  ///< Grid column count.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_WIDGET_H_
