/**
 * @defgroup LensFlare Lens Flare
 * @ingroup Modules
 *
 * Generate a lens flare.
 *
 * This module allows to create a dynamic lens flare, either using default model,
 * or adding new flares thanks to specified textures.
 *
 * A lens flare is an optical phenomenon that occurs when light reflects inside the lens of the camera.
 * The light bounces off of the camera lens and components inside of the lens and
 * camera to create streaks, haze, and other effects.
 *
 * Typically a lens flare is something to avoid, however some types of flare may actually enhance
 * the artistic meaning of a photo when it is used on purpose.
 *
 * It can help to add realism to the scene, or make an otherwise boring photo
 *  stand out with the artful use of light.
 *
 *
 * For instance, the following code defines the same model of a lens flare created by the sun,
 * than the built-in form @ref sun_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real).
 * @include lens_flare.ddd
 *
 * @image html lens_flare.png "Output from lens_flare.ddd"
 *
 *
 * @note In order to use correctly the depth test, the lens flare must be drawn after the others elements.
 * @note This module integrate already some lens flare textures, which can be used to create differents effects like in the previous example.
 * @note It is also possible to create these textures in Tao thanks to integrated fonctions (@ref frame_texture, @ref radial_gradient, etc.).
 *
 * @{
 */

/**
 * Directory of lens flare textures.
 *
 * This folder contains some images, which
 * can be used to generate differents type of lens flares.
 *
 * Its default value is the images folder in the lens flare module.
 */
text FLARES_DIR;

/**
 * Creates a local lens flare.
 *
 * Define and evaluate the lens flare tree while
 * preserving the current graphical state like a @ref locally.
 *
 * @param contents defines code applied on the current lens flare.
 */
lens_flare(contents:tree);

/**
 * Enable or disable the depth test for the lens flare.
 *
 * @param mode is @c ON or @c OFF.
 *
 */
lens_flare_depth_test(mode:name);

/**
 * Define lens flare source.
 *
 * Set the target of the lens source at (@p x, @p y, @p z).
 */
lens_flare_source(x:real, y:real, z:real);

/**
 * Define lens flare target.
 *
 * Set the target of the lens flare at (@p x, @p y, @p z).
 */
lens_flare_target(x:real, y:real, z:real);

/**
 * Add a new flare to the current lens flare.
 * @param id index of the flare texture.
 * @param loc relative location of the flare. 0 is the source position and 1 is the target one.
 * @param scale scaling rate of the texture. 1 is its normal size.
 * @param r red component of the flare color.
 * @param g green component of the flare color.
 * @param b blue component of the flare color.
 * @param a alpha-channel, transparency of the flare color. 0 is transparent and 1 is opaque.
 */
add_flare(id:integer, loc:real, scale:real, r:real, g:real, b:real, a:real);

/**
 * Create a specific lens flare due to a sun.
 * (@p sx, @p sy, @p sz) is the position of the light source.
 * The target of the lens flare is located at (@p tx, @p ty, @p tz).
 */
sun_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real);

/**
 * Create a simple white lens flare.
 * (@p sx, @p sy, @p sz) is the position of the light source.
 * The target of the lens flare is located at (@p tx, @p ty, @p tz).
 */
white_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real);
/**
* @}
*/
