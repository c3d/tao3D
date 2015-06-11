# Version: 3 integers >= 0
# MacOSX: the CFBundleVersion and CFBundleShortVersionString keys in Info.plist
# take the value of $$VERSION as is
# Windows: FILEVERSION, PRODUCTVERSION, and FileVersion are built using
# $$MAJOR, $$MINOR and $$RELEASE
TAO_VERSION = "2.0.0"
MAJOR = $$replace(TAO_VERSION, "\\.[0-9]+\\.[0-9]+\$", "")
MINOR = $$replace(TAO_VERSION, "^[0-9]+\\.", "")
MINOR = $$replace(MINOR, "\\.[0-9]+\$", "")
RELEASE = $$replace(TAO_VERSION, "^[0-9]+\\.[0-9]+\\.", "")

!isEmpty(TAO_PLAYER):MAYBE_PLAYER = " Player"
APP_NAME="Tao3D$$MAYBE_PLAYER"
