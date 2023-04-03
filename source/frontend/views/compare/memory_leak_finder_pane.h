//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Memory leak finder pane.
//=============================================================================

#ifndef RMV_VIEWS_COMPARE_MEMORY_LEAK_FINDER_PANE_H_
#define RMV_VIEWS_COMPARE_MEMORY_LEAK_FINDER_PANE_H_

#include "ui_memory_leak_finder_pane.h"

#include "models/compare/memory_leak_finder_model.h"
#include "models/heap_combo_box_model.h"
#include "models/resource_usage_combo_box_model.h"
#include "util/widget_util.h"
#include "views/base_pane.h"
#include "views/compare_pane.h"
#include "views/delegates/rmv_compare_id_delegate.h"

/// @brief Class declaration.
class MemoryLeakFinderPane : public ComparePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit MemoryLeakFinderPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~MemoryLeakFinderPane();

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Clean up.
    virtual void OnTraceClose() Q_DECL_OVERRIDE;

    /// @brief Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Update the UI when a new comparison is done.
    virtual void Refresh() Q_DECL_OVERRIDE;

private slots:
    /// @brief Handle what happens when user changes the filter.
    void SearchBoxChanged();

    /// @brief Update UI elements as needed due to DPI scale factor changes.
    void OnScaleFactorChanged();

    /// @brief Slot to handle what happens when the 'filter by size' slider changes.
    ///
    /// @param [in] min_value Minimum value of slider span.
    /// @param [in] max_value Maximum value of slider span.
    void FilterBySizeSliderChanged(int min_value, int max_value);

    /// @brief Checkboxes on the top were clicked.
    void CompareFilterChanged();

    /// @brief Refresh content if hashes changed.
    void UpdateHashes();

    /// @brief Slot to handle what happens after the resource list table is sorted.
    ///
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedResource();

    /// @brief Handle what happens when a checkbox in the heap dropdown is checked or unchecked.
    ///
    /// @param [in] checked Whether the checkbox is checked or unchecked.
    void HeapChanged(bool checked);

    /// @brief Handle what happens when a checkbox in the resource dropdown is checked or unchecked.
    ///
    /// @param [in] checked Whether the checkbox is checked or unchecked.
    void ResourceChanged(bool checked);

    /// @brief Slot to handle what happens when a resource in the table is double-clicked on.
    ///
    /// Select the resource and go to resource details.
    ///
    /// @param [in] index the model index of the selected item in the table.
    void TableDoubleClicked(const QModelIndex& index);

private:
    /// @brief Refresh what's visible on the UI.
    ///
    /// @param [in] reset_filters If true, reset the filters to the init state.
    void Update(bool reset_filters);

    /// @brief Figure out a filter given current UI state.
    ///
    /// @return filter flags.
    rmv::SnapshotCompareId GetCompareIdFilter() const;

    /// @brief Helper function to set the maximum height of the table so it only contains rows with valid data.
    inline void SetMaximumResourceTableHeight()
    {
        ui_->resource_table_view_->setMaximumHeight(rmv::widget_util::GetTableHeight(ui_->resource_table_view_, model_->GetResourceProxyModel()->rowCount()));
    }

    Ui::MemoryLeakFinderPane* ui_;  ///< Pointer to the Qt UI design.

    rmv::MemoryLeakFinderModel*      model_;                           ///< Container class for the widget models.
    rmv::HeapComboBoxModel*          preferred_heap_combo_box_model_;  ///< The heap combo box model.
    rmv::ResourceUsageComboBoxModel* resource_usage_combo_box_model_;  ///< The resource usage model.
    RMVCompareIdDelegate*            compare_id_delegate_;             ///< Custom delegate for compare ID column.
};

#endif  // RMV_VIEWS_COMPARE_MEMORY_LEAK_FINDER_PANE_H_
