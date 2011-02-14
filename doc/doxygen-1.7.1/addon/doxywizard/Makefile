#
# This file was generated from Makefile.in on Dim 13 fév 2011 16:48:28 CET
#

DOXYGEN   = /Users/cathy/Work/tao/doc/doxygen-1.7.1
TMAKEPATH = /Users/cathy/Work/tao/doc/doxygen-1.7.1/tmake/lib/macosx-c++
ENV       = env TMAKEPATH=$(TMAKEPATH)
TMAKE     = /Users/cathy/Work/tao/doc/doxygen-1.7.1/tmake/bin/tmake
MAKE      = /usr/bin/make
PERL      = /opt/local/bin/perl
RM        = rm -f
CP        = cp
VERSION   = 1.7.1
INSTALL   = /usr/local
INSTTOOL  = /usr/bin/install
DOXYDOCS  = ..
DOCDIR    = $(INSTALL)/share/doc/packages/doxygen
QTDIR     = 
MKSPECS   = -spec macx-g++
#
# 
#
# Copyright (C) 1997-2008 by Dimitri van Heesch.
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation under the terms of the GNU General Public License is hereby 
# granted. No representations are made about the suitability of this software 
# for any purpose. It is provided "as is" without express or implied warranty.
# See the GNU General Public License for more details.
# 

QMAKE=qmake $(MKSPECS)

all: Makefile.doxywizard  
	$(MAKE) -f Makefile.doxywizard 

Makefile.doxywizard: doxywizard.pro
	$(QMAKE) doxywizard.pro -o Makefile.doxywizard

qmake: 
	$(QMAKE) doxywizard.pro -o Makefile.doxywizard

clean: Makefile.doxywizard 
	$(MAKE) -f Makefile.doxywizard clean 

distclean: Makefile.doxywizard
	$(MAKE) -f Makefile.doxywizard distclean 
	$(RM) Makefile.doxywizard

install:
	$(INSTTOOL) -d $(INSTALL)/bin	
	$(INSTTOOL) -m 755 ../../bin/doxywizard $(INSTALL)/bin	
	$(INSTTOOL) -d $(INSTALL)/$(MAN1DIR)
	cat ../../doc/doxywizard.1 | sed -e "s/DATE/$(DATE)/g" -e "s/VERSION/$(VERSION)/g" > doxywizard.1
	$(INSTTOOL) -m 644 doxywizard.1 $(INSTALL)/$(MAN1DIR)/doxywizard.1
	rm doxywizard.1

FORCE:
