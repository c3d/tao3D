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
# $Id: Makefile.in,v 1.4 2001/03/19 19:27:39 root Exp $
#
# Copyright (C) 1997-2010 by Dimitri van Heesch.
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation under the terms of the GNU General Public License is hereby 
# granted. No representations are made about the suitability of this software 
# for any purpose. It is provided "as is" without express or implied warranty.
# See the GNU General Public License for more details.
# 
# Documents produced by Doxygen are derivative works derived from the
# input used in their production; they are not affected by this license.
# 

all: Makefile.libdoxygen Makefile.libdoxycfg Makefile.doxygen Makefile.doxytag Makefile
	$(MAKE) -f Makefile.libdoxycfg PERL=$(PERL) $@
	$(MAKE) -f Makefile.libdoxygen PERL=$(PERL) $@
	$(MAKE) -f Makefile.doxygen    PERL=$(PERL) $@
	$(MAKE) -f Makefile.doxytag    PERL=$(PERL) $@

Makefile.libdoxygen: libdoxygen.pro libdoxygen.t 
	$(ENV) $(PERL) $(TMAKE) libdoxygen.pro >Makefile.libdoxygen

Makefile.libdoxycfg: libdoxycfg.pro libdoxycfg.t 
	$(ENV) $(PERL) $(TMAKE) libdoxycfg.pro >Makefile.libdoxycfg

Makefile.doxygen: doxygen.pro 
	$(ENV) $(PERL) $(TMAKE) doxygen.pro >Makefile.doxygen

Makefile.doxytag: doxytag.pro doxytag.t 
	$(ENV) $(PERL) $(TMAKE) doxytag.pro >Makefile.doxytag

tmake: 
	$(ENV) $(PERL) $(TMAKE) libdoxygen.pro >Makefile.libdoxygen
	$(ENV) $(PERL) $(TMAKE) libdoxycfg.pro >Makefile.libdoxycfg
	$(ENV) $(PERL) $(TMAKE) doxygen.pro >Makefile.doxygen
	$(ENV) $(PERL) $(TMAKE) doxytag.pro >Makefile.doxytag

clean: Makefile.libdoxygen Makefile.libdoxycfg Makefile.doxygen Makefile.doxytag 
	$(MAKE) -f Makefile.libdoxygen clean 
	$(MAKE) -f Makefile.libdoxycfg clean 
	$(MAKE) -f Makefile.doxygen    clean 
	$(MAKE) -f Makefile.doxytag    clean

distclean: clean
	-$(RM) scanner.cpp code.cpp config.cpp pre.cpp ce_lex.cpp \
               ce_parse.cpp ce_parse.h doxytag.cpp tag.cpp commentscan.cpp \
	       declinfo.cpp defargs.cpp commentcnv.cpp doctokenizer.cpp \
	       pycode.cpp pyscanner.cpp fortrancode.cpp fortranscanner.cpp \
	       vhdlscanner.cpp vhdlcode.cpp

FORCE:
