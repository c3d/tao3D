/**
 * @defgroup Display Display and stereoscopy controls 
 * @ingroup TaoBuiltins
 *
 * Select display mode and stereoscopic 3D settings.
 * Tao Presentations can take advantage of several display technologies
 * to show 3D contents. Depending on the display hardware at your
 * disposal, you may select one of the display modes supported by
 * the application. If you have a 3D-capable screen or projector,
 * chances are that it can be adequately used with one of the display
 * modes listed on this page.
 * 
 * Here is an illustration of the modes currently supported by Tap
 * Presentations (see @ref display_mode): 
 * @image html 2D_small.png "Default (\"default\",  \"2D\")"
 * @image html anaglyph_small.png "Red/blue anaglyph (\"anaglyph\")"
 * @image html anaglyph_detail.png "Red/blue anaglyph (detail)"
 * @image html hsplitstereo_small.png "Side-by-side (\"hsplitstereo\", \"sidebysidestereo\")"
 * @image html vsplitstereo_small.png "Over/under (\"vsplitstereo\",  \"overunderstereo\")"
 * @image html hintstereo_small.png "Interlaced (\"hintstereo\")"
 * @image html hintstereo_detail.png "Interlaced (detail)"
 * @image html checkerboard_small.png "Checkerboard (\"checkerboard\")"
 * @image html checkerboard_detail.png "Checkerboard (detail)"
 * @image html hframepacking_small.png "Horizontal frame packing (\"hframepacking\")"
 * @image html vframepacking_small.png "vertical frame packing (\"vframepacking\")"
 * @image html alioscopy_small.png "Alioscopy® (\"alioscopy\")"
 * @image html alioscopy_detail.png "Alioscopy® (detail)"
 * @{
 */

/**
 * Controls stereoscopy.
 * When @p mode is @c false, stereoscopic output is disabled, and
 * the 2D output mode is selected.
 * When @p mode is @c true, the quad-buffer stereoscopic output
 * is enabled, if supported by the hardware. If this mode is not
 * supported, the primitive does nothing.
 * @param mode is @c true or @c false.
 */
enable_stereoscopy(mode:name);

/**
 * Select the display mode.
 * @param mode The name of the display mode. Currently supported
 * modes are:
 * - "default", "2D": two-dimensional display.
 * - "quadstereo": OpenGL quad-buffered stereoscopic 3D output.
 *   Exact video output format depends on the OpenGL driver.
 * - "anaglyph": stereoscopic 3D on a regular screen with red/blue
 *   glasses.
 * - "hsplitstereo", "sidebysidestereo": stereoscopic 3D with horizontal
 *   split enconding. The left half of the picture is for the left eye,
 *   the right half is for the right eye. Each half represents the full
 *   scene and is compressed horizontally. For instance, when rendering to
 *   a 1920x1080 (HDTV) screen you will get two 960x1080 images side-by-side.
 *   This format is typically supported by many consumer-grade 3D HDTV.
 * - "vsplitstereo", "overunderstereo": stereoscopic 3D with vertical
 *   split encoding. Similar to horizontal split (above), but vertically.
 * - "hintstereo": stereoscopic 3D with horizontal interlace encoding.
 *   Some 3D capable laptops (with passive glasses) expect this format.
 * - "checkerboard": stereoscopic 3D with checkerboard encoding.
 * - "hframepacking": stereoscopic 3D output in horizontal frame packing
 *   mode. This is similar to the horizontal side-by-side
 *   disposition ("hsplitstereo"), but without horizontal compression.
 * - "vframepacking": stereoscopic 3D output in verical frame packing
 *   mode. Like the vertical over/under disposition, but without vertical
 *   compression.
 * - "alioscopy": suitable encoding for
 *   <a href="http://www.alioscopy.com">Alioscopy® glass-free 3D
 *   displays</a>.
 * @note Display modes are implemented as modules. Some modes may
 * or may not be available, depending on your version of Tao Presentations.
 * You can obtain a list of all
 * valid mode strings for your configuration by running Tao Presentations
 * with the @c -tdisplaymode command-line parameter. Look for output
 * similar to:
 * @code
[Display Driver] Registering display function: checkerboard@0x1b4148c0
[Display Driver] Registering display function: hsplitstereo@0x1c76c110
[Display Driver] Registering alias: sidebysidestereo => hsplitstereo@0x1c76c110
 * @endcode
 * Display modes can also be selected with the command chooser (Escape
 * key), or with the View/Display mode menu. 
 */
display_mode(mode:text);

/**
 * The name of the current display mode.
 */
text display_mode();

/**
 * Test if a display mode is available.
 * @see display_mode, display_mode(mode:text)
 */
boolean has_display_mode(mode:text);

/**
 * Get eye distance.
 * @return [real] eye distance.
 */
eye_distance();

/**
 * Set eye distance.
 */
eye_distance(e:real);

/**
 * @}
 */
