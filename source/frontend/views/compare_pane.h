//=============================================================================
// Copyright Advanced Micro Devices, Inc.
// SPDX-License-Identifier: MIT
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the compare pane base class.
//=============================================================================

#ifndef RMV_VIEWS_COMPARE_PANE_H_
#define RMV_VIEWS_COMPARE_PANE_H_

#include <QWidget>

#include "views/base_pane.h"

/// @brief Base class for a compare pane in the UI.
class ComparePane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit ComparePane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~ComparePane();

    /// @brief Update the UI when a new comparison is done.
    virtual void Refresh() = 0;
};

#endif  // RMV_VIEWS_COMPARE_PANE_H_
