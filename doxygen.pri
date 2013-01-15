isEmpty(NO_DOC) {
  system(bash -c \"doxygen --version >/dev/null 2>&1\") {
    HAS_DOXYGEN=1
  } else {
    warning(doxygen not found - will not build online documentation)
  }
}
DOXYGEN = $$PWD/doxygen.sh
