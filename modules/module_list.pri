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
       movie_credits    \
       pan_and_zoom     \
       digital_clock    \
       slides           \
       animate          \
       filters          \
       shading          \
       materials        \
       lens_flare       \
       mapping          \
       display_anaglyph \
       display_quadstereo \
       display_splitstereo \
       display_intstereo \
       display_alioscopy \
       display_checkerboard \
       display_2dplusdepth \
       display_tridelity \
       shaders/monjori  \
       shaders/flares  \
       shaders/sinuous \
       shaders/sparks  \
       shaders/glow  \
       shaders/depth_of_field \
       themes/blue_claire \
       themes/autumn_on_earth \
       themes/keyboard \
       themes/seyes \
       themes/black_white \
       themes/pastel_triangles \
       themes/bright_rectangles \
       themes/finance \
       themes/lucky_stars \
       themes/landscapes \
       themes/seasons_greetings \
       themes/white_christmas \
       themes/water \
       slideshow_3d \
       vlc_audio_video \
       stereo_decoder \
       carousel \
       revolving_texts \
       point_cloud \
       network_access \
       water_surface \
       remote_control \
       regexp \
       chroma_key \
       transitions \
       news_feed \
       snowfall \
       nodejs \
       film_strip \
       speech \
       charts

    OTHER_MODULES =  \
        DIP          \
        hello_world  \
        tao_synchro  \
        tao_tester   \
        tao_quiz     \
        database     \
        web_remote \
        prez_at

} else {
    # We're building inside the Tao SDK: default modules = all sub-directories
    LSOUT=$$system(ls)
    for(f, LSOUT):exists($${f}/$${f}.pro):DEFAULT_MODULES += $$f
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
