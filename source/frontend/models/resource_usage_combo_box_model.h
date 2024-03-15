//=============================================================================
// Copyright (c) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a model corresponding to a resource combo box.
//=============================================================================

#ifndef RMV_MODELS_RESOURCE_USAGE_COMBO_BOX_MODEL_H_
#define RMV_MODELS_RESOURCE_USAGE_COMBO_BOX_MODEL_H_

#include "models/combo_box_model.h"

#include "rmt_format.h"

namespace rmv
{
    /// @brief Model encapsulating everything needed for a resource combo box.
    class ResourceUsageComboBoxModel : public ComboBoxModel
    {
        Q_OBJECT

    public:
        /// @brief Constructor.
        ResourceUsageComboBoxModel();

        /// @brief Destructor.
        virtual ~ResourceUsageComboBoxModel();

        /// @brief Set up the resource combo box taking into account any resources that are to be ignored.
        ///
        /// @param [in] combo_box Pointer to the combo box to set up.
        void SetupResourceComboBox(ArrowIconComboBox* combo_box);

        /// @brief Reset the resource combo box to its default values. Some values may be disabled by default.
        ///
        /// @param [in] combo_box Pointer to the combo box to reset.
        void ResetResourceComboBox(ArrowIconComboBox* combo_box);

        /// @brief Get the Filter string for the regular expression to be used when filtering a resource list table by resource usahe.
        ///
        /// @param [in] combo_box Pointer to the combo box to query.
        ///
        /// @return The filter string.
        QString GetFilterString(const ArrowIconComboBox* combo_box);

        /// @brief Get the bit mask for the resource usage filter.
        ///
        /// @param [in] combo_box Pointer to the combo box to query.
        ///
        /// @return Resource usage bit mask.
        uint64_t GetFilterMask(const ArrowIconComboBox* combo_box);

        /// @brief Check if an item is in the list.
        ///
        /// @param [in] item The item to search for.
        ///
        /// @return true if item is found in the list, false otherwise.
        bool ItemInList(int item) const;

        /// @brief Check the state of the combo box and setup the internal state representation of the ArrowIconComboBox.
        ///
        /// @param [in] combo_box Pointer to the combo box whose state is to be examined.
        void SetupState(const ArrowIconComboBox* combo_box);

        /// @brief Update the full set of checkboxes in the combobox based on the checkbox was changed.
        ///
        /// @param [in] changed_item_index The index of the item in the combo box change was updated.
        /// @param [in] combo_box          A pointer to the combo box.
        void UpdateCheckboxes(int changed_item_index, ArrowIconComboBox* combo_box);

    signals:
        /// @brief Signal emitted when a combo box item is changed.
        ///
        /// @param [in] checked    The checked state of the filter.
        /// @param [in] item_index The index of the combo box item that changed.
        void FilterChanged(bool checked, int item_index);

    private:
        int                            heap_checkbox_item_index_;      ///< The combo box item index for the "Heap" checkbox.
        int                            all_checkbox_item_index_;       ///< The combo box item index for the "All" checkbox.
        std::set<RmtResourceUsageType> checked_resource_usage_types_;  ///< The list of checked resource usage types.
    };
}  // namespace rmv

#endif  // RMV_MODELS_RESOURCE_USAGE_COMBO_BOX_MODEL_H_
