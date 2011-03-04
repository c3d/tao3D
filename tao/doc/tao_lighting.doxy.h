/**
 * @addtogroup Lighting
 * @ingroup Graphics
 *
 * Controls the lighting of the 3D scene.
 *
 * This page describes all the builtin primitives that are related to lighting.
 * The primitives are mostly direct mappings of the OpenGL calls. Please
 * refer to the OpenGL documentation of
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glLight.xml">glLight</a>
 * and
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glMaterial.xml">glMaterial</a>
 * for details.
 *
 * @par Enabling lighting
 *
 * Lighting calculations do not occur by default. They are enabled as
 * lights are declared and only within the current and child layouts
 * (layouts are created with @c locally or @c shape).
 * Therefore, the following code will
 * draw one orange rectangle and one yellow rectangle, as expected:
 @code
locally
    // Red light (diffuse)
    light 0
    light_diffuse 1, 0, 0, 1
    light_position 200, 200, 200
    // Yellow material
    material_ambient 1, 1, 0, 1
    material_diffuse 1, 1, 0, 1
    rectangle -100, 0, 150, 100

// Regular color model, no lighting
color "yellow"
rectangle 100, 0, 150, 100
 @endcode
 *
 * Without the <tt>locally</tt> call (for instance, if you replace
 * <tt>'locally'</tt> with <tt>'if true then'</tt>),
 * the program shows two orange rectangles. This is because
 * lighting is still active when the second rectangle is drawn, contrary
 * to what occurs when the light is enclosed in a @c locally block. The
 * yellow rectangle being lit with a red light, it appears orange.
 *
 * @par Colors and materials
 *
 * When lighting is enabled and until you set materials
 * explicitely (by calling one of the <tt>material_*</tt> functions),
 * the current color will be considered as a "front and back, ambient
 * and diffuse" material. That is, the following code:
 @code
// Yellow
color 1, 1, 0, 1
 @endcode
 * ...is equivalent to:
 @code
// Yellow
material_ambient 1, 1, 0, 1
material_diffuse 1, 1, 0, 1
 @endcode
 *
 * @par Example
 *
 * The following code draws a white sphere and creates three colored lights:
 * one red, one green and one blue. Each light rotates around the sphere, so
 * that you can see how lights blend into smooth colors gradients onto the
 * sphere.
 @code
// lighting.ddd

clear_color 0, 0, 0, 0

create_light id, x, y, z, dr, dg, db ->
    light id
    light_ambient 0, 0, 0, 1
    light_diffuse dr, dg, db, 1
    light_position x, y, z

red_light x, y, z -> create_light 0, x, y, z, 1, 0, 0
green_light x, y, z -> create_light 1, x, y, z, 0, 1, 0
blue_light x, y, z -> create_light 2, x, y, z, 0, 0, 1

white_sphere r ->
    // Note: material_ambient is visible due to the default
    // OpenGL global ambient light (0.2, 0.2, 0.2, 1.0)
    material_ambient 0.3, 0.3, 0.3, 1
    material_diffuse 1.0, 1.0, 1.0, 1.0
    slices := 50
    sphere 0, 0, 0, r, r, r, slices, slices

draw_scene ->
    d := window_height
    t := time
    red_light d * sin t, d * cos t, d
    green_light d * sin ( -t + 120) , d * cos ( -t + 120) , d
    blue_light 0, d * sin t, d * cos t
    white_sphere d * 60%

draw_scene
 @endcode
 *
 * @image html lighting.png "Lighting demo: lighting.ddd"
 * @{
 */

/**
 * Switch a light on or off and make it the current light.
 * @a id is an integer between 0 and the maximum number of lights minus 1.
 * The number of lights depends on the OpenGL implementation, but at least
 * eight lights are supported.
 * When the primitive returns, the light is the current light for the
 * primitives that control light properties.
 */
tree light(integer id, boolean enable);


/**
 * Switch a light on and make it the current light.
 * Same as calling light(integer id, boolean enable) with enable = true.
 */
tree light(integer id);


/**
 * Sets the color and intensity of the ambient emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_AMBIENT, {r, g, b, a})</tt>
 */
tree light_ambient(real r, real g, real b, real a);


/**
 * Sets the color and intensity of the diffuse emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_DIFFUSE, {r, g, b, a})</tt>
 */
tree light_diffuse(real r, real g, real b, real a);


/**
 * Sets the color and intensity of the specular emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_SPECULAR, {r, g, b, a})</tt>
 */
tree light_specular(real r, real g, real b, real a);


/**
 * Sets the position of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, 0})</tt>
 */
tree light_position(real x, real y, real z);


/**
 * Sets the position of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, w})</tt>
 */
tree light_position(real x, real y, real z, real w);


/**
 * Sets the spot direction of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_SPOT_DIRECTION, {x, y, z})</tt>
 */
tree light_spot_direction(real x, real y, real z);


/**
 * Sets the spot exponent of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightf(id, GL_SPOT_EXPONENT, x)</tt>
 */
tree light_spot_direction(real x);


/**
 * Sets the spot cutoff of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightf(id, GL_SPOT_CUTOFF, x)</tt>
 */
tree light_spot_cutoff(real x);


/**
 * Sets the attenuation factors of the current light.
 * Equivalent to the OpenGL calls:
 * <tt>glLightf(id, GL_CONSTANT_ATTENUATION, x)</tt> \n
 * <tt>glLightf(id, GL_LINEAR_ATTENUATION, x)</tt> \n
 * <tt>glLightf(id, GL_QUADRATIC_ATTENUATION, x)</tt>
 */
tree light_attenuation(real x);


/**
 * Sets the ambient parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 */
tree material_ambient(real r, real g, real b, real a);


/**
 * Sets the diffuse parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 */
tree material_diffuse(real r, real g, real b, real a);


/**
 * Sets the specular parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 */
tree material_specular(real r, real g, real b, real a);


/**
 * Sets the emission parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, {r, g, b, a})</tt>
 */
tree material_emission(real r, real g, real b, real a);


/**
 * Sets the emission parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, x)</tt>
 */
tree material_shininess(real x);


/**
 * Sets the ambient parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_AMBIENT, {r, g, b, a})</tt>
 */
tree material_front_ambient(real r, real g, real b, real a);


/**
 * Sets the diffuse parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_DIFFUSE, {r, g, b, a})</tt>
 */
tree material_front_diffuse(real r, real g, real b, real a);


/**
 * Sets the specular parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_SPECULAR, {r, g, b, a})</tt>
 */
tree material_front_specular(real r, real g, real b, real a);


/**
 * Sets the emission parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_EMISSION, {r, g, b, a})</tt>
 */
tree material_front_emission(real r, real g, real b, real a);


/**
 * Sets the emission parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_FRONT, GL_SHININESS, x)</tt>
 */
tree material_front_shininess(real x);


/**
 * Sets the ambient parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 */
tree material_back_ambient(real r, real g, real b, real a);


/**
 * Sets the diffuse parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 */
tree material_back_diffuse(real r, real g, real b, real a);


/**
 * Sets the specular parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 */
tree material_back_specular(real r, real g, real b, real a);


/**
 * Sets the emission parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_EMISSION, {r, g, b, a})</tt>
 */
tree material_back_emission(real r, real g, real b, real a);


/**
 * Sets the emission parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_BACK, GL_SHININESS, x)</tt>
 */
tree material_back_shininess(real x);


/**
 * @}
 */
