//=============================================================================
// Copyright (c) 2018-2023 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Implementation of Keyboard Shortcuts pane.
//=============================================================================

#include "views/settings/keyboard_shortcuts_pane.h"

#include "util/widget_util.h"

KeyboardShortcutsPane::KeyboardShortcutsPane(QWidget* parent)
    : BasePane(parent)
    , ui_(new Ui::KeyboardShortcutsPane)
{
    ui_->setupUi(this);

    rmv::widget_util::ApplyStandardPaneStyle(this, ui_->main_content_, ui_->main_scroll_area_);
}

KeyboardShortcutsPane::~KeyboardShortcutsPane()
{
}
