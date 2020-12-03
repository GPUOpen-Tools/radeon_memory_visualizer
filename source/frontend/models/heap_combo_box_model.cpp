//=============================================================================
/// Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for a model corresponding to a heap combo box.
//=============================================================================

#include "models/heap_combo_box_model.h"

#include <QCheckBox>

#include "rmt_print.h"
#include "rmt_assert.h"

namespace rmv
{
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

        // Add the "All" entry to the combo box
        QCheckBox* checkbox = combo_box->AddCheckboxItem("All", QVariant(), false, true);
        RMT_ASSERT(checkbox != nullptr);
        if (checkbox != nullptr)
        {
            connect(checkbox, &QCheckBox::clicked, this, [=]() { emit FilterChanged(true); });
        }

        for (uint32_t i = 0; i < (uint32_t)kRmtHeapTypeCount; i++)
        {
            checkbox = combo_box->AddCheckboxItem(RmtGetHeapTypeNameFromHeapType(RmtHeapType(i)), QVariant(), false, false);
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
        // index 0 is the "all" combo box entry
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

        // build the heap filter
        QString heap_filter = QString("(-|ORPHANED");
        for (int heap = 0; heap < kRmtHeapTypeCount; heap++)
        {
            if (ItemInList(heap) == true)
            {
                heap_filter += "|" + QString(RmtGetHeapTypeNameFromHeapType((RmtHeapType)heap));
            }
        }
        heap_filter += ")";

        return heap_filter;
    }

    void HeapComboBoxModel::SetupState(const ArrowIconComboBox* combo_box)
    {
        ComboBoxModel::SetupState(combo_box, true);
    }
}  // namespace rmv