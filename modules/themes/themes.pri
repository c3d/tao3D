# ******************************************************************************
# themes.pri                                                       Tao3D project
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
# (C) 2012-2013,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2013-2014, Jérôme Forissier <jerome@taodyne.com>
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
include(../modules.pri)

!contains(templates, none) {
  exists($$_PRO_FILE_PWD_/theme.xl):MAYBE_THEME_XL=theme.xl
  exists($$_PRO_FILE_PWD_/sample_slide.json) {
    MAYBE_JSON = sample_slide.json

    # Emit rules to generate .ddd from .json and install it
    sample_slide.ddd.path = $$APPINST/templates/$$MODINSTDIR
    sample_slide.ddd.commands = $$TAOTOPSRC/webui/webui/server.sh -c sample_slide.ddd
    sample_slide.ddd.files = sample_slide.ddd
    sample_slide.ddd.depends = sample_slide.json
    sample_slide.ddd.CONFIG = no_check_exist
    QMAKE_EXTRA_TARGETS *= sample_slide.ddd
    INSTALLS *= sample_slide.ddd
    QMAKE_CLEAN *= sample_slide.ddd
  }
  exists($$_PRO_FILE_PWD_/sample_slide_resources.json):MAYBE_JSON2=sample_slide_resources.json

  thismod_template.path = $$APPINST/templates/$$MODINSTDIR
  thismod_template.files = template.ini thumbnail.png $$MAYBE_THEME_XL sample_slide.ddd $$MAYBE_JSON $$MAYBE_JSON2

  INSTALLS += thismod_images thismod_template

  SIGN_XL_SOURCES = $$MAYBE_THEME_XL sample_slide.ddd
  SIGN_XL_INSTPATH=$$APPINST/templates/$$MODINSTDIR
  include(../sign_xl.pri)
}
