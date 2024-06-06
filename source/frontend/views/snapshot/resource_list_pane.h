//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Resource List pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_LIST_PANE_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_LIST_PANE_H_

#include "ui_resource_list_pane.h"

#include "models/heap_combo_box_model.h"
#include "models/resource_usage_combo_box_model.h"
#include "models/snapshot/resource_list_model.h"
#include "util/widget_util.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_carousel.h"

/// @brief Class declaration.
class ResourceListPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit ResourceListPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~ResourceListPane();

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden hide event. Fired when this pane is closed.
    ///
    /// @param [in] event The hide event object.
    virtual void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event The resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Clean up.
    virtual void OnTraceClose() Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Open a snapshot.
    ///
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

private slots:
    /// @brief Handle what happens when user changes the filter.
    void SearchBoxChanged();

    /// @brief Slot to handle what happens when the 'filter by size' slider changes.
    ///
    /// @param [in] min_value Minimum value of slider span.
    /// @param [in] max_value Maximum value of slider span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

    /// @brief Handle what happens when a checkbox in the heap dropdown is checked or unchecked.
    ///
    /// @param [in] checked Whether the checkbox is checked or unchecked.
    void HeapChanged(bool checked);

    /// @brief Handle what happens when a checkbox in the resource dropdown is checked or unchecked.
    ///
    /// @param [in] checked            Whether the checkbox is checked or unchecked.
    /// @param [in] changed_item_index The index of the checkbox in the combo box that was changed.
    void ResourceChanged(bool checked, int changed_item_index);

    /// @brief Handle what happens when an item in the resource table is clicked on.
    ///
    /// @param [in] index the model index of the item clicked on.
    void TableClicked(const QModelIndex& index);

    /// @brief Handle what happens when an item in the resource table is double-clicked on.
    ///
    /// @param [in] index the model index of the item clicked on.
    void TableDoubleClicked(const QModelIndex& index);

    /// @brief Select a resource on this pane.
    ///
    /// This is usually called when selecting a resource on a different pane to make sure the
    /// resource selection is propagated to all interested panes.
    ///
    /// @param [in] resource_identifier the resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// @brief Slot to handle what happens after the resource list table is sorted.
    ///
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedResource();

private:
    /// @brief Refresh what's visible on the UI.
    void Refresh();

    /// @brief Resize relevant items.
    void ResizeItems();

    /// @brief Populate the resource list table.
    void PopulateResourceTable();

    /// @brief Update the carousel.
    void UpdateCarousel();

    /// @brief Select the selected resource in the table and scroll to it if the resource is valid.
    void SelectResourceInTable();

    /// @brief Helper function to set the maximum height of the table so it only contains rows with valid data.
    inline void SetMaximumResourceTableHeight()
    {
        ui_->resource_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->resource_table_view_, model_->GetResourceProxyModel()->rowCount()));
    }

    Ui::ResourceListPane*            ui_;                              ///< Pointer to the Qt UI design.
    rmv::ResourceListModel*          model_;                           ///< Container class for the widget models.
    rmv::HeapComboBoxModel*          preferred_heap_combo_box_model_;  ///< The preferred heap combo box model.
    rmv::ResourceUsageComboBoxModel* resource_usage_combo_box_model_;  ///< The resource usage model.
    RMVCarousel*                     carousel_;                        ///< The carousel on the top.
    bool                             model_valid_;                     ///< Is the model data valid.
    RmtResourceIdentifier            selected_resource_identifier_;    ///< Identifier of the selected resource.
};

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_LIST_PANE_H_
