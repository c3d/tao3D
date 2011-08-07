# Defines the list of templates to install.
# Defaults can be changed with "qmake templates=..." (see below)

# Default template list

DEFAULT_TEMPLATES =   \
    blank \
    pythagorean_theorem \
    hello_world \
    guess_game \
    mandelbrot \
    simple_slides \
    pigs_fly \
    photo_viewer \
    lens_flare \
    filters \
    shading \
    movie \
    mapping \
    pan_and_zoom

OTHER_TEMPLATES =     \
    monjori

# Process qmake command line variable: 'templates'
# Allows to add/remove templates to/from the default value above
# Resulting list is stored in TEMPLATES
#   templates=none
#     Do not install any Tao template
#   templates=all
#     Build all Tao modules ($$DEFAULT_TEMPLATES and $$OTHER_TEMPLATES)
#   templates=+my_template
#     Add my_template to default template list
#   templates=-my_template
#     Remove my_template from default template list
#   templates="all -my_template"
#     Build all templates except my_template
#   templates="none +my_template"
#     Build only my_template
TEMPLATES = $$DEFAULT_TEMPLATES
!isEmpty(templates) {
    options = $$templates
    for(opt, options) {
        equals(opt, "all") {
            TEMPLATES = $$DEFAULT_TEMPLATES $$OTHER_TEMPLATES
        }
        equals(opt, "none") {
            TEMPLATES =
        }
        remove = $$find(opt, ^-.*)
        !isEmpty(remove) {
            remove ~= s/-//
            TEMPLATES -= $$remove
        }
        add = $$find(opt, ^\\+.*)
        !isEmpty(add) {
            add ~= s/\\+//
            exists($$add) {
                TEMPLATES += $$add
            } else {
                warning(Template directory not found: $$add -- not added)
            }
        }
    }
}

# Some templates depend on module availability
include (../modules/module_list.pri)
!contains (MODULES, slides):TEMPLATES -= simple_slides
!contains (MODULES, object_loader):TEMPLATES -= pigs_fly
!contains(MODULES, tao_visuals):TEMPLATES -= pigs_fly
!contains (MODULES, slideshow_3d):TEMPLATES -= photo_viewer
!contains (MODULES, lens_flare):TEMPLATES -= lens_flare
!contains (MODULES, filters):TEMPLATES -= filters
!contains (MODULES, shading):TEMPLATES -= shading
!contains (MODULES, movie_credits):TEMPLATES -= movie
!contains (MODULES, mapping):TEMPLATES -= mapping
!contains (MODULES, pan_and_zoom):TEMPLATES -= pan_and_zoom
