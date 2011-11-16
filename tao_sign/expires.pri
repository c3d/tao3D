!isEmpty(LICENCE_VALIDITY):VALID=$$LICENCE_VALIDITY
isEmpty(TAO_EDITION):VALID=120  # Days
!isEmpty(VALID) {
  macx {
    EXPIRES=$$system(LANG=C; date -u -j -f \"%s\" $(expr $(date +%s) + $$VALID \\* 86400 ) \"+%d-%h-%Y\")
  } else {
    # Linux, MinGW: GNU date
    EXPIRES=$$system(bash -c \"LANG=C; date -d \\\"now + $$VALID days\\\" +%d-%h-%Y\")
  }
  EXPIRES_LINE="expires $$EXPIRES"
}
