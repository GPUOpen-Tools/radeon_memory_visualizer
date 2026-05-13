//=============================================================================
// Copyright (c) 2019-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Definition of a number of widget utilities.
///
/// These functions apply a common look and feel to various widget types.
///
//=============================================================================

#ifndef RMV_UTIL_WIDGET_UTIL_H_
#define RMV_UTIL_WIDGET_UTIL_H_

#include "views/custom_widgets/rmv_range_slider.h"

#include <stdint.h>

#include <QGraphicsView>
#include <QMenu>
#include <QString>
#include <QTableView>
#include <QWidget>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/custom_widgets/colored_legend_scene.h"
#include "qt_common/custom_widgets/text_search_widget.h"

namespace rmv
{
    namespace widget_util
    {
        /// @brief Initialize a range slider widget.
        ///
        /// @param [in,out] range_slider            A pointer to the RmvRangeSlider widget to initialize.
        void InitRangeSlider(RmvRangeSlider* slider_widget);

        /// @brief Initialize an ArrowIconComboBox for single selection.
        ///
        /// @param [in]     parent              The combo box parent.
        /// @param [in,out] combo_box           The combo box to initialize.
        /// @param [in]     default_text        The default text string.
        /// @param [in]     retain_default_text If true, the combo box text doesn't change to the selected
        ///  item text (in the case of combo boxes requiring check boxes).
        /// @param [in]     prefix_text         The text used to prefix the default text.
        void InitSingleSelectComboBox(QWidget*           parent,
                                      ArrowIconComboBox* combo_box,
                                      const QString&     default_text,
                                      bool               retain_default_text,
                                      const QString      prefix_text = "");

        /// @brief Initialize an ArrowIconComboBox for multi selection.
        ///
        /// @param [in] parent         The combo box parent.
        /// @param [in, out] combo_box The combo box to initialize.
        /// @param [in] default_text   The default text string.
        void InitMultiSelectComboBox(QWidget* parent, ArrowIconComboBox* combo_box, const QString& default_text);

        /// @brief Initialize a graphics view to some common defaults.
        ///
        /// @param [in] view Pointer to the QGraphicsView.
        /// @param [in] fixed_height The maximum height.
        void InitGraphicsView(QGraphicsView* view, uint32_t fixed_height);

        /// @brief Apply standard styling for a given top level pane's scroll area.
        ///
        /// @param [in] scroll_area  The scroll area.
        void ApplyStandardPaneStyle(QScrollArea* scroll_area);

        /// @brief Init search box and double slider.
        ///
        /// @param [in,out] text_search_widget   Text search item.
        /// @param [in,out] double_slider_widget Double slider item.
        void InitCommonFilteringComponents(TextSearchWidget* text_search_widget, DoubleSliderWidget* double_slider_widget);

        /// @brief Method to initialize color legends.
        ///
        /// @param [in] view          The graphics view for this legend.
        ///
        /// @return A new scene for this legend.
        ColoredLegendScene* InitColorLegend(QGraphicsView* view);

        /// @brief Set a custom palette for tables.
        ///
        /// @param view The table view.
        void UpdateTablePalette(QTableView* view);

        /// @brief Get the height of a table depending on how many rows are in the table.
        ///
        /// Used to make the table as large as it needs to be so there are no empty rows.
        ///
        /// @param table_view The table view.
        /// @param row_count  The number of rows.
        int GetTableHeight(const QTableView* table_view, int row_count);

        /// @brief Template function to create a context menu for a resource list table and execute it.
        ///
        /// The model type is templated to allow different classes to be used for the model that share
        /// the same member function.
        ///
        template <typename ModelType>
        void CreateResourceTableContextMenu(QWidget* parent, const QTableView* table_view, const QPoint& pos, ModelType* model)
        {
            QModelIndex index = table_view->indexAt(pos);
            if (!index.isValid())
            {
                return;  // No item under the cursor.
            }

            QMenu    menu(parent);
            QAction* save_action = new QAction("Save resources to disk", parent);

            // Connect actions to slots.
            parent->connect(save_action, &QAction::triggered, parent, [=]() { model->DumpResourceTable(parent); });

            menu.addAction(save_action);

            // Map the position to global coordinates.
            QPoint global_pos = table_view->viewport()->mapToGlobal(pos);
            menu.exec(global_pos);

            // Clean up.
            delete save_action;
        }

    }  // namespace widget_util
}  // namespace rmv

#endif  // RMV_UTIL_WIDGET_UTIL_H_
