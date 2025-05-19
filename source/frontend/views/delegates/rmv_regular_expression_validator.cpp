//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of the regular expression validator with tooltip support.
//=============================================================================

#include "rmv_regular_expression_validator.h"

#include <QToolTip>
#include <QValidator>

RMVRegularExpressionValidator::RMVRegularExpressionValidator(const QRegularExpression& regular_expression, QObject* parent)
    : QRegularExpressionValidator(regular_expression, parent)
{
}

RMVRegularExpressionValidator::~RMVRegularExpressionValidator()
{
}

QValidator::State RMVRegularExpressionValidator::validate(QString& input, int& position) const
{
    QValidator::State result = QValidator::Intermediate;

    result = QRegularExpressionValidator::validate(input, position);

    if (result == QValidator::Invalid)
    {
        QWidget* widget = qobject_cast<QWidget*>(parent());
        if (widget != nullptr)
        {
            QPoint tool_tip_position;
            tool_tip_position.setY(widget->sizeHint().height());
            QToolTip::showText(widget->mapToGlobal(tool_tip_position), invalid_input_message_);
        }
    }
    else
    {
        QToolTip::hideText();
    }

    return result;
}

void RMVRegularExpressionValidator::SetInvalidInputMessage(const QString& text)
{
    invalid_input_message_ = text;
}
