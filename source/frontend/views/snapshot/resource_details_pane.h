//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Resource details pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_RESOURCE_DETAILS_PANE_H_
#define RMV_VIEWS_SNAPSHOT_RESOURCE_DETAILS_PANE_H_

#include "ui_resource_details_pane.h"

#include "qt_common/custom_widgets/colored_legend_scene.h"

#include "models/snapshot/resource_details_model.h"
#include "views/base_pane.h"
#include "views/delegates/rmv_resource_event_delegate.h"
#include "util/thread_controller.h"
#include "util/widget_util.h"

class MainWindow;

/// Class declaration.
class ResourceDetailsPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit ResourceDetailsPane(MainWindow* parent = nullptr);

    /// Destructor.
    virtual ~ResourceDetailsPane();

    /// Update time units.
    virtual void SwitchTimeUnits() Q_DECL_OVERRIDE;

    /// Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Overridden show event. Fired when this pane is opened.
    /// \param event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Overridden hide event. Fired when this pane is closed.
    /// \param event the hide event object.
    virtual void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;

private slots:
    /// Slot to handle what happens when the timeline is clicked on.
    /// Coordinate values passed in are logical positions between 0.0 and 1.0, where 0.0 corresponds to
    /// the left of the timeline and 1.0 corresponds to the right.
    /// \param logical_position The logical position clicked on in the timeline.
    /// \param icon_size The size of the icon in logical coordinates.
    void TimelineSelected(double logical_position, double icon_size);

    /// Select a resource on this pane. This is usually called when selecting a resource
    /// on a different pane to make sure the resource selection is propagated to all
    /// interested panes.
    /// \param resource_identifier the resource identifier of the resource to select.
    void SelectResource(RmtResourceIdentifier resource_identifier);

    /// Slot to handle what happens after the resource history table is sorted.
    /// Make sure the selected item (if there is one) is visible.
    void ScrollToSelectedEvent();

    /// Respond to DPI scale factor changes.
    void OnScaleFactorChanged();

private:
    /// Helper function to set the maximum height of the timeline table so it only contains rows with valid data.
    inline void SetMaximumTimelineTableHeight()
    {
        ui_->resource_timeline_table_view_->setMaximumHeight(
            rmv::widget_util::GetTableHeight(ui_->resource_timeline_table_view_, model_->GetTimelineProxyModel()->rowCount()));
    }

    /// Enum of residency types.
    enum ResidencyTypes
    {
        kResidencyLocal,
        kResidencyInvisible,
        kResidencySystem,
        kResidencyUnmapped,

        kResidencyCount
    };

    /// Refresh the UI.
    void Refresh();

    /// Resize all relevant UI items.
    void ResizeItems();

    Ui::ResourceDetailsPane*   ui_;                                    ///< Pointer to the Qt UI design.
    MainWindow*                main_window_;                           ///< Reference to the mainwindow (parent).
    rmv::ResourceDetailsModel* model_;                                 ///< Container class for the widget models.
    RmtResourceIdentifier      resource_identifier_;                   ///< The selected resource identifier.
    ColoredLegendScene*        legends_scene_heaps_[kResidencyCount];  ///< Pointer to the residency legends scene.
    RMVResourceEventDelegate*  legend_delegate_;                       ///< Delegate responsible for custom painting in the timeline table.
    rmv::ThreadController*     thread_controller_;                     ///< The thread for processing backend data.
};

#endif  // RMV_VIEWS_SNAPSHOT_RESOURCE_HISTORY_PANE_H_
