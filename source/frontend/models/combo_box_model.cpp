//=============================================================================
// Copyright (c) 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a model corresponding to a combo box.
///
/// This can be used for the ArrowIconComboBoxes when in the checkbox mode,
/// where it's possible to have more than 1 entry selected.
///
//=============================================================================

#include "models/combo_box_model.h"

namespace rmv
{
    ComboBoxModel::ComboBoxModel()
    {
    }

    ComboBoxModel::~ComboBoxModel()
    {
    }

    void ComboBoxModel::SetupExcludeIndexList(const std::set<int>& indices)
    {
        excluded_list_.clear();

        for (auto it = indices.begin(); it != indices.end(); ++it)
        {
            excluded_list_.push_back(*it);
        }
    }

    void ComboBoxModel::SetupState(const ArrowIconComboBox* combo_box, bool all_option)
    {
        // Skip the first entry in the list if it is "All" or something similar.
        int start_index = (!all_option) ? 0 : 1;

        // Add all of the checked items to the list. This will be their index
        // in the combo box.
        checked_items_list_.clear();
        for (int i = start_index; i < combo_box->RowCount(); i++)
        {
            if (combo_box->IsChecked(i) == true)
            {
                checked_items_list_.insert(i - start_index);
            }
        }
    }

    bool ComboBoxModel::ItemInList(int item) const
    {
        int count = 0;
        for (auto exclude_it = excluded_list_.begin(); exclude_it != excluded_list_.end(); ++exclude_it)
        {
            // If the item is in the excluded list, then item isn't valid.
            if (item == (*exclude_it))
            {
                return false;
            }

            // If the item is more than the current entry in the excluded list, adjust the item to take into
            // account it's missing from the UI (it's index in the UI will now be 1 less than the enum value
            // passed in.
            if (item > (*exclude_it))
            {
                count++;
            }
        }

        auto it = checked_items_list_.find(item - count);
        if (it != checked_items_list_.end())
        {
            return true;
        }
        return false;
    }
}  // namespace rmv
