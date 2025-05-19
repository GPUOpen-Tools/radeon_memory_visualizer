//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header file for the timeline colorizer control.
///
/// The colorizer is responsible for the functionality for the coloring the
/// timeline depending on the timeline type. It sets up the timeline type
/// combo box with the required timeline types currently supported by the
/// backend and updates the timeline and the legends depending on which
/// coloring mode is required.
///
//=============================================================================

#ifndef RMV_MODELS_TIMELINE_TIMELINE_COLORIZER_H_
#define RMV_MODELS_TIMELINE_TIMELINE_COLORIZER_H_

#include <QWidget>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_graphics_view.h"

#include "rmt_data_timeline.h"
#include "rmt_resource_list.h"

#include "models/colorizer_base.h"

namespace rmv
{
    /// @brief Class to handle control of the timeline type combo boxes and picking which colors to use.
    class TimelineColorizer : public ColorizerBase
    {
    public:
        /// @brief Constructor.
        explicit TimelineColorizer();

        /// @brief Destructor.
        virtual ~TimelineColorizer();

        /// @brief Initialize the timeline colorizer.
        ///
        /// @param [in] parent The parent pane or widget.
        /// @param [in] combo_box The 'color by' combo box to set up.
        /// @param [in] legends_view The graphics view containing the color legends.
        /// @param [in] type_list The list of timeline types required.
        void Initialize(QWidget* parent, ArrowIconComboBox* combo_box, ColoredLegendGraphicsView* legends_view, const RmtDataTimelineType* type_list);

        /// @brief Called when the combo box is selected.
        ///
        /// Update the internal data based on the selection in the combo box.
        ///
        /// @param [in] index The index of the combo box item selected.
        ///
        /// @return The new timeline type selected.
        RmtDataTimelineType ApplyColorMode(int index);

        /// @brief Override method to update color legends on the UI depending on the coloring mode (hides heap resource type).
        virtual void UpdateLegends() Q_DECL_OVERRIDE;

    private:
        RmtDataTimelineType timeline_type_;                                 ///< The timeline type.
        RmtDataTimelineType timeline_type_map_[kRmtDataTimelineTypeCount];  ///< The mapping of combo box index to timeline type.
    };
}  // namespace rmv

#endif  // RMV_MODELS_TIMELINE_TIMELINE_COLORIZER_H_
