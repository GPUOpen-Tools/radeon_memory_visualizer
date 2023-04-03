//=============================================================================
// Copyright (c) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a model corresponding to a resource combo box.
//=============================================================================

#include "models/resource_usage_combo_box_model.h"

#include <QCheckBox>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"

#include "rmt_resource_list.h"
#include "rmt_assert.h"
#include "rmt_print.h"

#include "util/string_util.h"

// Set of resources that shouldn't be in the resource combo box.
static const std::set<int> kExcludedResources = {kRmtResourceUsageTypeUnknown};

// Set of resources that should be disabled in the resource combo box.
static const std::set<int> kDisabledResources = {kRmtResourceUsageTypeHeap, kRmtResourceUsageTypeFree};

namespace rmv
{
    ResourceUsageComboBoxModel::ResourceUsageComboBoxModel()
        : ComboBoxModel()
    {
        // Inform the model which entries are ignored in the UI.
        SetupExcludeIndexList(kExcludedResources);
    }

    ResourceUsageComboBoxModel::~ResourceUsageComboBoxModel()
    {
    }

    void ResourceUsageComboBoxModel::SetupResourceComboBox(ArrowIconComboBox* combo_box)
    {
        // Add the "All" entry to the combo box.
        QCheckBox* checkbox = combo_box->AddCheckboxItem("All", QVariant(), false, true);
        RMT_ASSERT(checkbox != nullptr);
        if (checkbox != nullptr)
        {
            connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true); });
        }

        // Add resources if they are not excluded.
        for (int i = 0; i < static_cast<int>(kRmtResourceUsageTypeCount); i++)
        {
            auto it = kExcludedResources.find(i);
            if (it == kExcludedResources.end())
            {
                checkbox = combo_box->AddCheckboxItem(RmtGetResourceUsageTypeNameFromResourceUsageType(RmtResourceUsageType(i)), QVariant(), false, false);
                RMT_ASSERT(checkbox != nullptr);
                if (checkbox != nullptr)
                {
                    connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true); });
                }
            }
        }
        ResetResourceComboBox(combo_box);
    }

    void ResourceUsageComboBoxModel::ResetResourceComboBox(ArrowIconComboBox* combo_box)
    {
        // Index 0 is the "all" combo box entry.
        if (kDisabledResources.size() > 0)
        {
            // If there are disabled resources, then disable the "All" checkbox.
            int combo_box_row = 0;
            combo_box->SetChecked(combo_box_row, false);
            combo_box_row++;

            for (int i = 0; i < static_cast<int>(kRmtResourceUsageTypeCount); i++)
            {
                // If this resource is excluded, skip it.
                auto exclude_it = kExcludedResources.find(i);
                if (exclude_it != kExcludedResources.end())
                {
                    continue;
                }

                auto disabled_it = kDisabledResources.find(i);
                if (disabled_it != kDisabledResources.end())
                {
                    combo_box->SetChecked(combo_box_row, false);
                }
                else
                {
                    combo_box->SetChecked(combo_box_row, true);
                }
                combo_box_row++;
            }
        }
        SetupState(combo_box);
    }

    QString ResourceUsageComboBoxModel::GetFilterString(const ArrowIconComboBox* combo_box)
    {
        SetupState(combo_box);

        // Build the resource filter.
        QString resource_filter = QString("(=");
        for (int resource = 0; resource < kRmtResourceUsageTypeCount; resource++)
        {
            if (ItemInList(resource) == true)
            {
                resource_filter += "|" + QString(RmtGetResourceUsageTypeNameFromResourceUsageType(static_cast<RmtResourceUsageType>(resource)));
            }
        }
        resource_filter += ")";

        return resource_filter;
    }

    void ResourceUsageComboBoxModel::SetupState(const ArrowIconComboBox* combo_box)
    {
        ComboBoxModel::SetupState(combo_box, true);
    }

}  // namespace rmv