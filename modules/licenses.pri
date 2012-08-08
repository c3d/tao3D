# Installation of license files for Tao Presentation modules
#
# Performs two things:
# 1. Installs license template(s) into the module installation folder
# 2. (Taodyne private build only) If LICENSE_VALIDITY and SIGN_MODULE_LICENSES
#    are both defined (LICENSE_VALIDITY is a duration in days),
#    create a temporary license file, sign it and install it into
#    the Tao Presentations install folder.
#
# Usage: In module_name/module_name.pro:
#   LICENSE_FILES = module_name.taokey.notsigned
#   include(../licenses.pri)

isEmpty(MODINSTPATH):error(Please include modules.pri before licenses.pri)

!isEmpty(LICENSE_FILES) {

  # In Taodyne build environment, enable license checking in modules
  exists(../tao_sign/expires.pri):DEFINES += USE_LICENSE

  isEmpty(SIGN_MODULE_LICENSES) {

    # Install all license templates into the module installation directory
    license_files.path  = $$MODINSTPATH
    license_files.files = $$LICENSE_FILES
    INSTALLS += license_files

  } else {

    # Tao Presentations full build only (Taodyne)
    # Generate and sign temporary licenses, and install them into the Tao
    # licenses folder
    !exists(../tao_sign/expires.pri):error(Must be in Taodyne internal build environment to sign licenses)

    # Enable license checking in modules
    DEFINES += USE_LICENSE

    include(../tao_sign/expires.pri)
    !isEmpty(EXPIRES) {  # We want to create temporary licenses

      for(file, LICENSE_FILES):SIGNED_FILES += $$replace(file, .notsigned, )
      !build_pass:message("Will generate $$VALID-day(s) license(s) for module $$MODINSTDIR: $$SIGNED_FILES")

      signed.path = $$APPINST/licenses
      signed.commands = for f in $$LICENSE_FILES ; do $$TAOTOPSRC/tao_sign/sign_temporary.sh \"$$EXPIRES\" \$\$f ; done ; cp $$SIGNED_FILES \"$$APPINST/licenses\"
      signed.files = $$SIGNED_FILES
      signed.depends = $$LICENSE_FILES
      QMAKE_EXTRA_TARGETS += signed
      INSTALLS += signed
      QMAKE_CLEAN += $$SIGNED_FILES
    }
  }
}
