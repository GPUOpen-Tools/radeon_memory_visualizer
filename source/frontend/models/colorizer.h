//=============================================================================
// Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header file for a colorizer control.
///
/// The colorizer is responsible for the functionality for the "color by"
/// combo box across multiple panes. It sets up the combo box with all or
/// a subset of the available coloring modes and updates the allocations and
/// resource widgets and the legends depending on which coloring mode is
/// required.
///
//=============================================================================

#ifndef RMV_MODELS_COLORIZER_H_
#define RMV_MODELS_COLORIZER_H_

#include <QWidget>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_graphics_view.h"

#include "colorizer_base.h"

namespace rmv
{
    /// @brief Handles control of the "color by" combo boxes and picking which colors to use.
    class Colorizer : public ColorizerBase
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        explicit Colorizer();

        /// @brief Destructor.
        virtual ~Colorizer();

        /// @brief Initialize the colorizer.
        ///
        /// @param [in] parent       The parent pane or widget.
        /// @param [in] combo_box    The 'color by' combo box to set up.
        /// @param [in] legends_view The graphics view containing the color legends.
        /// @param [in] mode_list    The list of color modes required.
        void Initialize(QWidget* parent, ArrowIconComboBox* combo_box, ColoredLegendGraphicsView* legends_view, const ColorMode* mode_list);

    public slots:
        /// @brief Slot to handle what happens when the combo box is selected.
        void ApplyColorMode();
    };
}  // namespace rmv

#endif  // RMV_MODELS_COLORIZER_H_
