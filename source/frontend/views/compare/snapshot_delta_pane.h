//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Snapshot Delta pane.
//=============================================================================

#ifndef RMV_VIEWS_COMPARE_SNAPSHOT_DELTA_PANE_H_
#define RMV_VIEWS_COMPARE_SNAPSHOT_DELTA_PANE_H_

#include "ui_snapshot_delta_pane.h"

#include "qt_common/custom_widgets/colored_legend_scene.h"

#include "rmt_format.h"

#include "models/compare/snapshot_delta_model.h"
#include "views/base_pane.h"
#include "views/custom_widgets/rmv_carousel.h"
#include "views/custom_widgets/rmv_delta_display.h"
#include "util/definitions.h"

/// Enum containing indices for the snapshot delta information.
enum SnapshotDeltaDataType
{
    kSnapshotDeltaTypeAvailableSize,
    kSnapshotDeltaTypeAllocatedAndBound,
    kSnapshotDeltaTypeAllocatedAndUnbound,
    kSnapshotDeltaTypeAllocationCount,
    kSnapshotDeltaTypeResourceCount,

    kSnapshotDeltaTypeCount,
};

/// Pairs a delta display row with a simple line.
struct DeltaDisplayLinePair
{
    RMVDeltaDisplay* display;  ///< The delta items.
    QFrame*          line;     ///< The separator line.
};

/// Class declaration.
class SnapshotDeltaPane : public BasePane
{
    Q_OBJECT

public:
    /// Constructor.
    /// \param parent The widget's parent.
    explicit SnapshotDeltaPane(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~SnapshotDeltaPane();

    /// Overridden show event. Fired when this pane is opened.
    /// \param event the show event object.
    virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    /// Update this pane with any redraw ops that should happen on pane switch.
    virtual void PaneSwitched() Q_DECL_OVERRIDE;

    /// Reset UI state.
    virtual void Reset() Q_DECL_OVERRIDE;

    /// Update UI coloring.
    virtual void ChangeColoring() Q_DECL_OVERRIDE;

    /// Refresh what's visible on the UI.
    void Refresh();

private slots:
    /// Switch snapshots.
    void SwitchSnapshots();

    /// Resize widgets as needed according to the DPI Scale.
    void OnScaleFactorChanged();

private:
    /// Update the UI.
    void UpdateUI();

    /// Resize relevant items.
    void ResizeItems();

    /// Add the memory delta legends to the required scene.
    void AddMemoryDeltaLegends();

    Ui::SnapshotDeltaPane* ui_;  ///< Pointer to the Qt UI design.

    rmv::SnapshotDeltaModel* model_;                                ///< Container class for the widget models.
    RMVCarousel*             carousel_;                             ///< Pointer to the carousel object.
    ColoredLegendScene*      legends_;                              ///< Pointer to the legends scene.
    QVector<DeltaItem>       delta_items_;                          ///< Array of delta items.
    DeltaDisplayLinePair     delta_line_pairs_[kRmtHeapTypeCount];  ///< Array of delta pairs.
};

#endif  // RMV_VIEWS_COMPARE_SNAPSHOT_DELTA_PANE_H_
