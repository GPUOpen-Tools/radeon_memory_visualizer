#!/usr/bin/python
# Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.

import sys

# prevent generation of .pyc file
sys.dont_write_bytecode = True

####### Git Dependencies #######

# To allow for future updates where we may have cloned the project somewhere other than gerrit, store the root of
# the repo in a variable. In future, we can automatically calculate this based on the git config
gitRoot = "https://github.com/GPUOpen-Tools/"

# Define a set of dependencies that exist as separate git projects.
# each git dependency has a desired directory where it will be cloned - along with a commit to checkout
gitMapping = {
    gitRoot + "QtCommon"                : ["../external/qt_common",        "v3.6.0"],
    gitRoot + "UpdateCheckAPI"          : ["../external/update_check_api", "v2.0.0"],
}

