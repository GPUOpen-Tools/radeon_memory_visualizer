//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Colors and Themes pane.
//=============================================================================

#ifndef RMV_VIEWS_SETTINGS_THEMES_AND_COLORS_PANE_H_
#define RMV_VIEWS_SETTINGS_THEMES_AND_COLORS_PANE_H_

#include <QButtonGroup>

#include "ui_themes_and_colors_pane.h"

#include "qt_common/custom_widgets/scaled_push_button.h"

#include "views/base_pane.h"
#include "views/custom_widgets/themes_and_colors_item_button.h"

/// Class declaration.
class ThemesAndColorsPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit ThemesAndColorsPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~ThemesAndColorsPane();

private:
    /// Refresh - updates all pane elements so they reflect the settings currently
    /// set in RMVSettings.
    void Refresh();

    /// Get the RMV settings color which corresponds to the button indicated by
    /// button_id.
    /// \param button_id The button id.
    /// \return The color setting.
    QColor GetSettingsColor(int button_id);

    /// Get the RMV settings palette id which corresponds to the button indicated by
    /// button_id.
    /// \param button_id The button id.
    /// \return The palette id setting.
    int GetSettingsPaletteId(int button_id);

    /// Set the RMV settings palette id which corresponds to the button indicated by
    /// button_id.
    /// \param button_id The button id.
    /// \param palette_id The palette id to assign in RMV settings.
    void SetSettingsPaletteId(int button_id, int palette_id);

signals:
    void RefreshedColors();

private slots:
    /// Picker color selected slot - called when a color is selected on the picker
    /// (also triggered by calling ColorPickerWidget::select()).
    /// \param palette_id The palette if of the selected color.
    /// \param color The QColor value of the selected color.
    void PickerColorSelected(int palette_id, const QColor& theme_color);

    /// Item button clicked slot - called when one of the item buttons is clicked.
    /// \param button_id The id of the selected button.
    void ItemButtonClicked(int button_id);

    /// Default settings button clicked slot - called when the default settings
    /// button is clicked.
    void DefaultSettingsButtonClicked();

    /// Default palette button clicked slot - called when the default palette
    /// button is clicked.
    void DefaultPaletteButtonClicked();

    /// RGB value changed slot - called when the on screen RGB values are changed,
    /// either by using the sliders or the spinboxes.
    void RgbValuesChanged();

private:
    Ui::ThemesAndColorsPane* ui_;            ///< Pointer to the Qt UI design.
    QButtonGroup             button_group_;  ///< Group for item buttons
};

#endif  // RMV_VIEWS_SETTINGS_THEMES_AND_COLORS_PANE_H_
