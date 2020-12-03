//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a clickable table view. Extend the ScaledTableView
/// and allow the mouse cursor to change to a hand cursor if the table elements
/// can be clicked on.
//=============================================================================

#include "views/custom_widgets/rmv_clickable_table_view.h"

#include <QApplication>

RMVClickableTableView::RMVClickableTableView(QWidget* parent)
    : ScaledTableView(parent)
{
}

RMVClickableTableView::~RMVClickableTableView()
{
}

void RMVClickableTableView::enterEvent(QEvent* event)
{
    Q_UNUSED(event);
    qApp->setOverrideCursor(Qt::PointingHandCursor);
}

void RMVClickableTableView::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    qApp->restoreOverrideCursor();
}
