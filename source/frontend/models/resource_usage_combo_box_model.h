//=============================================================================
// Copyright (c) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Header for a model corresponding to a resource combo box.
//=============================================================================

#ifndef RMV_MODELS_RESOURCE_USAGE_COMBO_BOX_MODEL_H_
#define RMV_MODELS_RESOURCE_USAGE_COMBO_BOX_MODEL_H_

#include "models/combo_box_model.h"

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

#endif  // RMV_MODELS_RESOURCE_USAGE_COMBO_BOX_MODEL_H_
