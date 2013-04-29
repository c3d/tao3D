include(../modules.pri)

!contains(templates, none) {
  thismod_template.path = $$APPINST/templates/$$MODINSTDIR
  thismod_template.files = template.ini thumbnail.png theme.xl sample_slide.ddd

  INSTALLS += thismod_images thismod_template

  SIGN_XL_SOURCES = theme.xl sample_slide.ddd
  SIGN_XL_INSTPATH=$$APPINST/templates/$$MODINSTDIR
  include(../sign_xl.pri)
}
