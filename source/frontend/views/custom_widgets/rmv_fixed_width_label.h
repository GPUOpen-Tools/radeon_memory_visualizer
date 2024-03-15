//=============================================================================
/// Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for the fixed width label widget.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_FIXED_WIDTH_LABEL_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_FIXED_WIDTH_LABEL_H_

#include "qt_common/custom_widgets/scaled_label.h"

/// Reimplements the scaled label to have a fixed width.
class RmvFixedWidthLabel : public ScaledLabel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent widget
    RmvFixedWidthLabel(QWidget* parent);

    /// Virtual destructor
    virtual ~RmvFixedWidthLabel();

    void SetWidestTextString(const QString& string);

    /// @brief Provide a sizeHint with a fixed width to match the size of the longest expected string.
    ///
    /// @return The desired size of the widget.
    QSize sizeHint() const Q_DECL_OVERRIDE;

private:
    QString widest_text_string_;  ///< A string representing the longest expected width.
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_FIXED_WIDTH_LABEL_H_
