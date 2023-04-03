#! python3
# Copyright (c) 2020-2023 Advanced Micro Devices, Inc. All rights reserved.

import sys

# prevent generation of .pyc file
sys.dont_write_bytecode = True

####### Git Dependencies #######

# To allow for future updates where we may have cloned the project somewhere other than gerrit, store the root of
# the repo in a variable. In future, we can automatically calculate this based on the git config
github_tools = "https://github.com/GPUOpen-Tools/"
github_root  = "https://github.com/"

# Define a set of dependencies that exist as separate git projects.
# each git dependency has a desired directory where it will be cloned - along with a commit to checkout
git_mapping = {
    github_tools    + "QtCommon"                              : ["../external/qt_common",         "v3.8.0"],
    github_tools    + "UpdateCheckApi"                        : ["../external/update_check_api",  "v2.0.1"],
    github_tools    + "system_info_utils"                     : ["../external/system_info_utils", "88a338a01949f8d8bad60a30b78b65300fd13a9f"],
    github_root     + "GPUOpen-Drivers/libamdrdf"             : ["../external/rdf",               "v1.1.2"],
}
