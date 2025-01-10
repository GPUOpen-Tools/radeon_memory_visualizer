//=============================================================================
// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation for a model corresponding to a heap combo box.
//=============================================================================

#include "models/heap_combo_box_model.h"

#include <QCheckBox>

#include "rmt_print.h"
#include "rmt_assert.h"

namespace rmv
{
    // Heap combo box string for "Other" filter option.
    const QString kOtherHeapTypeString("Other");

    // Regular expression text used for "Other" heap type filter option.
    // For heaps reported as "none" by the driver, the RMV Backend may change the heap type to a custom string."
    // The custom types are also included for the "other" heap type filtering.
    const QString kUnspecifiedHeapTypeFilterString("-|Orphaned|Detached|Unspecified|Unknown");

    HeapComboBoxModel::HeapComboBoxModel()
        : ComboBoxModel()
    {
    }

    HeapComboBoxModel::~HeapComboBoxModel()
    {
    }

    void HeapComboBoxModel::SetupHeapComboBox(ArrowIconComboBox* combo_box)
    {
        combo_box->ClearItems();

        // Add the "All" entry to the combo box.
        QCheckBox* checkbox = combo_box->AddCheckboxItem("All", QVariant(), false, true);
        RMT_ASSERT(checkbox != nullptr);
        if (checkbox != nullptr)
        {
            connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true); });
        }

        for (uint32_t i = 0; i < (uint32_t)kRmtHeapTypeCount; i++)
        {
            if (i == RmtHeapType::kRmtHeapTypeNone)
            {
                checkbox = combo_box->AddCheckboxItem(kOtherHeapTypeString, QVariant(), false, false);
            }
            else
            {
                checkbox = combo_box->AddCheckboxItem(RmtGetHeapTypeNameFromHeapType(RmtHeapType(i)), QVariant(), false, false);
            }

            RMT_ASSERT(checkbox != nullptr);
            if (checkbox != nullptr)
            {
                connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true); });
            }
        }
        ResetHeapComboBox(combo_box);
    }

    void HeapComboBoxModel::ResetHeapComboBox(ArrowIconComboBox* combo_box)
    {
        // Index 0 is the "all" combo box entry.
        int combo_box_row = 0;
        combo_box->SetChecked(combo_box_row, true);
        combo_box_row++;

        for (uint32_t i = 0; i < (uint32_t)kRmtHeapTypeCount; i++)
        {
            combo_box->SetChecked(combo_box_row, true);
            combo_box_row++;
        }
        SetupState(combo_box);
    }

    QString HeapComboBoxModel::GetFilterString(const ArrowIconComboBox* combo_box)
    {
        SetupState(combo_box);

        // The number of selected filter types.
        int filter_count = 0;

        // Build the heap filter.
        QString heap_filter = QString("(");
        for (int heap = 0; heap < RmtHeapType::kRmtHeapTypeCount; heap++)
        {
            if (ItemInList(heap) == true)
            {
                if (filter_count > 0)
                {
                    // Add a bar between the filter types if this isn't the first selected item.
                    heap_filter += "|";
                }

                if (heap == RmtHeapType::kRmtHeapTypeNone)
                {
                    // Handle special case for unspecified heap type.
                    heap_filter += kUnspecifiedHeapTypeFilterString;
                }
                else
                {
                    heap_filter += QString(RmtGetHeapTypeNameFromHeapType(static_cast<RmtHeapType>(heap)));
                }
                filter_count++;
            }
        }

        heap_filter += ")";

        if (filter_count == 0)
        {
            // No heap types selected.  Use an empty RegEx character set to filter all rows.
            heap_filter = "([])";
        }

        return heap_filter;
    }

    void HeapComboBoxModel::SetupState(const ArrowIconComboBox* combo_box)
    {
        ComboBoxModel::SetupState(combo_box, true);
    }
}  // namespace rmv
