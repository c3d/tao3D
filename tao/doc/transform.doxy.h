/**
 * @addtogroup transforms Geometric transforms
 * @ingroup Graphics
 *
 * This group deals with transformation of the space.
 * Transformations are based on OpenGL capacity.
 * Transformations are
 * - rotation, expressed in degrees. Rotations follows the right-hand rule, so if the rotation axis
 *      points toward the user, the rotation will be counterclockwise.
 * - translation, expressed in pixels
 * - scaling, expressed with a scaling factor
 *
 * Those transformations are applied to the whole system : origin and axes are modified by transformation. 
 * So the user should focus on rotation and translation order because 
 * a translation and then a rotation do not give the same location as rotation
 * and then translation, as shown in the following image and sample.
 * @image html rotation_translation.png
 *
 * Examples: <a href="examples/transform_samples.ddd">transform_samples.ddd</a>
 * @{
 */

/**
 * Scales along the three axes.
 *
 * @image html scale.png
 */
scale (x:real, y:real, z:real);

/**
 * Scales along the X axis.
 *
 * Equivalent to @code scale x, 1, 1 @endcode
 */
scalex (x:real);

/**
 * Scales along the Y axis.
 *
 * Equivalent to @code scale 1, y, 1 @endcode
 */
scaley (y:real);

/**
 * Scales along the Z axis.
 *
 * Equivalent to @code scale 1, 1, z @endcode
 */
scalez (z:real);

/**
 * Rotates along an arbitrary axis.
 *
 * Rotation of @p ra degrees around the axis determined by the
 * (@p rx, @p ry, @p rz) vector.
 *
 * Rotations follows the right-hand rule, so if the vector (x,  y,  z)
 * points toward the user, the rotation will be counterclockwise.
 *
 * @image html rotation.png
 */
rotate (ra:real, rx:real, ry:real, rz:real);

/**
 * Rotates around the X axis.
 *
 * Equivalent to @code rotate rx, 1, 0, 0 @endcode
 */
rotatex (rx:real);

/**
 * Rotates around the Y axis.
 *
 * Equivalent to @code rotate ry, 0, 1, 0 @endcode
 */
rotatey (ry:real);

/**
 * Rotates around the Z axis.
 *
 * Equivalent to @code rotate rz, 0, 0, 1 @endcode
 */
rotatez (rz:real);

/**
 * Translates along the three axes.
 *
 * @image html translation.png
 */
translate (x:real, y:real, z:real);

/**
 * Translates along the X axis.
 *
 * Equivalent to @code translate x, 0, 0 @endcode
 */
translatex (x:real);

/**
 * Translates along the Y axis.
 *
 * Equivalent to @code translate 0, y, 0 @endcode
 */
translatey (y:real);

/**
 * Translates along the Z axis.
 *
 * Equivalent to @code translate 0, 0, z @endcode
 */
translatez (z:real);

/**
 * Reset the geometric transform to original projection state.
 */
reset_transform ();

/** @} */
