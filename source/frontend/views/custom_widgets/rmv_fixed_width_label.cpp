//=============================================================================
/// Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for the fixed width label widget.
//=============================================================================

#include "views/custom_widgets/rmv_fixed_width_label.h"

RmvFixedWidthLabel::RmvFixedWidthLabel(QWidget* parent)
    : ScaledLabel(parent)
{
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
}

RmvFixedWidthLabel::~RmvFixedWidthLabel()
{
}

void RmvFixedWidthLabel::SetWidestTextString(const QString& string)
{
    widest_text_string_ = string;
}

QSize RmvFixedWidthLabel::sizeHint() const
{
    QSize size = ScaledLabel::sizeHint();
    const QFontMetrics fm(font());
    const int          width = fm.horizontalAdvance(widest_text_string_);
    size.setWidth(width);

    return size;
}
