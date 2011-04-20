# ******************************************************************************
#  mandelbrot.pro                                                   Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Mandelbrot 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/mandelbrot
files.files = template.ini mandelbrot.ddd mandelbrot.png

INSTALLS += files
