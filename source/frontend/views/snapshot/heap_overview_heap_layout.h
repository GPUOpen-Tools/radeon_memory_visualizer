//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a layout for a single heap in the heap overview pane.
//=============================================================================

#ifndef RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_HEAP_LAYOUT_H_
#define RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_HEAP_LAYOUT_H_

#include "ui_heap_overview_heap_layout.h"

#include <QWidget>

#include "qt_common/custom_widgets/colored_legend_scene.h"
#include "qt_common/custom_widgets/colored_legend_graphics_view.h"

#include "rmt_types.h"

#include "models/snapshot/heap_overview_heap_model.h"

/// Class declaration
class HeapOverviewHeapLayout : public QWidget
{
    Q_OBJECT

public:
    /// The number of resource legends set up in the UI.
    static const int kNumResourceLegends = 6;

    /// Constructor.
    /// \param parent Pointer to the parent widget.
    explicit HeapOverviewHeapLayout(QWidget* parent = nullptr);

    /// Destructor.
    virtual ~HeapOverviewHeapLayout();

    /// Initialize the widget.
    /// \param heap The heap this widget represents.
    void Initialize(RmtHeapType heap);

    /// Update the widget.
    void Update();

    /// Gets the width of the section containing the donut, legend, and a horizontal spacer.
    /// \return width of the donut section in pixels.
    int DonutSectionWidth();

    /// Sets the minimum width of the donut section.
    /// \param width The minimum width in pixels to set the donut section.
    void SetDonutSectionWidth(const int width);

protected:
    /// Overridden window resize event.
    /// \param event the resize event object.
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    Ui::HeapOverviewHeapLayout* ui_;  ///< Pointer to the Qt UI design.

    rmv::HeapOverviewHeapModel* model_;                                         ///< Container class for the widget model.
    ColoredLegendGraphicsView*  resource_legends_views_[kNumResourceLegends];   ///< Duplicate of ui elements in array form for easy access.
    ColoredLegendScene*         resource_legends_scenes_[kNumResourceLegends];  ///< Pointer to the heap residency legends scene.
};

#endif  // RMV_VIEWS_SNAPSHOT_HEAP_OVERVIEW_HEAP_LAYOUT_H_
