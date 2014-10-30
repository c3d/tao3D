# ******************************************************************************
#  hello_world.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Hello World module
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = hello_world

include(../modules.pri)

OTHER_FILES = hello_world.xl

INSTALLS    -= thismod_bin
