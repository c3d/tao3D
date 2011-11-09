# Translations: convenience targets "make lupdate" and "make lrelease"
# Include this file AFTER relevant variables have been defined
lupdate.commands = lupdate -verbose $$SOURCES $$HEADERS $$FORMS $$CXXTBL_SOURCES $$NOWARN_SOURCES -ts $$TRANSLATIONS
lupdate.depends  = FORCE
QMAKE_EXTRA_TARGETS += lupdate
lrelease.commands = lrelease $$TRANSLATIONS
lrelease.depends  = FORCE
QMAKE_EXTRA_TARGETS += lrelease

