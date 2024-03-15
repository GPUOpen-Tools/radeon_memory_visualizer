//=============================================================================
// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Intended to hold globally-known definitions.
//=============================================================================

#ifndef RMV_UTIL_DEFINITIONS_H_
#define RMV_UTIL_DEFINITIONS_H_

#ifdef _WIN32
#pragma warning(disable : 4189)
#endif

#ifdef _DEBUG
#define RMV_DEBUG_WINDOW 1
#else
#define RMV_DEBUG_WINDOW 0
#endif

#include "util/constants.h"

#endif  // RMV_UTIL_DEFINITIONS_H_
