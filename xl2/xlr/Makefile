#******************************************************************************
# Makefile                          (C) 1992-2003 Christophe de Dinechin (ddd) 
#                                                                  XL2 project 
#******************************************************************************
#
#  File Description:
#
#    Makefile for the XL runtime environment
#
#
#
#
#
#
#
#
#******************************************************************************
# This program is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
#******************************************************************************
#* File       : $RCSFile$
#* Revision   : $Revision$
#* Date       : $Date$
#******************************************************************************
ifndef BUILDENV
BUILDENV=macosx
endif

OBJ_DIR=obj/$(BUILDENV)/

all:
	mkdir -p $(OBJ_DIR)
	cd $(OBJ_DIR) && /usr/bin/qmake ../../xlr.pro -spec macx-g++ -r && make -w

clean:
	cd $(OBJ_DIR) && make clean

distclean:
	cd $(OBJ_DIR) && make distclean

