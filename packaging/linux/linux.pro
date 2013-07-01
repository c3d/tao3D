# ******************************************************************************
# linux.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Qt build file to generate a Linux package
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

TEMPLATE = subdirs

DEB_HOST_ARCH=$$system(dpkg-architecture -qDEB_HOST_ARCH)
!build_pass {
  isEmpty(DEB_HOST_ARCH) {
    warning("dpkg-architecture not found, .deb file will not be built (try: sudo apt-get install dpkg-dev)")
    KIT = tar
  } else {
    KIT = deb tar
  }
}
kit.commands   = for target in $$KIT ; do $(MAKE) -f Makefile.linux \$\$target ; done
prepare.commands = $(MAKE) -f Makefile.linux prepare
tar.commands   = $(MAKE) -f Makefile.linux tar
clean.commands = $(MAKE) -f Makefile.linux clean
distclean.depends = clean

QMAKE_EXTRA_TARGETS = kit prepare tar clean distclean

include (../../main_defs.pri)
include (../../version.pri)
# Minimal version of Qt packages (Ubuntu packaging, see control.in)
# Note: Use show_deps.sh to maintain the list of dependencies
QTVER="4:4.7.4"
QMAKE_SUBSTITUTES = Makefile.config.in tao.sh.in
isEmpty(TAO_PLAYER) {
  TAOEXE = Tao Presentations
  QMAKE_SUBSTITUTES += control.in
} else {
  TAOEXE = Tao Presentations Player
  QMAKE_SUBSTITUTES += control_player.in
}
