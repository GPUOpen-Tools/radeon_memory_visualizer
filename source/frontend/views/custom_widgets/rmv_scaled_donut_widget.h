//=============================================================================
// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the donut widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_SCALED_DONUT_WIDGET_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_SCALED_DONUT_WIDGET_H_

#include <QWidget>
#include <QPainter>

/// @brief Support for the donut graphics item widget.
class RMVScaledDonutWidget : public QWidget
{
public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVScaledDonutWidget(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVScaledDonutWidget();

    /// @brief Implementation of Qt's heightForWidth method.
    ///
    /// @param [in] width The width to calculate the height from.
    ///
    /// @return The width (force proportional aspect ratio).
    virtual int heightForWidth(int width) const Q_DECL_OVERRIDE;

    /// @brief Set the number of segments for this control.
    ///
    /// This is the number of unique data elements to be shown in this widget.
    ///
    /// @param [in] num_segments The number of segments needed.
    void SetNumSegments(unsigned int num_segments);

    /// @brief Set the value for the given index for the widget.
    ///
    /// @param [in] index The index whose value is to change.
    /// @param [in] value The new value to use.
    void SetIndexValue(unsigned int index, qreal value);

    /// @brief Set the fill color for the given index for the widget.
    ///
    /// @param [in] index      The index whose color is to change.
    /// @param [in] fill_color The color to use.
    void SetIndexColor(unsigned int index, const QColor& fill_color);

    /// @brief Set the text to be displayed in the pie segment.
    ///
    /// @param [in] index The index whose text is to change.
    /// @param [in] text  The text to be shown.
    void SetIndexText(unsigned int index, const QString& text);

    /// @brief Set how wide the donut section should be.
    ///
    /// @param [in] arc_width The width of the donut arc.
    void SetArcWidth(qreal arc_width);

    /// @brief Set the first line of text inside the donut.
    ///
    /// @param [in] text The text to set.
    void SetTextLineOne(const QString& text);

    /// @brief Set the second line of text inside the donut.
    ///
    /// @param [in] text Text to set.
    void SetTextLineTwo(const QString& text);

    /// @brief Set the background color.
    ///
    /// @param [in] color The color to set.
    void SetBackgroundColor(const QColor& color);

    /// @brief Adjust the size of the widget and proportionately adjust the font and pen point sizes.
    void AdjustSize();

protected:
    /// @brief Implementation of Qt's paint event for this widget.
    ///
    /// @param [in] event The paint event.
    virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    /// @brief Capture a resize event.
    ///
    /// @param [in] event The resize event.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    /// @brief Container class for each slice in the donut.
    class SliceData
    {
    public:
        /// @brief Constructor.
        SliceData()
            : value(0)
            , fill_color(Qt::black)
            , slice_text()
        {
        }

        /// @brief Destructor.
        ~SliceData()
        {
        }

        qreal   value;       ///< Current value to represent.
        QColor  fill_color;  ///< Color used to fill the value part of the arc.
        QString slice_text;  ///< Additional text description.
    };

    QVector<SliceData> slices_;  ///< The list of donut slices.

    int          width_;             ///< Width of this widget.
    int          height_;            ///< Height of this widget.
    unsigned int num_segments_;      ///< Number of segments (values) in the donut.
    qreal        arc_width_;         ///< Width of the control arc, in pixels. This is used as the size of the pen used to draw the arc.
    QString      text_line_one_;     ///< Text in the center of the donut.
    QString      text_line_two_;     ///< Text in the center of the donut.
    QColor       background_color_;  ///< The background color.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_SCALED_DONUT_WIDGET_H_
