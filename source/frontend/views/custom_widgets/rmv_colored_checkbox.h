//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header of a widget that implements a custom check box.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLORED_CHECKBOX_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLORED_CHECKBOX_H_

#include <QCheckBox>

/// @brief RMV-style colored checkbox that is aware of resize and mouse events.
class RMVColoredCheckbox : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(qreal button_text_ratio READ GetButtonTextRatio WRITE SetButtonTextRatio)

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVColoredCheckbox(QWidget* parent);

    /// @brief Destructor.
    virtual ~RMVColoredCheckbox();

    /// @brief Initialize an RMVColoredCheckbox.
    ///
    /// @param [in] checked         Whether the checkbox starts in the checked state or not.
    /// @param [in] primary_color   Primary color theme.
    /// @param [in] secondary_color Secondary color theme.
    /// @param [in] multi_color     True if this is a multi-color color checkbox.
    void Initialize(bool checked, const QColor& primary_color, const QColor& secondary_color, bool multi_color = false);

    /// @brief Update the text to be displayed next to the checkbox.
    ///
    /// @param [in] text The text to be displayed.
    void UpdateText(const QString& text);

    /// @brief Update the primary checkbox color. This is the color used in single-colored checkboxes.
    ///
    /// @param [in] color The primary color to use.
    void UpdatePrimaryColor(const QColor& color);

    /// @brief Update the secondary checkbox color.
    ///
    /// @param [in] color The primary color to use.
    void UpdateSecondaryColor(const QColor& color);

    /// @brief Should this checkbox be used a multi-color mode.
    ///
    /// @param [in] multi_color If true, set as multi-color, otherwise single color.
    void UpdateMultiColorState(bool multi_color);

    /// @brief Set the button-to-text ratio.
    ///
    /// @param [in] button_text_ratio The button to text ratio.
    void SetButtonTextRatio(qreal button_text_ratio);

    /// @brief Get the button-to-text ratio.
    ///
    /// @return The button to text ratio.
    qreal GetButtonTextRatio() const;

signals:
    /// @brief Signal to indicate that the checkbox was clicked on.
    void Clicked();

protected:
    /// @brief Provides a desired sizeHint that allows the text and bar to be visible.
    ///
    /// @return The size hint.
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's paint for this widget.
    ///
    /// @param [in] paint_event The paint event.
    virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event The resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Implementation of Qt's mouse press event for this widget.
    ///
    /// @param [in] event The mouse press event.
    virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    /// @brief Get the switch height based on the font size.
    ///
    /// @param [in] font_metrics The font metrics for the font used by this widget.
    ///
    /// @return The switch height.
    qreal GetSwitchHeight(const QFontMetricsF& font_metrics) const;

    QColor primary_color_;      ///< Primary color for checkbox. Either full color, or top half color if multicolor.
    QColor secondary_color_;    ///< Secondary color for checkbox. Either not used, or bottom half color if multicolor.
    bool   multi_color_;        ///< When enabled, top half will be primary color, and bottom half will be secondary color.
    qreal  button_text_ratio_;  ///< The button-to-text ratio ie how much bigger the button is relative to the button text.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_COLORED_CHECKBOX_H_
