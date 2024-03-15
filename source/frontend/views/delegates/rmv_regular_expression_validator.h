//=============================================================================
// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the regular expression validator with tooltip support.
//=============================================================================

#ifndef RMV_VIEWS_DELEGATES_RMV_REGULAR_EXPRESSION_VALIDATOR_H_
#define RMV_VIEWS_DELEGATES_RMV_REGULAR_EXPRESSION_VALIDATOR_H_

#include <QRegularExpressionValidator>

/// Support for the regular expression validator.
class RMVRegularExpressionValidator : public QRegularExpressionValidator
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The validator's parent.
    explicit RMVRegularExpressionValidator(QObject* parent = nullptr);

    /// @brief Constructor.
    ///
    /// @param [in] regular_expression The regular expression string.
    /// @param [in] parent             The parent object for the validator.
    explicit RMVRegularExpressionValidator(const QRegularExpression& regular_expression, QObject* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVRegularExpressionValidator();

    /// @brief Override to handle validating the user's input string.
    ///
    /// @param [in] input    The string entered by the user.
    /// @param [in] position The current cursor position in the input string.
    ///
    /// @return The validation state.
    virtual State validate(QString& input, int& position) const Q_DECL_OVERRIDE;

    /// @brief Assign a string displayed when the user enters an invalid character.
    ///
    /// @param [in] text The tooltip text message displayed.
    void SetInvalidInputMessage(const QString& text);

private:
    QString invalid_input_message_;  ///< The tooltip message displayed if the input string is invalid.
};

#endif  // RMV_VIEWS_DELEGATES_RMV_REGULAR_EXPRESSION_VALIDATOR_H_
