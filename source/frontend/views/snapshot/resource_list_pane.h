//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Allocation List pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_LIST_PANE_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_LIST_PANE_H_

#include "ui_resource_list_pane.h"

#include "rmt_resource_list.h"

#include "models/heap_combo_box_model.h"
#include "models/resource_usage_combo_box_model.h"
#include "models/snapshot/resource_list_model.h"
#include "util/widget_util.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_carousel.h"

/// Class declaration.
class ResourceListPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit ResourceListPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~ResourceListPane();

    /// Overridden show event. Fired when this pane is opened.
    /// \param event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Overridden hide event. Fired when this pane is closed.
    /// \param event the hide event object.
    virtual void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Clean up.
    virtual void OnTraceClose() Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// Open a snapshot.
    /// \param snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

private slots:
    /// Handle what happens when user changes the filter.
    void SearchBoxChanged();

    /// Slot to handle what happens when the 'filter by size' slider changes.
    /// \param min_value Minimum value of slider span.
    /// \param max_value Maximum value of slider span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

    /// Handle what happens when a checkbox in the heap dropdown is checked or unchecked.
    /// \param checked Whether the checkbox is checked or unchecked.
    void HeapChanged(bool checked);

    /// Handle what happens when a checkbox in the resource dropdown is checked or unchecked.
    /// \param checked Whether the checkbox is checked or unchecked.
    void ResourceChanged(bool checked);

    /// Resize child UI widgets when the DPI scale factor changes.
    void OnScaleFactorChanged();

    /// Handle what happens when an item in the resource table is clicked on.
    /// \param index the model index of the item clicked on.
    void TableClicked(const QModelIndex& index);

    /// Handle what happens when an item in the resource table is double-clicked on.
    /// \param index the model index of the item clicked on.
    void TableDoubleClicked(const QModelIndex& index);

    /// Select a resource on this pane. This is usually called when selecting a resource.
    /// on a different pane to make sure the resource selection is propagated to all
    /// interested panes.
    /// \param resource_identifier the resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// Slot to handle what happens after the resource list table is sorted.
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedResource();

private:
    /// Refresh what's visible on the UI.
    void Refresh();

    /// Resize relevant items.
    void ResizeItems();

    /// Populate the resource list table.
    void PopulateResourceTable();

    /// Update the carousel.
    void UpdateCarousel();

    /// Select the selected resource in the table and scroll to it if the resource is valid.
    void SelectResourceInTable();

    /// Helper function to set the maximum height of the table so it only contains rows with valid data.
    inline void SetMaximumResourceTableHeight()
    {
        ui_->resource_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->resource_table_view_, model_->GetResourceProxyModel()->rowCount()));
    }

    Ui::ResourceListPane*            ui_;                              ///< Pointer to the Qt UI design.
    rmv::ResourceListModel*          model_;                           ///< Container class for the widget models.
    rmv::HeapComboBoxModel*          preferred_heap_combo_box_model_;  ///< The preferred heap combo box model.
    rmv::ResourceUsageComboBoxModel* resource_usage_combo_box_model_;  ///< The resource usage model.
    RMVCarousel*                     carousel_;                        ///< the carousel on the top.
    bool                             model_valid_;                     ///< Is the model data valid.
    RmtResourceIdentifier            selected_resource_identifier_;    ///< Identifier of the selected resource.
};

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_LIST_PANE_H_
