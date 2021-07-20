//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Allocation Explorer pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_ALLOCATION_EXPLORER_PANE_H_
#define RMV_VIEWS_SNAPSHOT_ALLOCATION_EXPLORER_PANE_H_

#include "ui_allocation_explorer_pane.h"

#include <QItemSelection>
#include <QGraphicsScene>

#include "ui_allocation_explorer_pane.h"

#include "models/colorizer.h"
#include "models/allocation_bar_model.h"
#include "models/snapshot/allocation_explorer_model.h"
#include "util/widget_util.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_allocation_bar.h"

/// @brief Class declaration.
class AllocationExplorerPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit AllocationExplorerPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~AllocationExplorerPane();

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Open a snapshot.
    ///
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

    /// @brief Resize relevant items.
    void ResizeItems();

private slots:
    /// @brief Something changed in the allocation table, so need to update the UI.
    ///
    /// Same functionality as clicking on a table entry.
    ///
    /// @param [in] selected   The newly selected table item.
    /// @param [in] deselected The previously selected table item.
    void AllocationTableChanged(const QItemSelection& selected, const QItemSelection& deselected);

    /// @brief Handle what happens when a selection in the resource table changes, so need to update the UI.
    ///
    /// Same functionality as clicking on a table entry.
    ///
    /// @param [in] selected   The newly selected table item.
    /// @param [in] deselected The previously selected table item.
    void ResourceTableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    /// @brief Select an entry and go to the resource details pane.
    ///
    /// @param [in] index the model index of the table item clicked on.
    void ResourceTableDoubleClicked(const QModelIndex& index) const;

    /// @brief Handle what happens when the 'show aliasing' checkbox is clicked on.
    void AliasedResourceClicked() const;

    /// @brief Select a resource on this pane.
    ///
    /// This is usually called when selecting a resource on a different pane to make sure the
    /// resource selection is propagated to all interested panes.
    ///
    /// @param [in] resource_identifier the resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// @brief Slot to handle what happens when a resource has been selected.
    ///
    /// This can be used to broadcast the resource selection to any panes listening for the signal
    /// so they can also update their selected resource.
    ///
    /// @param [in] resource_identifier The resource Identifier of the selected resource.
    /// @param [in] navigate_to_pane    If true, navigate to the allocation explorer pane.
    void SelectedResource(RmtResourceIdentifier resource_identifier, bool navigate_to_pane);

    /// @brief Handle what happens when the allocation search filter changes.
    void AllocationSearchBoxChanged();

    /// @brief Handle what happens when the allocation 'filter by size' slider changes.
    ///
    /// @param [in] min_value Minimum value of slider span.
    /// @param [in] max_value Maximum value of slider span.
    void AllocationSizeFilterChanged(int min_value, int max_value);

    /// @brief Handle what happens when the resource search filter changes.
    void ResourceSearchBoxChanged();

    /// @brief Slot to handle what happens when the resource 'filter by size' slider changes.
    ///
    /// @param [in] min_value Minimum value of slider span.
    /// @param [in] max_value Maximum value of slider span.
    void ResourceSizeFilterChanged(int min_value, int max_value);

    /// @brief Handle what happens when the color mode changes.
    void ColorModeChanged();

    /// @brief Slot to handle what happens when an unbound resource is selected.
    ///
    /// @param [in] virtual_allocation The allocation of the unbound resource that was selected.
    void SelectUnboundResource(const RmtVirtualAllocation* virtual_allocation);

    /// @brief Slot to handle what happens after the allocation table is sorted.
    ///
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedAllocation();

    /// @brief Slot to handle what happens after the resource list table is sorted.
    ///
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedResource();

    /// @brief Helper function to set the maximum height of the allocation table so it only contains rows with valid data.
    inline void SetMaximumAllocationTableHeight()
    {
        ui_->allocation_table_view_->setMaximumHeight(
            rmv::widget_util::GetTableHeight(ui_->allocation_table_view_, model_->GetAllocationProxyModel()->rowCount()));
    }

    /// @brief Helper function to set the maximum height of the resource table so it only contains rows with valid data.
    inline void SetMaximumResourceTableHeight()
    {
        ui_->resource_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->resource_table_view_, model_->GetResourceProxyModel()->rowCount()));
    }

private:
    /// @brief Refresh a selected view.
    void Refresh();

    Ui::AllocationExplorerPane* ui_;  ///< Pointer to the Qt UI design.

    rmv::VirtualAllocationExplorerModel* model_;             ///< Container class for the widget models.
    QGraphicsScene*                      allocation_scene_;  ///< The scene containing the allocation widget.
    RMVAllocationBar*                    allocation_item_;   ///< The graphic item associated with the displayed allocation.
    rmv::Colorizer*                      colorizer_;         ///< The colorizer used by the 'color by' combo box.
};

#endif  // RMV_VIEWS_SNAPSHOT_ALLOCATION_EXPLORER_PANE_H_
