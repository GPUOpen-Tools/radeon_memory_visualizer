//=============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the MessageManager class. Basically a bunch of
/// signals that other UI elements can observe and react to
//=============================================================================

#include "models/message_manager.h"

// single instance of the Message Manager.
static MessageManager message_manager;

MessageManager& MessageManager::Get()
{
    return message_manager;
}
