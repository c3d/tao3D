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

all clean depend distclean: Makefile.doxyapp 
	$(MAKE) -f Makefile.doxyapp $@

distclean: clean
	$(RM) -rf Makefile doxyapp.pro Makefile.doxyapp

tmake:
	$(ENV) $(PERL) $(TMAKE) doxyapp.pro >Makefile.doxyapp

strip:
	strip doxyapp

Makefile.doxyapp: doxyapp.pro
	$(ENV) $(PERL) $(TMAKE) doxyapp.pro >Makefile.doxyapp

install:
