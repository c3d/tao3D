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
# Documents produced by Doxygen are derivative works derived from the
# input used in their production; they are not affected by this license.

all: language FORCE
	DOXYGEN_DOCDIR=$(DOXYDOCS); \
	export DOXYGEN_DOCDIR; \
        VERSION=$(VERSION) ; \
	export VERSION; \
	$(DOXYGEN)/bin/doxygen
	@rm -f ../latex/refman.tex
	@cp doxygen_logo*.gif ../html
	@cp Makefile.latex ../latex/Makefile
	@sed -e "s/\$$VERSION/$(VERSION)/g" doxygen_manual.tex >../latex/doxygen_manual.tex
	@sed -e "s/\$$VERSION/$(VERSION)/g" doxygen.sty >../latex/doxygen.sty 
	@epstopdf doxygen_logo.eps --outfile=../latex/doxygen_logo.pdf

clean: 
	rm -rf ../html ../latex *.bak 

language: language.doc 

language.doc: $(wildcard ../src/translator*.h) maintainers.txt language.tpl translator.py
	python translator.py

FORCE:
