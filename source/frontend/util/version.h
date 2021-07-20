//=============================================================================
// Copyright (c) 2018-2021 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief  Version number info for RMV
//=============================================================================

#ifndef RMV_VERSION_H
#define RMV_VERSION_H

#define STRINGIFY_VALUE(x) STRINGIFY(x)
#define STRINGIFY(x) #x

#define RMV_APP_NAME "Radeon Memory Visualizer"  ///< Application name.
#define RMV_BUILD_SUFFIX ""                      ///< The build suffix to apply to the product name.(alpha, beta etc.)

#define RMV_MAJOR_VERSION 1                 ///< major version number.
#define RMV_MINOR_VERSION 2                 ///< minor version number.
#define RMV_BUGFIX_NUMBER 0                 ///< bugfix number.
#define RMV_BUILD_NUMBER 0                  ///< build number.
#define RMV_BUILD_DATE_STRING "03/10/2021"  ///< build date string.
#define RMV_BUILD_CURRENT_YEAR 2021         ///< The current year.

#define RMV_COPYRIGHT_STRING "Copyright (C) 2018-" STRINGIFY_VALUE(RMV_BUILD_CURRENT_YEAR) " Advanced Micro Devices, Inc. All rights reserved."

#define RMV_VERSION_STRING             \
    STRINGIFY_VALUE(RMV_MAJOR_VERSION) \
    "." STRINGIFY_VALUE(RMV_MINOR_VERSION) "." STRINGIFY_VALUE(RMV_BUGFIX_NUMBER) "." STRINGIFY_VALUE(RMV_BUILD_NUMBER)  ///< The full revision string.

#endif  // RMV_VERSION_H
