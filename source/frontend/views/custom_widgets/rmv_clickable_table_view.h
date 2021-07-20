//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a clickable table view.
///
/// Extend the ScaledTableView and allow the mouse cursor to change to a hand
/// cursor if the table elements can be clicked on.
///
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CLICKABLE_TABLE_VIEW_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CLICKABLE_TABLE_VIEW_H_

#include "qt_common/custom_widgets/scaled_table_view.h"

/// @brief Container class for the clickable table view.
class RMVClickableTableView : public ScaledTableView
{
public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVClickableTableView(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~RMVClickableTableView();

    /// @brief Overridden Qt enterEvent.
    ///
    /// @param [in] event The event object.
    virtual void enterEvent(QEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden Qt leaveEvent.
    ///
    /// @param [in] event The event object.
    virtual void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CLICKABLE_TABLE_VIEW_H_
