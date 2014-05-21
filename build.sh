#!/bin/bash -x
#
# Convenience script to create the automated builds of Tao Presentations. 

# Fetch the current release tag (default Jenkins does not do it)
git fetch --tags

# Create a clean build each time we have a "true" release candidate
(tao/updaterev.sh -n | grep [dg]) || make distclean

# Configure with the Jenkins input parameters
./configure -$BUILD_OPT $BUILD_VERSION VLC=$VLCPATH 

# Build it
make kit
