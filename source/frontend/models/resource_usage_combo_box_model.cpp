//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a model corresponding to a resource combo box.
//=============================================================================

#include "models/resource_usage_combo_box_model.h"

#include <cmath>

#include <QCheckBox>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"

#include "rmt_assert.h"
#include "rmt_print.h"
#include "rmt_resource_list.h"

#include "util/string_util.h"

// Set of resources that shouldn't be in the resource combo box or are specially managed.
static const std::set<int> kExcludedResources = {RmtResourceUsageType::kRmtResourceUsageTypeUnknown,
                                                 RmtResourceUsageType::kRmtResourceUsageTypeHeap,
                                                 RmtResourceUsageType::kRmtResourceUsageTypeAll};

// Set of resources that should be unchecked by default in the resource combo box.
static const std::set<int> kDefaultUncheckedResources = {kRmtResourceUsageTypeHeap, kRmtResourceUsageTypeFree};

// Indentation string for checkbox labels.
static const QString kCheckboxIndentationString("> ");

// Indicates an invalid index for the combobox.
static const int kInvalidIndex = -1;

namespace rmv
{
    ResourceUsageComboBoxModel::ResourceUsageComboBoxModel()
        : ComboBoxModel()
        , heap_checkbox_item_index_(kInvalidIndex)
        , all_checkbox_item_index_(kInvalidIndex)
        , default_unchecked_resources_(&kDefaultUncheckedResources)
    {
        // Inform the model which entries are ignored in the UI.
        SetupExcludeIndexList(kExcludedResources);
    }

    ResourceUsageComboBoxModel::ResourceUsageComboBoxModel(const std::set<int>* default_unchecked_resources)
        : ComboBoxModel()
        , heap_checkbox_item_index_(kInvalidIndex)
        , all_checkbox_item_index_(kInvalidIndex)
        , default_unchecked_resources_(default_unchecked_resources)
    {
        // Inform the model which entries are ignored in the UI.
        SetupExcludeIndexList(kExcludedResources);
    }

    ResourceUsageComboBoxModel::~ResourceUsageComboBoxModel()
    {
    }

