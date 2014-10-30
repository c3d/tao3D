# Defines the list of modules to build.
# Defaults can be changed with "qmake modules=..." (see below)

# Default module list

# Default modules = all sub-directories
LSOUT=$$system(ls)
for(f, LSOUT):exists($${f}/$${f}.pro):DEFAULT_MODULES += $$f

# Add all themes
THEMES=$$system(ls themes)
for(f, THEMES):exists(themes/$${f}/$${f}.pro):DEFAULT_MODULES += themes/$$f

# No other modules
OTHER_MODULES =

# Process qmake command line variable: 'modules'
# Allows to add/remove modules to/from the default value above
# Resulting list is stored in MODULES
#   modules=none
#     Do not build any Tao module
#   modules=all
#     Build all Tao modules ($$DEFAULT_MODULES and $$OTHER_MODULES)
#   modules=+my_module
#     Add my_module to default module list
#   modules=-my_module
#     Remove my_modules from default module list
#   modules="all -my_module"
#     Build all modules except my_module
#   modules="none +my_module"
#     Build only my_module
MODULES = $$DEFAULT_MODULES
!isEmpty(modules) {
    options = $$modules
    for(opt, options) {
        equals(opt, "all") {
            MODULES = $$DEFAULT_MODULES $$OTHER_MODULES
        }
        equals(opt, "none") {
            MODULES =
        }
        remove = $$find(opt, ^-.*)
        !isEmpty(remove) {
            remove ~= s/-//
            MODULES -= $$remove
        }
        add = $$find(opt, ^\\+.*)
        !isEmpty(add) {
            add ~= s/\\+//
            exists($$add) {
                MODULES += $$add
            } else {
                warning(Module directory not found: $$add -- not added)
            }
        }
    }
}
