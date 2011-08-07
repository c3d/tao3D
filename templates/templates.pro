# ******************************************************************************
#  templates.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file for Tao templates
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

# "make install" will copy the templates to the staging directory

TEMPLATE = subdirs
SUBDIRS  = blank pythagorean_theorem hello_world guess_game mandelbrot custom_slides

# Some templates depend on module availability
include (../modules/module_list.pri)
contains (MODULES, slides):SUBDIRS += simple_slides
contains (MODULES, object_loader):contains(MODULES, tao_visuals):SUBDIRS += pigs_fly
contains (MODULES, slideshow_3d):SUBDIRS += photo_viewer
contains (MODULES, lens_flare):SUBDIRS += lens_flare
contains (MODULES, filters):SUBDIRS += filters
contains (MODULES, shading):SUBDIRS += shading
contains (MODULES, movie_credits):SUBDIRS += movie
contains (MODULES, mapping):SUBDIRS += mapping
contains (MODULES, pan_and_zoom):SUBDIRS += pan_and_zoom

message(Templates to install: $$SUBDIRS)
