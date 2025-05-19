//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for the Resource details pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_DETAILS_PANE_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_DETAILS_PANE_H_

#include "ui_resource_details_pane.h"

#include "qt_common/custom_widgets/colored_legend_scene.h"

#include "models/snapshot/resource_details_model.h"
#include "util/thread_controller.h"
#include "util/widget_util.h"
#include "views/base_pane.h"
#include "views/delegates/rmv_resource_event_delegate.h"

/// @brief Class declaration.
class ResourceDetailsPane : public BasePane
{
    Q_OBJECT

public:
    /// @brief Constructor.
    ///
    /// @param [in] parent The parent widget.
    explicit ResourceDetailsPane(QWidget* parent = nullptr);

    /// @brief Destructor.
    virtual ~ResourceDetailsPane();

    /// @brief Update time units.
    virtual void SwitchTimeUnits() Q_DECL_OVERRIDE;

    /// @brief Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// @brief Overridden window resize event.
    ///
    /// @param [in] event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden show event. Fired when this pane is opened.
    ///
    /// @param [in] event The show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// @brief Overridden hide event. Fired when this pane is closed.
    ///
    /// @param [in] event The hide event object.
    virtual void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;

    /// @brief Load the resource timeline.
    ///
    /// Called during a showEvent or just after the snapshot has loaded
    /// (in the case when switching from the memory leak pane).
    void LoadResourceTimeline();

private slots:
    /// @brief Slot to handle what happens when the timeline is clicked on.
    ///
    /// Coordinate values passed in are logical positions between 0.0 and 1.0, where 0.0 corresponds to
    /// the left of the timeline and 1.0 corresponds to the right.
    ///
    /// @param [in] logical_position The logical position clicked on in the timeline.
    /// @param [in] icon_size        The size of the icon in logical coordinates.
    void TimelineSelected(double logical_position, double icon_size);

    /// @brief Select a resource on this pane.
    ///
    /// This is usually called when selecting a resource on a different pane to make
    /// sure the resource selection is propagated to all interested panes.
    ///
    /// @param [in] resource_identifier The resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// @brief Slot to handle what happens after the resource history table is sorted.
    ///
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedEvent();

    /// @brief Update the pane based on the color theme.
    void OnColorThemeUpdated();

    /// @brief Show a context menu if the user right-clicks on the Properties table.
    ///
    /// Present the user to copy the properties table contents to the clipboard as raw
    /// text or as csv-formatted text.
    ///
    /// @param [in] pos  The position of the mouse when the user clicked.
    void ShowPropertiesTableContextMenu(const QPoint& pos);

    /// @brief Handle what happens when the user selects one of the right-click options.
    ///
    /// This will do the actual clipboard copy. The raw text format uses spacing to line
    /// the table contents up.
    ///
    /// @param [in] as_csv  If true, copy the text in csv format, otherwise save as raw text.
    void SavePropertiesToClipboard(bool as_csv) const;

private:
    /// @brief Helper function to set the maximum height of the timeline table so it only contains rows with valid data.
    inline void SetMaximumTimelineTableHeight()
    {
        ui_->resource_timeline_table_view_->setMaximumHeight(
            rmv::widget_util::GetTableHeight(ui_->resource_timeline_table_view_, model_->GetTimelineProxyModel()->rowCount()));
    }

    /// @brief Refresh the UI.
    void Refresh();

    /// @brief Resize all relevant UI items.
    void ResizeItems();

    /// @brief Enum of residency types.
    enum ResidencyTypes
    {
        kResidencyLocal,
        kResidencyInvisible,
        kResidencySystem,
        kResidencyUnmapped,

        kResidencyCount
    };

    Ui::ResourceDetailsPane*   ui_;                                    ///< Pointer to the Qt UI design.
    rmv::ResourceDetailsModel* model_;                                 ///< Container class for the widget models.
    RmtResourceIdentifier      resource_identifier_;                   ///< The selected resource identifier.
    ColoredLegendScene*        legends_scene_heaps_[kResidencyCount];  ///< Pointer to the residency legends scene.
    RMVResourceEventDelegate*  legend_delegate_;                       ///< Delegate responsible for custom painting in the timeline table.
    rmv::ThreadController*     thread_controller_;                     ///< The thread for processing backend data.
};

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_HISTORY_PANE_H_
