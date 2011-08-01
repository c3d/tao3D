/**
 * @addtogroup Lighting
 * @ingroup Graphics
 *
 * Controls the lighting of 3D scenes.
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
 * (layouts are created with @ref code.doxy.h::locally "locally" or @ref graph.doxy.h::shape "shape").
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
 * The following code (<a href="examples/lighting.ddd">lighting.ddd</a>) draws
 * a white sphere and creates three colored lights:
 * one red, one green and one blue. Each light rotates around the sphere, so
 * that you can see how lights blend into smooth colors gradients onto the
 * sphere.
 * @include lighting.ddd
 *
 * @image html lighting.png "Lighting demo: lighting.ddd"
 * @{
 */

/**
 * Switch a light on or off and make it the current light.
 * @p id is an integer between 0 and the maximum number of lights minus 1.
 * The number of lights depends on the OpenGL implementation, but at least
 * eight lights are supported.
 * When the primitive returns, the light is the current light for the
 * primitives that control light properties.
 */
tree light(id:integer, enable:boolean);


/**
 * Switch a light on and make it the current light.
 * Same as calling light(integer id, boolean enable) with enable = true.
 */
light(id:integer);

/**
 * Return current light id.
 * The default value is -1 if there is no enabled light.
 */
integer light_id();

/**
 * Sets the color and intensity of the ambient emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_AMBIENT, {r, g, b, a})</tt>
 */
light_ambient(r:real, g:real, b:real, a:real);


/**
 * Sets the color and intensity of the diffuse emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_DIFFUSE, {r, g, b, a})</tt>
 */
light_diffuse(r:real, g:real, b:real, a:real);


/**
 * Sets the color and intensity of the specular emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_SPECULAR, {r, g, b, a})</tt>
 */
light_specular(r:real, g:real, b:real, a:real);


/**
 * Sets the position of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, 0})</tt>
 */
light_position(x:real, y:real, z:real);


/**
 * Sets the position of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, w})</tt>
 */
light_position(x:real, y:real, z:real, w:real);


/**
 * Sets the spot direction of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_SPOT_DIRECTION, {x, y, z})</tt>
 */
light_spot_direction(x:real, y:real, z:real);


/**
 * Sets the spot exponent of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightf(id, GL_SPOT_EXPONENT, x)</tt>
 */
light_spot_exponent(x:real);

/**
 * Sets the spot cutoff of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightf(id, GL_SPOT_CUTOFF, x)</tt>
 */
light_spot_cutoff(x:real);

/**
 * Sets the attenuation factors of the current light.
 * Equivalent to the OpenGL calls:
 * @code
 * glLightf(id, GL_CONSTANT_ATTENUATION, c);
 * glLightf(id, GL_LINEAR_ATTENUATION, l);
 * glLightf(id, GL_QUADRATIC_ATTENUATION, q);
 * @endcode
 */
light_attenuation(c:real, l:real, q:real);


/**
 * Sets the ambient parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 */
material_ambient(r:real, g:real, b:real, a:real);


/**
 * Sets the diffuse parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 */
material_diffuse(r:real, g:real, b:real, a:real);


/**
 * Sets the specular parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 */
material_specular(r:real, g:real, b:real, a:real);


/**
 * Sets the emission parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, {r, g, b, a})</tt>
 */
material_emission(r:real, g:real, b:real, a:real);


/**
 * Sets the shininess parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, x)</tt>
 */
material_shininess(x:real);


/**
 * Sets the ambient parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_AMBIENT, {r, g, b, a})</tt>
 */
material_front_ambient(r:real, g:real, b:real, a:real);


/**
 * Sets the diffuse parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_DIFFUSE, {r, g, b, a})</tt>
 */
material_front_diffuse(r:real, g:real, b:real, a:real);


/**
 * Sets the specular parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_SPECULAR, {r, g, b, a})</tt>
 */
material_front_specular(r:real, g:real, b:real, a:real);


/**
 * Sets the emission parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_EMISSION, {r, g, b, a})</tt>
 */
material_front_emission(r:real, g:real, b:real, a:real);


/**
 * Sets the shininess parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_FRONT, GL_SHININESS, x)</tt>
 */
material_front_shininess(x:real);


/**
 * Sets the ambient parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 */
material_back_ambient(r:real, g:real, b:real, a:real);


/**
 * Sets the diffuse parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 */
material_back_diffuse(r:real, g:real, b:real, a:real);


/**
 * Sets the specular parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 */
material_back_specular(r:real, g:real, b:real, a:real);


/**
 * Sets the emission parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_EMISSION, {r, g, b, a})</tt>
 */
material_back_emission(r:real, g:real, b:real, a:real);


/**
 * Sets the shininess parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_BACK, GL_SHININESS, x)</tt>
 */
material_back_shininess(x:real);


/**
 * @}
 */
