# We need bash and either lupdate, lupdate-qt5 or lupdate-qt4
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
system(bash -c \"lupdate -help >/dev/null 2>&1\") {
  !build_pass:message(Found lupdate)
  LUPDATE=lupdate
} else {
  system(bash -c \"lupdate-qt5 -help >/dev/null 2>&1\") {
    !build_pass:message(Found lupdate-qt5)
    LUPDATE=lupdate-qt5
  } else {
    system(bash -c \"lupdate-qt4 -help >/dev/null 2>&1\") {
      !build_pass:message(Found lupdate-qt4)
      LUPDATE=lupdate-qt4
    }
    else {
     error("Can't execute lupdate, lupdate-qt5 nor lupdate-qt4")
    }
  }
}

# Translations: convenience targets "make lupdate" and "make lrelease"
# Include this file AFTER relevant variables have been defined
lupdate.commands = $$LUPDATE -verbose $$SOURCES $$HEADERS $$FORMS $$CXXTBL_SOURCES $$NOWARN_SOURCES -ts $$TRANSLATIONS || lupdate -verbose $$SOURCES $$HEADERS $$FORMS $$CXXTBL_SOURCES $$NOWARN_SOURCES -ts $$TRANSLATIONS

lupdate.depends  = FORCE
QMAKE_EXTRA_TARGETS += lupdate
lrelease.commands = lrelease $$TRANSLATIONS
lrelease.depends  = FORCE
QMAKE_EXTRA_TARGETS += lrelease

