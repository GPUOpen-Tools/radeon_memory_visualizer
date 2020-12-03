//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Resource Overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_OVERVIEW_PANE_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_OVERVIEW_PANE_H_

#include "ui_resource_overview_pane.h"

#include <QGraphicsScene>

#include "rmt_resource_list.h"

#include "models/snapshot/resource_overview_model.h"
#include "views/base_pane.h"
#include "views/colorizer.h"
#include "views/custom_widgets/rmv_resource_details.h"
#include "views/custom_widgets/rmv_tree_map_blocks.h"

/// Class declaration.
class ResourceOverviewPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit ResourceOverviewPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~ResourceOverviewPane();

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Overridden showEvent handler.
    /// \param event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// Open a snapshot.
    /// \param snapshot_id snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

private slots:
    /// Update parts of the UI when the scale factor changes.
    void OnScaleFactorChanged();

    /// Select a resource on this pane. This is usually called when selecting a resource
    /// on a different pane to make sure the resource selection is propagated to all
    /// interested panes.
    /// \param resource_identifier the resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// Slot to handle what happens when a resource has been selected. This can be used to broadcast the resource
    /// selection to any panes listening for the signal so they can also update their selected resource.
    /// \param resource_identifier the resource Identifier.
    /// \param broadcast If true, emit a signal to select the resource on any listening panes, otherwise call
    /// SelectResource() directly.
    /// \param navigate_to_pane If true, navigate to the resource details pane.
    void SelectedResource(RmtResourceIdentifier resource_identifier, bool broadcast, bool navigate_to_pane);

    /// Slot to handle what happens when an unbound resource has been selected. In this case, need to use
    /// the allocation.
    /// \param allocation Pointer to the allocation.
    /// \param broadcast If true, emit a signal to select the allocation on any listening panes.
    /// \param navigate_to_pane If true, navigate to the allocation explorer pane.
    void SelectedUnboundResource(const RmtResource* unbound_resource, bool broadcast, bool navigate_to_pane);

    /// Show or hide the resource details.
    void ToggleResourceDetails();

    /// Handle what happens when a checkbox in one of the filter dropdowns is checked or unchecked.
    /// \param checked Whether the checkbox is checked or unchecked.
    void ComboFiltersChanged(bool checked);

    /// The slicing level changed.
    void SlicingLevelChanged();

    /// Handle what happens when the color mode changes.
    void ColorModeChanged();

    /// Handle what happens when the size slider range changes.
    /// \param min_value Minimum value of span.
    /// \param max_value Maximum value of span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

private:
    /// Refresh what's visible on the UI.
    void Refresh();

    /// Select an unbound resource.
    /// \param unbound_resource The unbound resource selected.
    void SelectUnboundResource(const RmtResource* unbound_resource);

    /// Update the title for the details section.
    void UpdateDetailsTitle();

    /// Resize all relevant UI items.
    void ResizeItems();

    /// Get the row in the slicing combo box for a slice type. Slice types are
    /// not in the same ordering as the enum and some slicing modes are disabled.
    int GetRowForSliceType(int slice_type);

    /// Update the combo box filters. Read the values from the combo box UI and
    /// inform the treeview model.
    void UpdateComboFilters();

    /// Update the slicing level.  Read the values from the combo box UI and
    /// inform the treeview model.
    void UpdateSlicingLevel();

    Ui::ResourceOverviewPane*   ui_;                                                            ///< Pointer to the Qt UI design.
    rmv::ResourceOverviewModel* model_;                                                         ///< Container class for the widget models.
    const RmtResource*          selected_resource_;                                             ///< Pointer to selected allocation.
    RMVResourceDetails*         resource_details_;                                              ///< Pointer to allocation details section at the bottom.
    QGraphicsScene*             allocation_details_scene_;                                      ///< The Qt scene for allocation details at the bottom.
    TreeMapModels               tree_map_models_;                                               ///< The models needed for the tree map.
    Colorizer*                  colorizer_;                                                     ///< The colorizer used by the 'color by' combo box.
    int                         slice_mode_map_[RMVTreeMapBlocks::SliceType::kSliceTypeCount];  ///< The mapping of a slicing combo box index to slicing mode.
};

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_OVERVIEW_PANE_H_