    void ResourceUsageComboBoxModel::SetupResourceComboBox(ArrowIconComboBox* combo_box, const bool include_heap_checkbox)
    {
        QCheckBox* checkbox = nullptr;
        if (include_heap_checkbox)
        {
            // Add the "Heap" checkbox to the combo box.
            heap_checkbox_item_index_ = combo_box->RowCount();
            checkbox = combo_box->AddCheckboxItem(RmtGetResourceUsageTypeNameFromResourceUsageType(RmtResourceUsageType::kRmtResourceUsageTypeHeap),
                                                  RmtResourceUsageType::kRmtResourceUsageTypeHeap,
                                                  false,
                                                  false);
            RMT_ASSERT(checkbox != nullptr);
            if (checkbox != nullptr)
            {
                connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true, heap_checkbox_item_index_); });
            }
        }

        // Add the "All" checkbox to the combo box.
        all_checkbox_item_index_ = combo_box->RowCount();
        checkbox                 = combo_box->AddCheckboxItem("All resource usage types", RmtResourceUsageType::kRmtResourceUsageTypeCount, false, false);
        RMT_ASSERT(checkbox != nullptr);
        if (checkbox != nullptr)
        {
            connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true, all_checkbox_item_index_); });
        }

        // Add resources if they are not excluded.
        for (int i = 0; i < static_cast<int>(RmtResourceUsageType::kRmtResourceUsageTypeCount); i++)
        {
            const auto it = kExcludedResources.find(i);
            if (it == kExcludedResources.end())
            {
                checkbox = combo_box->AddCheckboxItem(
                    kCheckboxIndentationString + RmtGetResourceUsageTypeNameFromResourceUsageType(RmtResourceUsageType(i)), i, false, false);
                RMT_ASSERT(checkbox != nullptr);
                if (checkbox != nullptr)
                {
                    const int item_index = combo_box->RowCount() - 1;
                    connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true, item_index); });
                }
            }
        }
        ResetResourceComboBox(combo_box);
    }

    void ResourceUsageComboBoxModel::ResetResourceComboBox(ArrowIconComboBox* combo_box)
    {
        bool      found_unchecked_item = false;
        const int item_count           = combo_box->RowCount();
        for (int i = 0; i < item_count; i++)
        {
            // If this is the heap checkbox, uncheck it.
            if (i == heap_checkbox_item_index_)
            {
                combo_box->SetChecked(i, false);
                continue;
            }

            const RmtResourceUsageType usage_type           = static_cast<RmtResourceUsageType>(combo_box->ItemData(i).toInt());
            const auto                 default_unchecked_it = default_unchecked_resources_->find(usage_type);
            if (default_unchecked_it != default_unchecked_resources_->end())
            {
                combo_box->SetChecked(i, false);
                if (i != heap_checkbox_item_index_)
                {
                    // The heap checkbox is a special case, only set the 'found_unchecked_item' flag if any usage types are unchecked.
                    found_unchecked_item = true;
                }
            }
            else
            {
                combo_box->SetChecked(i, true);
            }

            // If this resource is excluded, skip it.
            const auto exclude_it = kExcludedResources.find(usage_type);
            if (exclude_it != kExcludedResources.end())
            {
                continue;
            }

            // If all items are checked, check the "All" checkbox.
            combo_box->SetChecked(all_checkbox_item_index_, !found_unchecked_item);
        }
        SetupState(combo_box);
    }

    QString ResourceUsageComboBoxModel::GetFilterString(const ArrowIconComboBox* combo_box)
    {
        SetupState(combo_box);

        // Build the resource filter.
        QString resource_filter = QString("(=");

        for (auto item_usage_type : checked_resource_usage_types_)
        {
            resource_filter += "|" + QString(RmtGetResourceUsageTypeNameFromResourceUsageType(item_usage_type));
        }
        resource_filter += ")";

        return resource_filter;
    }

    uint64_t ResourceUsageComboBoxModel::GetFilterMask(const ArrowIconComboBox* combo_box)
    {
        RMT_ASSERT(combo_box != nullptr);

        SetupState(combo_box);

        uint64_t filter_mask = 0;

        if ((all_checkbox_item_index_ != kInvalidIndex) && (combo_box->IsChecked(all_checkbox_item_index_)))
        {
            // Set bit mask for all usage types to true.
            filter_mask = kRmtResourceUsageTypeBitMaskAll;
        }
        else
        {
            const int item_count = combo_box->RowCount();
            for (int i = 0; i < item_count; i++)
            {
                if (i == all_checkbox_item_index_)
                {
                    continue;
                }

                const RmtResourceUsageType usage_type = static_cast<RmtResourceUsageType>(combo_box->ItemData(i).toInt());
                if (combo_box->IsChecked(i))
                {
                    filter_mask |= static_cast<uint64_t>(1) << usage_type;
                }
            }
        }

        return filter_mask;
    }

    bool ResourceUsageComboBoxModel::ItemInList(int usage_type) const
    {
        bool result = false;

        for (auto item_usage_type : checked_resource_usage_types_)
        {
            if (item_usage_type == usage_type)
            {
                result = true;
                break;
            }
        }

        return result;
    }

    void ResourceUsageComboBoxModel::SetupState(const ArrowIconComboBox* combo_box)
    {
        checked_resource_usage_types_.clear();

        bool all_usage_types_checked = false;
        if ((all_checkbox_item_index_ != kInvalidIndex) && (combo_box->IsChecked(all_checkbox_item_index_)))
        {
            all_usage_types_checked = true;
        }

        if ((heap_checkbox_item_index_ != kInvalidIndex) && (combo_box->IsChecked(heap_checkbox_item_index_)))
        {
            checked_resource_usage_types_.insert(RmtResourceUsageType::kRmtResourceUsageTypeHeap);
        }
        else
        {
            const int item_count = combo_box->RowCount();
            for (int i = 0; i < item_count; i++)
            {
                if (i == all_checkbox_item_index_)
                {
                    continue;
                }

                if (i == heap_checkbox_item_index_)
                {
                    continue;
                }

                const RmtResourceUsageType usage_type = static_cast<RmtResourceUsageType>(combo_box->ItemData(i).toInt());
                if ((all_usage_types_checked) || (combo_box->IsChecked(i)))
                {
                    checked_resource_usage_types_.insert(usage_type);
                }
            }
        }
    }

    void ResourceUsageComboBoxModel::UpdateCheckboxes(const int changed_item_index, ArrowIconComboBox* combo_box)
    {
        const int item_count = combo_box->RowCount();

        if (changed_item_index == all_checkbox_item_index_)
        {
            // Handle case where "All" checkbox changed.  Uncheck Heap and either check or uncheck all other checkbox items.
            const bool all_usage_types_checked = combo_box->IsChecked(changed_item_index);

            for (int index = 0; index < item_count; index++)
            {
                if (index == changed_item_index)
                {
                    continue;
                }

                RmtResourceUsageType item_resource_usage_type = static_cast<RmtResourceUsageType>(combo_box->ItemData(index).toInt());

                if ((all_usage_types_checked) && (item_resource_usage_type == RmtResourceUsageType::kRmtResourceUsageTypeHeap))
                {
                    combo_box->SetChecked(index, false);
                }
                else
                {
                    // Handle all other checkbox items.
                    combo_box->SetChecked(index, all_usage_types_checked);
                }
            }
        }
        else
        {
            // Uncheck "All" checkbox item.
            combo_box->SetChecked(all_checkbox_item_index_, false);

            if (changed_item_index == heap_checkbox_item_index_)
            {
                if (combo_box->IsChecked(changed_item_index))
                {
                    // Handle case where "Heap" checked.  Uncheck all other items.
                    for (int index = 0; index < item_count; index++)
                    {
                        if (index == heap_checkbox_item_index_)  // skip heap checkbox.
                        {
                            continue;
                        }

                        combo_box->SetChecked(index, false);
                    }
                }
            }
            else
            {
                // A usage type checkbox was checked so uncheck the "Heap" checkbox.
                if (heap_checkbox_item_index_ != kInvalidIndex)
                {
                    combo_box->SetChecked(heap_checkbox_item_index_, false);
                }

                // If all usage types are checked, also check the "All" checkbox.  Otherwise, uncheck the "All" checkbox.
                bool all_usage_types_checked = true;
                for (int index = 0; index < item_count; index++)
                {
                    if (index == heap_checkbox_item_index_)
                    {
                        continue;
                    }

                    if (index == all_checkbox_item_index_)
                    {
                        continue;
                    }

                    all_usage_types_checked &= combo_box->IsChecked(index);
                }
                combo_box->SetChecked(all_checkbox_item_index_, all_usage_types_checked);
            }
        }
    }
}  // namespace rmv
