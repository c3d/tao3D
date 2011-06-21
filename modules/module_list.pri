# Defines the list of modules to build.
# Defaults can be changed with "qmake modules=..." (see below)

# Default module list

!exists(../main.pro):IS_SDK=true
!equals(IS_SDK, true) {
    # Building the full Tao Presentations.
    # The modules listed here are packaged with Tao and installed at the same time
    # as the application ; they are not managed by Git (and therefore, each module
    # MUST have its own version attribute in <module_name>.xl).
    DEFAULT_MODULES =   \
       lorem_ipsum      \
       object_loader    \
       tao_visuals      \
       digital_clock    \
       slides           \
       animate          \
       display_quadstereo \
       display_splitstereo \
       display_intstereo \
       display_alioscopy \
       slideshow_3d
    OTHER_MODULES =     \
        hello_world     \
        taoTester
} else {
    # We're building inside the Tao SDK. Only some modules are available.
    DEFAULT_MODULES = hello_world lorem_ipsum object_loader
    OTHER_MODULES =
}

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
