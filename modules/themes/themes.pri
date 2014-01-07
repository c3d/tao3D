include(../modules.pri)

!contains(templates, none) {
  exists($$_PRO_FILE_PWD_/theme.xl):MAYBE_THEME_XL=theme.xl

  thismod_template.path = $$APPINST/templates/$$MODINSTDIR
  thismod_template.files = template.ini thumbnail.png $$MAYBE_THEME_XL sample_slide.ddd sample_slide.json sample_slide_images.json

  INSTALLS += thismod_images thismod_template

  SIGN_XL_SOURCES = $$MAYBE_THEME_XL sample_slide.ddd
  SIGN_XL_INSTPATH=$$APPINST/templates/$$MODINSTDIR
  include(../sign_xl.pri)
}
