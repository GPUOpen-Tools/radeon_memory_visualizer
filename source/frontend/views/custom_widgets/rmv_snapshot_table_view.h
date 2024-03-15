//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the snapshot table view.
///
/// Emit a signal whenever the table selection is changed so external objects
/// can respond to it.
///
//=============================================================================

#ifndef RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TABLE_VIEW_H_
#define RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TABLE_VIEW_H_

#include "qt_common/custom_widgets/scaled_table_view.h"
#include "views/delegates/rmv_snapshot_name_delegate.h"

/// @brief Class declaration for the Snapshot table view.
class RMVSnapshotTableView : public ScaledTableView
{
    Q_OBJECT
public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit RMVSnapshotTableView(QWidget* parent = NULL);

    /// @brief Destructor.
    virtual ~RMVSnapshotTableView();

protected:
    /// @brief Overridden Qt selectionChanged method.
    ///
    /// Called when a table entry is changed, either by mouse clicking on an entry or using the cursor keys.
    ///
    /// @param [in] selected   The items selected.
    /// @param [in] deselected The items deselected.
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) Q_DECL_OVERRIDE;

private:
    RMVSnapshotNameDelegate snapshot_name_delegate_;  ///< The delegate that handles editing of snapshot names for the snapshot table.

signals:
    /// @brief Signal the table selection has changed.
    void SelectionChanged();
};

#endif  // RMV_VIEWS_CUSTOM_WIDGETS_RMV_SNAPSHOT_TABLE_VIEW_H_
