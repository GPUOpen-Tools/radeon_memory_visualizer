//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Block Explorer pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_ALLOCATION_EXPLORER_PANE_H_
#define RMV_VIEWS_SNAPSHOT_ALLOCATION_EXPLORER_PANE_H_

#include "ui_allocation_explorer_pane.h"

#include <QItemSelection>
#include <QGraphicsScene>

#include "ui_allocation_explorer_pane.h"

#include "rmt_resource_list.h"

#include "models/allocation_bar_model.h"
#include "models/snapshot/allocation_explorer_model.h"
#include "util/widget_util.h"
#include "views/base_pane.h"
#include "views/colorizer.h"
#include "views/custom_widgets/rmv_allocation_bar.h"

/// Class declaration
class AllocationExplorerPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit AllocationExplorerPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~AllocationExplorerPane();

    /// Overridden show event. Fired when this pane is opened.
    /// \param event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// Open a snapshot.
    /// \param snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

    /// Resize relevant items.
    void ResizeItems();

private slots:
    /// Something changed in the allocation table, so need to update the UI. Same functionality as clicking on a table entry.
    /// \param selected The newly selected table item.
    /// \param deselected The previously selected table item.
    void AllocationTableChanged(const QItemSelection& selected, const QItemSelection& deselected);

    /// Handle what happens when a selection in the resource table changes, so need to update the UI. Same functionality as clicking on a table entry.
    /// \param selected The newly selected table item.
    /// \param deselected The previously selected table item.
    void ResourceTableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    /// Select an entry and go to the resource details pane.
    /// \param index the model index.
    void ResourceTableDoubleClicked(const QModelIndex& index) const;

    /// Handle what happens when the 'show aliasing' checkbox is clicked on.
    void AliasedResourceClicked() const;

    /// Select a resource on this pane. This is usually called when selecting a resource
    /// on a different pane to make sure the resource selection is propagated to all
    /// interested panes.
    /// \param resource_identifier the resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// Slot to handle what happens when a resource has been selected. This can be used to broadcast the resource
    /// selection to any panes listening for the signal so they can also update their selected resource.
    /// \param resource_identifier the resource Identifier.
    /// \param navigate_to_pane If true, navigate to the allocation explorer pane.
    void SelectedResource(RmtResourceIdentifier resource_identifier, bool navigate_to_pane);

    /// Handle what happens when the allocation search filter changes.
    void AllocationSearchBoxChanged();

    /// Handle what happens when the allocation 'filter by size' slider changes.
    /// \param min_value Minimum value of slider span.
    /// \param max_value Maximum value of slider span.
    void AllocationSizeFilterChanged(int min_value, int max_value);

    /// Handle what happens when the resource search filter changes.
    void ResourceSearchBoxChanged();

    /// Slot to handle what happens when the resource 'filter by size' slider changes.
    /// \param min_value Minimum value of slider span.
    /// \param max_value Maximum value of slider span.
    void ResourceSizeFilterChanged(int min_value, int max_value);

    /// Handle what happens when the color mode changes.
    void ColorModeChanged();

    /// Slot to handle what happens when an unbound resource is selected.
    /// \param virtual_allocation The allocation of the unbound resource that was selected.
    void SelectUnboundResource(const RmtVirtualAllocation* virtual_allocation);

    /// Slot to handle what happens after the allocation table is sorted.
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedAllocation();

    /// Slot to handle what happens after the resource list table is sorted.
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedResource();

    /// Helper function to set the maximum height of the allocation table so it only contains rows with valid data.
    inline void SetMaximumAllocationTableHeight()
    {
        ui_->allocation_table_view_->setMaximumHeight(
            rmv::widget_util::GetTableHeight(ui_->allocation_table_view_, model_->GetAllocationProxyModel()->rowCount()));
    }

    /// Helper function to set the maximum height of the resource table so it only contains rows with valid data.
    inline void SetMaximumResourceTableHeight()
    {
        ui_->resource_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->resource_table_view_, model_->GetResourceProxyModel()->rowCount()));
    }

private:
    /// Refresh a selected view.
    void Refresh();

    Ui::AllocationExplorerPane* ui_;  ///< Pointer to the Qt UI design.

    rmv::VirtualAllocationExplorerModel* model_;             ///< container class for the widget models.
    QGraphicsScene*                      allocation_scene_;  ///< The scene containing the allocation widget.
    RMVAllocationBar*                    allocation_item_;   ///< The graphic item associated with the displayed allocation.
    Colorizer*                           colorizer_;         ///< The colorizer used by the 'color by' combo box.
};

#endif  // RMV_VIEWS_SNAPSHOT_ALLOCATION_EXPLORER_PANE_H_
