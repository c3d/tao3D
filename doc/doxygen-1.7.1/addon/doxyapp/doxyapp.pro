#
# This file was generated from doxyapp.pro.in on Dim 13 fév 2011 16:48:29 CET
#

TEMPLATE     =	app.t
CONFIG       =	console warn_on debug
HEADERS      =	
SOURCES      =	doxyapp.cpp
LIBS          += -L../../lib -L../../lib -ldoxygen -lqtools -lmd5 -ldoxycfg -lpng
DESTDIR        = 
OBJECTS_DIR    = ../../objects
TARGET         = ../../bin/doxyapp
INCLUDEPATH   += ../../qtools ../../src
DEPENDPATH    += ../../src
TARGETDEPS     = ../../lib/libdoxygen.a

