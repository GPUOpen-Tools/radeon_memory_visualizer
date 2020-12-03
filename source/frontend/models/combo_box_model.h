//=============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for a model corresponding to a combo box.
/// This can be used for the ArrowIconComboBoxes when in the checkbox mode,
/// where it's possible to have more than 1 entry selected.
//=============================================================================

#ifndef RMV_MODELS_COMBO_BOX_MODEL_H_
#define RMV_MODELS_COMBO_BOX_MODEL_H_

#include <QObject>
#include <set>
#include <vector>

#include "qt_common/custom_widgets/arrow_icon_combo_box.h"

namespace rmv
{
    class ComboBoxModel : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        ComboBoxModel();

        /// Destructor.
        ~ComboBoxModel();

        /// Set up the list of excluded items. These are items from an enum list that are
        /// not shown in the UI so the model also needs to know they are excluded.
        /// \param indices This set of indices to exclude.
        void SetupExcludeIndexList(const std::set<int>& indices);

        /// Is an item in the list.
        /// \param item The item to search for.
        /// \return true if so, false otherwise.
        bool ItemInList(int item) const;

    protected:
        /// Check the state of the combo box and setup the internal state representation
        /// of the ArrowIconComboBox.
        /// \param combo_box Pointer to the combo box whose state is to be examined.
        /// \param all_option True if the combo box contains an "All" option.
        void SetupState(const ArrowIconComboBox* combo_box, bool all_option);

    private:
        std::set<int>    checked_items_list_;  ///< The list of checked items.
        std::vector<int> excluded_list_;       ///< Ths list of excluded itenms.
    };
}  // namespace rmv

#endif  // RMV_MODELS_COMBO_BOX_MODEL_H_
