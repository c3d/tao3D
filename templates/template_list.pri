# ******************************************************************************
# template_list.pri                                                Tao3D project
# ******************************************************************************
#
# File description:
#
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# (C) 2011-2013, Catherine Burvelle <catherine@taodyne.com>
# (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
# (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# (C) 2011, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tao3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tao3D, in a file named COPYING.
# If not, see <https://www.gnu.org/licenses/>.
# ******************************************************************************
# Defines the list of templates to install.
# Defaults can be changed with "qmake templates=..." (see below)

# Default template list

DEFAULT_TEMPLATES =   \
    blank \
    hello_world \
    mandelbrot \
    photo_viewer \
    lens_flare \
    movie \
    materials \
    pan_and_zoom \
    picture_slider \
    shift \
    simple_slides \
    opening_pane \
    rising_text \
    mobile \
    newsfeed \
    water_en_fr \
    charts

OTHER_TEMPLATES =     \
    mapping \
    shading \
    filters \
    pigs_fly \
    guess_game \
    pythagorean_theorem \
    balls \
    monjori \
    flares \
    sinuous \
    sparks \
    glow \
    YouCube \
    quiz

# Process qmake command line variable: 'templates'
# Allows to add/remove templates to/from the default value above
# Resulting list is stored in TEMPLATES
#   templates=none
#     Do not install any Tao template
#   templates=all
#     Build all Tao modules ($$DEFAULT_TEMPLATES and $$OTHER_TEMPLATES)
#   templates=+my_template
#     Add my_template to default template list
#   templates=-my_template
#     Remove my_template from default template list
#   templates="all -my_template"
#     Build all templates except my_template
#   templates="none +my_template"
#     Build only my_template
TEMPLATES = $$DEFAULT_TEMPLATES
!isEmpty(templates) {
    options = $$templates
    for(opt, options) {
        equals(opt, "all") {
            TEMPLATES = $$DEFAULT_TEMPLATES $$OTHER_TEMPLATES
        }
        equals(opt, "none") {
            TEMPLATES =
        }
        remove = $$find(opt, ^-.*)
        !isEmpty(remove) {
            remove ~= s/-//
            TEMPLATES -= $$remove
        }
        add = $$find(opt, ^\\+.*)
        !isEmpty(add) {
            add ~= s/\\+//
            exists($$add) {
                TEMPLATES += $$add
            } else {
                warning(Template directory not found: $$add -- not added)
            }
        }
    }
}

# Some templates depend on module availability
include (../modules/module_list.pri)
!contains (MODULES, slides):TEMPLATES -= simple_slides
!contains (MODULES, object_loader):TEMPLATES -= pigs_fly
!contains (MODULES, tao_visuals):TEMPLATES -= pigs_fly
!contains (MODULES, slideshow_3d):TEMPLATES -= photo_viewer
!contains (MODULES, lens_flare):TEMPLATES -= lens_flare
!contains (MODULES, filters):TEMPLATES -= filters
!contains (MODULES, shading):TEMPLATES -= shading
!contains (MODULES, movie_credits):TEMPLATES -= movie
!contains (MODULES, mapping):TEMPLATES -= mapping
!contains (MODULES, pan_and_zoom):TEMPLATES -= pan_and_zoom
!contains (MODULES, shaders/monjori):TEMPLATES -= monjori
!contains (MODULES, shaders/flares):TEMPLATES -= flares
!contains (MODULES, vlc_audio_video):TEMPLATES -= YouCube
!contains (MODULES, themes/water):TEMPLATES -= water_en_fr
