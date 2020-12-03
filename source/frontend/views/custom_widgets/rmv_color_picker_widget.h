//=============================================================================
/// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for a color picker widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_WIDGET_H_

#include <QWidget>
#include <QColor>
#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>

#include "qt_common/utils/color_palette.h"

/// Support for RMV's color picker widget.
class RMVColorPickerWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The color picker widget's parent.
    explicit RMVColorPickerWidget(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~RMVColorPickerWidget();

    /// Set the number of button rows/columns.
    /// \param rows The button row count.
    /// \param columns The button column count.
    void SetRowAndColumnCount(unsigned int rows, unsigned int columns);

    /// Get the currently selected color.
    /// \return The currently selected color on the picker.
    QColor GetSelectedColor();

    /// Get the palette id of the currently selected color.
    /// \return The palette id of the currently selected color on the picker.
    int GetSelectedPaletteId();

    /// Get the color palette used by this color picker.
    /// \return The color palette currently in use by this picker.
    ColorPalette GetPalette();

    /// Set the selected color on the picker given a palette id.
    /// \param id The palette id of the color to select.
    void Select(int id);

    /// Set the palette for this picker to use.
    /// \param palette The ColorPalette object this picker will use.
    void SetPalette(const ColorPalette& palette);

signals:
    /// Signals that a color has been selected.
    /// \param palette_id The id of the color that has been changed in the palette
    /// \param color The color that corresponds to the id
    void ColorSelected(int palette_id, QColor color);

    /// Signals that the palette has changed.
    /// \param palette The new palette
    void PaletteChanged(const ColorPalette& palette);

private:
    ColorPalette palette_;            ///< Color palette used by this picker.
    QGridLayout  grid_layout_;        ///< Grid layout used to layout button array.
    QButtonGroup button_group_;       ///< Button group used to group all color buttons together.
    int          grid_row_count_;     ///< Grid row count.
    int          grid_column_count_;  ///< Grid column count.

    /// Generate and arrange the collection of buttons that make up this color
    /// picker.
    void GenerateButtons();

    /// Correctly set the color of all the buttons using colors from the palette.
    void SetButtonColors();

    class PickerButton;

private slots:

    /// Slot that is called when one of the buttons is clicked.
    /// \param button_id The id of the button that is clicked.
    void ButtonClicked(int button_id);
};

/// Helper class for color picker - allows custom button painting.
class RMVColorPickerWidget::PickerButton : public QPushButton
{
public:
    /// Constructor.
    /// \param parent The parent widget.
    explicit PickerButton(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~PickerButton();

    /// Set the color of the button.
    /// \param color The button color
    void SetColor(const QColor& color);

    /// Provides the desired height for the specified width which will
    /// keep the button square.
    /// \param width The pixel width of this widget.
    /// \return The desired pixel height of this width.
    virtual int heightForWidth(int width) const Q_DECL_OVERRIDE;

    /// Size hint, which is the scaled default button dimensions.
    /// \return The scaled size hint.
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

    /// Minimum size hint, which is the unscaled default button dimensions.
    /// \return The mimium size hint.
    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;

protected:
    /// Picker button paint event - overrides button draw function to implement
    /// custom drawing functionality.
    /// \param event Qt paint event.
    virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

private:
    QColor button_color_;  ///< Color of this button.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLOR_PICKER_WIDGET_H_
