#! /usr/bin/python
# Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
#
# Script to fetch all external git and/or downloable dependencies needed to build the project
#
#   fetch_dependencies.py (--internal)
#
# If --internal is specified, then any additional dependencies required for internal builds will also
# be checked out.
#
# Each git repo will be updated to the commit specified in the "gitMapping" table.

import os
import subprocess
import sys
import zipfile
import tarfile
import platform
import argparse

# Check for the python 3.x name and import it as the 2.x name
try:
    import urllib.request as urllib
# if that failed, then try the 2.x name
except ImportError:
    import urllib

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# also store the basename of the file
scriptName = os.path.basename(__file__)

# Print a message to the console with appropriate pre-amble
def logPrint(message):
    print ("\n" + scriptName + ": " + message)
    sys.stdout.flush()

# add script root to support import of URL and git maps
sys.path.append(scriptRoot)
from dependency_map import gitMapping

# Clone or update a git repo
def updateGitDependencies(gitMapping, update):
    for gitRepo in gitMapping:
        # add script directory to path
        tmppath = os.path.join(scriptRoot, gitMapping[gitRepo][0])

        # clean up path, collapsing any ../ and converting / to \ for Windows
        path = os.path.normpath(tmppath)

        # required commit
        reqdCommit = gitMapping[gitRepo][1]

        doCheckout = False
        if not os.path.isdir(path):
            # directory doesn't exist - clone from git
            logPrint("Directory %s does not exist, using 'git clone' to get latest from %s" % (path, gitRepo))
            p = subprocess.Popen((["git", "clone", gitRepo ,path]), stderr=subprocess.STDOUT)
            p.wait()
            if(p.returncode == 0):
                doCheckout = True
            else:
                logPrint("git clone failed with return code: %d" % p.returncode)
                return False
        elif update == True:
            # directory exists and update requested - get latest from git
            logPrint("Directory %s exists, using 'git fetch --tags' to get latest from %s" % (path, gitRepo))
            p = subprocess.Popen((["git", "fetch", "--tags"]), cwd=path, stderr=subprocess.STDOUT)
            p.wait()
            if(p.returncode == 0):
                doCheckout = True
            else:
                logPrint("git fetch failed with return code: %d" % p.returncode)
                return False
        else:
            # Directory exists and update not requested
            logPrint("Git Dependency %s found and not updated" % gitRepo)

        if doCheckout == True:
            logPrint("Checking out required commit: %s" % reqdCommit)
            p = subprocess.Popen((["git", "checkout", reqdCommit]), cwd=path, stderr=subprocess.STDOUT)
            p.wait()
            if(p.returncode != 0):
                logPrint("git checkout failed with return code: %d" % p.returncode)
                return False
            logPrint("Ensuring any branch is on the head using git pull --ff-only origin %s" % reqdCommit)
            p = subprocess.Popen((["git", "pull", "--ff-only", "origin", reqdCommit]), cwd=path, stderr=subprocess.STDOUT)
            p.wait()
            if(p.returncode != 0):
                logPrint("git merge failed with return code: %d" % p.returncode)
                return False

    return True

# Main body of update functionality
def doFetchDependencies(update, internal):
    # Print git version being used
    gitCmd = ["git", "--version"]
    gitOutput = subprocess.check_output(gitCmd, stderr=subprocess.STDOUT)
    logPrint("%s" % gitOutput)

    # Update all git dependencies
    if updateGitDependencies(gitMapping, update):
        return True
    else:
        return False

if __name__ == '__main__':
    # fetch_dependencies.py executed as a script

    # parse the command line arguments
    parser = argparse.ArgumentParser(description="A script that fetches all the necessary build dependencies for the project")
    parser.add_argument("--internal", action="store_true", help="fetch dependencies required for internal builds of the tool (only used within AMD")
    args = parser.parse_args()

    doFetchDependencies(True, args.internal)
