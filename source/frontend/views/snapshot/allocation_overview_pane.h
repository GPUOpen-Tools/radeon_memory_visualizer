//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Allocation overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_ALLOCATION_OVERVIEW_PANE_H_
#define RMV_VIEWS_SNAPSHOT_ALLOCATION_OVERVIEW_PANE_H_

#include "ui_allocation_overview_pane.h"

#include <QGraphicsScene>

#include "models/colorizer.h"
#include "models/heap_combo_box_model.h"
#include "models/snapshot/allocation_overview_model.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_allocation_bar.h"

/// @brief Class declaration.
class AllocationOverviewPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit AllocationOverviewPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~AllocationOverviewPane();

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden hide event. Fired when this pane is closed.
    ///
    /// @param [in] event the hide event object.
    virtual void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;

    /// @brief Clean up.
    virtual void OnTraceClose() Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Open a snapshot.
    ///
    /// @param [in] snapshot The snapshot to open.
    virtual void OpenSnapshot(RmtDataSnapshot* snapshot) Q_DECL_OVERRIDE;

private slots:
    /// @brief Handle what happens when user changes filters.
    void ApplyFilters();

    /// @brief Handle what happens when user changes sort mode.
    void ApplySort();

    /// @brief Handle what happens when the color mode changes.
    void ColorModeChanged();

    /// @brief Handle what happens when a checkbox in the heap dropdown is checked or unchecked.
    ///
    /// @param [in] checked Whether the checkbox is checked or unchecked.
    void HeapChanged(bool checked);

    /// @brief Resize UI elements when the DPI scale factor changes.
    void OnScaleFactorChanged();

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
    /// @param [in] resource_identifier the resource Identifier.
    /// @param [in] navigate_to_pane If true, navigate to the allocation explorer pane.
    void SelectedResource(RmtResourceIdentifier resource_identifier, bool navigate_to_pane);

    /// @brief Handle what happens when the Normalize allocations checkbox is clicked.
    void ToggleNormalizeAllocations();

    /// @brief Handle what happens when the 'show aliasing' checkbox is clicked on.
    void ToggleAliasedResources() const;

    /// @brief Handle what happens when the allocation height slider changes.
    void AllocationHeightChanged();

private:
    /// @brief Resize all views.
    void ResizeItems();

    /// @brief Update the scene rect of the allocationListView.
    void UpdateAllocationListSceneRect();

    Ui::allocation_overview_pane* ui_;  ///< Pointer to the Qt UI design.

    rmv::AllocationOverviewModel*  model_;                           ///< Container class for the widget models.
    rmv::HeapComboBoxModel*        preferred_heap_combo_box_model_;  ///< The heap combo box model.
    QGraphicsScene*                allocation_list_scene_;           ///< Scene containing all allocation widgets.
    std::vector<RMVAllocationBar*> allocation_graphic_objects_;      ///< The list of allocation objects.
    rmv::Colorizer*                colorizer_;                       ///< The colorizer used by the 'color by' combo box.
    int                            allocation_height_;               ///< The allocation height.
};

#endif  // RMV_VIEWS_SNAPSHOT_ALLOCATION_OVERVIEW_PANE_H_
