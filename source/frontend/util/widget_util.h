//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Definition of a number of widget utilities. These functions
///  apply a common look and feel to various widget types.
//=============================================================================

#ifndef RMV_UTIL_WIDGET_UTIL_H_
#define RMV_UTIL_WIDGET_UTIL_H_

#include <QWidget>
#include <QString>
#include <QGraphicsView>
#include <QTableView>
#include <stdint.h>

#include "qt_common/custom_widgets/text_search_widget.h"
#include "qt_common/custom_widgets/double_slider_widget.h"
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_scene.h"

namespace rmv
{
    namespace widget_util
    {
        /// Initialize a double-slider widgets.
        void InitDoubleSlider(DoubleSliderWidget* double_slider_widgets);

        /// Initialize an ArrowIconComboBox for single selection.
        /// \param parent The combo box parent.
        /// \param combo_box The combo box to initialize.
        /// \param default_text The default text string.
        /// \param retain_default_text If true, the combo box text doesn't change to the selected
        /// item text (in the case of combo boxes requiring check boxes).
        /// \param prefix_text The text used to prefix the default text.
        void InitSingleSelectComboBox(QWidget*           parent,
                                      ArrowIconComboBox* combo_box,
                                      const QString&     default_text,
                                      bool               retain_default_text,
                                      const QString      prefix_text = "");

        /// Initialize an ArrowIconComboBox for multi selection.
        /// \param parent The combo box parent.
        /// \param combo_box The combo box to initialize.
        /// \param default_text The default text string.
        void InitMultiSelectComboBox(QWidget* parent, ArrowIconComboBox* combo_box, const QString& default_text);

        /// Initialize a graphics view to some common defaults.
        /// \param view Pointer to the QGraphicsView.
        /// \param fixed_height The maximum height.
        void InitGraphicsView(QGraphicsView* view, uint32_t fixed_height);

        /// Set a widget's background color.
        /// \param widget The widget to change background color.
        /// \param color The new color.
        void SetWidgetBackgroundColor(QWidget* widget, const QColor& color);

        /// Apply standard styling for a given top level pane.
        /// \param root The root widget.
        /// \param main_content The widget containing all content.
        /// \param scroll_area The scroll area.
        void ApplyStandardPaneStyle(QWidget* root, QWidget* main_content, QScrollArea* scroll_area);

        /// Init search box and double slider.
        /// \param text_search_widget text search item.
        /// \param double_slider_widget double slider item.
        void InitCommonFilteringComponents(TextSearchWidget* text_search_widget, DoubleSliderWidget* double_slider_widget);

        /// Method to initialize color legends.
        /// \param legend_widget The scene for this legend.
        /// \param view The graphics view for this legend.
        void InitColorLegend(ColoredLegendScene*& legend_widget, QGraphicsView* view);

        /// Set a custom palette for tables.
        /// \param view the table view.
        void UpdateTablePalette(QTableView* view);

        /// Get the height of a table depending on how many rows are in the table. Used to make the
        /// table as large as it needs to be so there are no empty rows.
        /// \param table_view The table.
        /// \param row_count The number of rows.
        int GetTableHeight(const QTableView* table_view, int row_count);

    }  // namespace widget_util
}  // namespace rmv

#endif  // RMV_UTIL_WIDGET_UTIL_H_
