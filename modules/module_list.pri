# The modules listed here are packaged with Tao and installed at the same time
# as the application ; they are not managed by Git (and therefore, each module
# MUST have its own version attribute in module.xl).

DEFAULT_MODULES = lorem_ipsum object_loader tao_visuals digital_clock slides
OTHER_MODULES = hello_world taoTester

# Process qmake command line variable: 'modules'
# Allows to add/remove modules to/from the default value above
# Resulting list is stored in MODULES
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

