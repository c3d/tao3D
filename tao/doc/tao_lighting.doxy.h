/**
 * @addtogroup Lighting3D 3D lighting
 * @ingroup Graphics3D
 *
 * Controls the lighting of the 3D scene.
 *
 * This page describes all the builtin primitives that are related to lighting.
 * The primitives are mostly direct mappings of the OpenGL calls. Please
 * refer to the OpenGL documentation for details:
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glLight.xml">glLight</a>
 * and
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glMaterial.xml">glMaterial</a>.
 *
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
