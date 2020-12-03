//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \brief  Header file for a colorizer control.
/// The colorizer is responsible for the functionality for the "color by"
/// combo box across multiple panes. It sets up the combo box with all or
/// a subset of the available coloring modes and updates the allocations and
/// resource widgets and the legends depending on which coloring mode is
/// required.
//=============================================================================

#ifndef RMV_VIEWS_COLORIZER_H_
#define RMV_VIEWS_COLORIZER_H_

#include <QWidget>

#include "qt_common/custom_widgets/colored_legend_graphics_view.h"
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"

#include "colorizer_base.h"

/// Handles control of the "color by" combo boxes and picking which colors to use.
class Colorizer : public ColorizerBase
{
    Q_OBJECT

public:
    /// Constructor.
    explicit Colorizer();

    /// Destructor.
    virtual ~Colorizer();

    /// Initialize the colorizer.
    /// \param parent The parent pane or widget.
    /// \param combo_box The 'color by' combo box to set up.
    /// \param legends_view The graphics view containing the color legends.
    /// \param mode_list The list of color modes required.
    void Initialize(QWidget* parent, ArrowIconComboBox* combo_box, ColoredLegendGraphicsView* legends_view, const ColorMode* mode_list);

public slots:

    /// Slot to handle what happens when the combo box is selected.
    void ApplyColorMode();
};

#endif  // RMV_VIEWS_COLORIZER_H_
