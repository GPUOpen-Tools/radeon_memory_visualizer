#! python3
##=============================================================================
## Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
## \author AMD Developer Tools Team
## \file
## \brief List of all external dependencies.
##=============================================================================

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
    github_tools + "qt_common"                 : ["../external/qt_common",          "v4.4.0",  True],
    github_tools + "system_info_utils"         : ["../external/system_info_utils",  "v2.1",    True],
    github_tools + "update_check_api"          : ["../external/update_check_api",   "v2.1.1",  True],
    github_root  + "GPUOpen-Drivers/libamdrdf" : ["../external/rdf",                "v1.4.2",  True],
    github_root  + "nlohmann/json"             : ["../external/third_party/json",   "v3.9.1",  True],
    github_root  + "catchorg/Catch2"           : ["../external/third_party/catch2", "v2.13.6", True],
}
