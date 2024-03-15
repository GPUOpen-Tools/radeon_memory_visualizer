//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for the snapshot table view.
///
/// Emit a signal whenever the table selection is changed so external objects
/// can respond to it.
///
//=============================================================================

#include "models/timeline/snapshot_item_model.h"
#include "views/custom_widgets/rmv_snapshot_table_view.h"

RMVSnapshotTableView::RMVSnapshotTableView(QWidget* parent)
    : ScaledTableView(parent)
    , snapshot_name_delegate_(this)
{
    setItemDelegateForColumn(rmv::kSnapshotTimelineColumnName, &snapshot_name_delegate_);
}

RMVSnapshotTableView::~RMVSnapshotTableView()
{
}

void RMVSnapshotTableView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    ScaledTableView::selectionChanged(selected, deselected);
    emit SelectionChanged();
}
