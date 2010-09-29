# ******************************************************************************
#  main.pro                                                         Tao project
# ******************************************************************************
# File Description:
# Main Qt build file for Tao
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


TEMPLATE = subdirs
SUBDIRS  = libxlr tao ssh_ask_pass
win32:SUBDIRS += detach
