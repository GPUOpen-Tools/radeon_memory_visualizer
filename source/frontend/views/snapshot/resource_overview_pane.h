//=============================================================================
// Copyright (c) 2018-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Resource Overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_OVERVIEW_PANE_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_OVERVIEW_PANE_H_

#include "ui_resource_overview_pane.h"

#include <QGraphicsScene>

#include "models/colorizer.h"
#include "models/snapshot/resource_overview_model.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_resource_details.h"
#include "views/custom_widgets/rmv_tree_map_blocks.h"

/// @brief Class declaration.
class ResourceOverviewPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit ResourceOverviewPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~ResourceOverviewPane();

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event The resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden showEvent handler.
    ///
    /// @param [in] event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Open a snapshot.
    ///
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

private slots:
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
    /// @param [in] broadcast           If true, emit a signal to select the resource on any listening panes,
    ///  otherwise call SelectResource() directly.
    /// @param [in] navigate_to_pane    If true, navigate to the resource details pane.
    void SelectedResource(RmtResourceIdentifier resource_identifier, bool broadcast, bool navigate_to_pane);

    /// @brief Slot to handle what happens when an unbound resource has been selected. In this case, need to use
    /// the allocation.
    ///
    /// @param [in] unbound_resource Pointer to the unbound resource.
    /// @param [in] broadcast        If true, emit a signal to select the unbound resource on any listening panes.
    /// @param [in] navigate_to_pane If true, navigate to the allocation explorer pane.
    void SelectedUnboundResource(const RmtResource* unbound_resource, bool broadcast, bool navigate_to_pane);

    /// @brief Show or hide the resource details.
    void ToggleResourceDetails();

    /// @brief Handle what happens when a checkbox in one of the filter dropdowns is checked or unchecked.
    ///
    /// @param [in] checked Whether the checkbox is checked or unchecked.
    void ComboFiltersChanged(bool checked);

    /// @brief Handle what happens when a checkbox in the resource usage type filter dropdown is checked or unchecked.
    ///
    /// @param [in] checked            Whether the checkbox is checked or unchecked.
    /// @param [in] changed_item_index The index of the check box in the combo box that changed.
    void ResourceComboFiltersChanged(bool checked, int changed_item_index);

    /// @brief The slicing level changed.
    void SlicingLevelChanged();

    /// @brief Handle what happens when the color mode changes.
    void ColorModeChanged();

    /// @brief Handle what happens when the size slider range changes.
    ///
    /// @param [in] min_value Minimum value of span.
    /// @param [in] max_value Maximum value of span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

private:
    /// @brief Refresh what's visible on the UI.
    void Refresh();

    /// @brief Select an unbound resource.
    ///
    /// @param unbound_resource The unbound resource selected.
    void SelectUnboundResource(const RmtResource* unbound_resource);

    /// @brief Update the title for the details section.
    void UpdateDetailsTitle();

    /// @brief Resize all relevant UI items.
    void ResizeItems();

    /// @brief Get the row in the slicing combo box for a slice type.
    ///
    /// Slice types are not in the same ordering as the enum and some slicing modes are disabled.
    ///
    /// @param [in] slice_type The slice type to look for.
    ///
    /// @return The row in the combo box the slice type is in.
    int GetRowForSliceType(int slice_type);

    /// @brief Update the combo box filters.
    ///
    /// Read the values from the combo box UI and inform the treeview model.
    void UpdateComboFilters();

    /// @brief Update the slicing level.
    ///
    /// Read the values from the combo box UI and inform the treeview model.
    void UpdateSlicingLevel();

    Ui::ResourceOverviewPane*   ui_;                                                            ///< Pointer to the Qt UI design.
    rmv::ResourceOverviewModel* model_;                                                         ///< Container class for the widget models.
    const RmtResource*          selected_resource_;                                             ///< Pointer to selected allocation.
    RMVResourceDetails*         resource_details_;                                              ///< Pointer to allocation details section at the bottom.
    QGraphicsScene*             allocation_details_scene_;                                      ///< The Qt scene for allocation details at the bottom.
    TreeMapModels               tree_map_models_;                                               ///< The models needed for the tree map.
    rmv::Colorizer*             colorizer_;                                                     ///< The colorizer used by the 'color by' combo box.
    int                         slice_mode_map_[RMVTreeMapBlocks::SliceType::kSliceTypeCount];  ///< The mapping of a slicing combo box index to slicing mode.
};

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_OVERVIEW_PANE_H_
