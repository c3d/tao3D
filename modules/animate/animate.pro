# *****************************************************************************
#  animate.pro                                                     Tao project
# *****************************************************************************
# 
#   File Description:
# 
#     Qt configuration file for the 'animate' module
# 
# 
# 
# 
# 
# 
# 
# 
# *****************************************************************************
# This document is released under the GNU General Public License.
#  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
#  (C) 2010 Taodyne SAS
# *****************************************************************************

MODINSTDIR = animate

include(../modules.pri)

OTHER_FILES = module.xl module.doxy.h

# icon from http://www.iconfinder.com/icondetails/15430/32/keynote_presentation_stand_icon
INSTALLS    += thismod_icon pics
INSTALLS    -= thismod_bin
