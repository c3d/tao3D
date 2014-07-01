#!/bin/bash -x
#
# Convenience script to create the automated builds of Tao Presentations. 

# Fetch the current release tag (default Jenkins does not do it)
git fetch --tags

# Create a clean build each time we have a "true" release candidate
# Other releases have a git hash beginning with g, like 1.50-1-ge0db3af
REV="$(tao/updaterev.sh -n)"
(echo $REV | grep g) || make distclean

# Configure with the Jenkins input parameters
./configure -$BUILD_OPT $BUILD_VERSION VLC=$VLCPATH 

upload() {
    if [ "$BUILD_OPT" == "release" -a "$BUILD_VERSION" != "internal" ]; then
        UNAME=""
        ( echo $REV | grep g) || UNAME=`uname`
        case "$UNAME" in
            Darwin)
                scp packaging/macosx/Tao*.dmg www.taodyne.com:downloads/
                ;;
            MINGW*)
                scp packaging/win/Tao*.exe www.taodyne.com:downloads/
                ;;
            Linux*)
                scp packaging/linux/Tao*.deb packaging/linux/Tao*.tar.bz2 www.taodyne.com:downloads/
                ;;
        esac
    fi
}

# Build it and upload on server
make kit && upload
