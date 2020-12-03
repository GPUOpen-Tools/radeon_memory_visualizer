//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header file for the timeline colorizer control.
/// The colorizer is responsible for the functionality for the coloring the
/// timeline depending on the timeline type. It sets up the timeline type
/// combo box with the required timeline types currently supported by the
/// backend and updates the timeline and the legends depending on which
/// coloring mode is required.
//=============================================================================

#ifndef RMV_VIEWS_TIMELINE_COLORIZER_H_
#define RMV_VIEWS_TIMELINE_COLORIZER_H_

#include <QWidget>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_graphics_view.h"

#include "rmt_format.h"
#include "rmt_resource_list.h"
#include "rmt_data_timeline.h"

#include "views/colorizer_base.h"

/// Class to handle control of the timeline type combo boxes and picking which colors to use.
class TimelineColorizer : public ColorizerBase
{
public:
    /// Constructor.
    explicit TimelineColorizer();

    /// Destructor.
    virtual ~TimelineColorizer();

    /// Initialize the timeline colorizer.
    /// \param parent The parent pane or widget.
    /// \param combo_box The 'color by' combo box to set up.
    /// \param legends_view The graphics view containing the color legends.
    /// \param type_list The list of timeline types required.
    void Initialize(QWidget* parent, ArrowIconComboBox* combo_box, ColoredLegendGraphicsView* legends_view, const RmtDataTimelineType* type_list);

    /// Called when the combo box is selected. Update the internal data
    /// based on the selection in the combo box.
    /// \param index The index of the combo box item selected.
    /// \return The new timeline type selected.
    RmtDataTimelineType ApplyColorMode(int index);

private:
    RmtDataTimelineType timeline_type_;                                 ///< The timeline type.
    RmtDataTimelineType timeline_type_map_[kRmtDataTimelineTypeCount];  ///< The mapping of combo box index to timeline type.
};

#endif  // RMV_VIEWS_TIMELINE_COLORIZER_H_
