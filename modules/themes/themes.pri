include(../modules.pri)

thismod_template.path = $$APPINST/templates/$$MODINSTDIR
thismod_template.files = template.ini thumbnail.png theme.xl sample_slide.ddd

INSTALLS += thismod_images thismod_template
