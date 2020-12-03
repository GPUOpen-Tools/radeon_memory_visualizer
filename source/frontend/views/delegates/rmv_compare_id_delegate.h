//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for the custom colored circle with checkmark.
//=============================================================================

#ifndef RMV_VIEWS_DELEGATES_RMV_COMPARE_ID_DELEGATE_H_
#define RMV_VIEWS_DELEGATES_RMV_COMPARE_ID_DELEGATE_H_

#include <QItemDelegate>
#include <QPolygon>

/// Support for the custom colored circle with checkmark.
class RMVCompareIdDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The delegate's parent.
    RMVCompareIdDelegate(QWidget* parent = nullptr);

    /// Destructor.
    ~RMVCompareIdDelegate();

    /// Overridden delegate paint method. This is responsible for the custom
    /// painting in the Color Swatch.
    /// \param painter Pointer to the painter object.
    /// \param option A QStyleOptionViewItem object, which contains the bounding
    /// rectangle for this element.
    /// \param index The model index for this element.
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

    /// Generates the checkmark geometry based on the supplied height of the widget.
    /// \param height Height of the widget in pixels, which includes the diameter
    /// of the circle, and the margins above and below the circle.
    void CalculateCheckmarkGeometry(const int height);

    /// Provides the default size hint, which is independent of style or index for this delegate.
    /// \return A default size hint that is scaled for the current DPI settings.
    QSize DefaultSizeHint() const;

protected:
    /// Overridden sizeHint of the CompareIdDelegate. See Qt documentation for parameters.
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    /// Calculates the desired margin and circle diameter based on the height which
    /// this widget has to draw.
    /// \param height Height of the widget in pixels, which includes the diameter
    /// of the circle, and the margins above and below the circle.
    /// \param [out] margin The height of the margin in pixels.
    /// \param [out] diameter The diameter of the circle in pixels.
    void HeightToMarginAndDiameter(const int height, int& margin, int& diameter) const;

    /// Draw a circle with a checkmark inside it.
    /// \param painter Pointer to a painter object.
    /// \param color Circle color
    /// \param x_pos The x coord
    /// \param y_pos The y coord
    /// \param diameter The circle diameter.
    void DrawCircleCheckmark(QPainter* painter, const QColor& color, int x_pos, int y_pos, int diameter) const;

    QPolygon checkmark_geometry_;  ///< The checkmark geometry.
};

#endif  // RMV_VIEWS_DELEGATES_RMV_COMPARE_ID_DELEGATE_H_
