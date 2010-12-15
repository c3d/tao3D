# Define useful variables for modules
# FIXME/TODO: clean duplication in modules.pri

isEmpty(TAO_SDK) {
  TAOTOPSRC = ../..
} else {
  TAOTOPSRC = $${TAO_SDK}
}

include(../main_defs.pri)

# Try to install into (in this order, depending on what variable is defined):
# 1. $$MODINSTPATH
# 2. $$MODINSTROOT/$$MODINSTDIR
# 3. <Tao application install dir>/$$MODINSTDIR
isEmpty(MODINSTPATH) {
  isEmpty(MODINSTROOT) {
    isEmpty(APPINST):error(APPINST not defined)
    MODINSTROOT = $${APPINST}/modules
  }
  isEmpty(MODINSTDIR):error(MODINSTDIR not defined)
  MODINSTPATH      = $${MODINSTROOT}/$$MODINSTDIR
}
