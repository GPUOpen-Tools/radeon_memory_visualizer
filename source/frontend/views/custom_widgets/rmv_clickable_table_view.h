//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a clickable table view. Extend the ScaledTableView
/// and allow the mouse cursor to change to a hand cursor if the table elements
/// can be clicked on.
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_CLICKABLE_TABLE_VIEW_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_CLICKABLE_TABLE_VIEW_H_

#include "qt_common/custom_widgets/scaled_table_view.h"

/// Container class for the clickable table view.
class RMVClickableTableView : public ScaledTableView
{
public:
    /// Constructor
    explicit RMVClickableTableView(QWidget* parent = nullptr);

    /// Destructor
    virtual ~RMVClickableTableView();

    /// Overridden Qt enterEvent.
    /// \param event The event object.
    void enterEvent(QEvent* event) override;

    /// Overridden Qt leaveEvent.
    /// \param event The event object.
    void leaveEvent(QEvent* event) override;
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_CLICKABLE_TABLE_VIEW_H_
