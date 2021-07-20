//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the snapshot table view.
///
/// Emit a signal whenever the table selection is changed so external objects
/// can respond to it.
///
//=============================================================================

#include "views/custom_widgets/rmv_snapshot_table_view.h"

RMVSnapshotTableView::RMVSnapshotTableView(QWidget* parent)
    : RMVClickableTableView(parent)
{
}

RMVSnapshotTableView::~RMVSnapshotTableView()
{
}

void RMVSnapshotTableView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    RMVClickableTableView::selectionChanged(selected, deselected);
    emit SelectionChanged();
}
