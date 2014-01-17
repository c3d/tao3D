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
