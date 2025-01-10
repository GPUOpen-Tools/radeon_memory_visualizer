//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a model corresponding to a heap combo box.
//=============================================================================

#ifndef RMV_MODELS_HEAP_COMBO_BOX_MODEL_H_
#define RMV_MODELS_HEAP_COMBO_BOX_MODEL_H_

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"

#include "combo_box_model.h"

namespace rmv
{
    /// @brief Model encapsulating everything needed for a heap combo box.
    class HeapComboBoxModel : public ComboBoxModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        HeapComboBoxModel();

        /// @brief Destructor.
        virtual ~HeapComboBoxModel();

        /// @brief Set up the heap combo box taking into account any resources that are to be ignored.
        ///
        /// @param [in] combo_box Pointer to the combo box to set up.
        void SetupHeapComboBox(ArrowIconComboBox* combo_box);

        /// @brief Reset the heap combo box to its default values.
        ///
        /// @param [in] combo_box Pointer to the combo box to reset.
        void ResetHeapComboBox(ArrowIconComboBox* combo_box);

        /// @brief Get the Filter string for the regular expression to be used when filtering a resource list table by heap.
        ///
        /// @param [in] combo_box Pointer to the combo box to query.
        QString GetFilterString(const ArrowIconComboBox* combo_box);

        /// @brief Check the state of the combo box and setup the internal state representation of the ArrowIconComboBox.
        ///
        /// @param [in] combo_box Pointer to the combo box whose state is to be examined.
        void SetupState(const ArrowIconComboBox* combo_box);

    signals:
        /// @brief Signal emitted when a combo box item is changed.
        ///
        /// @param [in] checked The checked state of the filter.
        void FilterChanged(bool checked);
    };
}  // namespace rmv

#endif  // RMV_MODELS_HEAP_COMBO_BOX_MODEL_H_
