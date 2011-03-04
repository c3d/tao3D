/**
 * @addtogroup transforms Geometric transforms
 * @ingroup Graphics
 *
 * This group deals with transformation of the space.
 * Transformations are based on OpenGL capacity.
 * Transformations are
 * - rotation, express in degrees
 * - translation, express in pixels
 * - scaling, express in percentage
 *
 * @note Rotations follows the right-hand rule, so if the rotation axis
 *      points toward the user, the rotation will be counterclockwise.
 *
 * @{
 */

/**
 * @fn scalex ( x)
 * @brief Rescale along X
 *
 *  Rescale along X-axis. It is equivalent to @code scale x, 1, 1 @endcode
 *
 * @param x [real] scale factor.
 */
scalex ( x);

/**
 * @fn scaley ( y)
 * @brief Rescale along Y
 *
 *  Rescale along Y-axis. It is equivalent to @code scale 1, y, 1 @endcode
 *
 * @param y [real] scale factor.
 */
scaley ( y);

/**
 * @fn scalez ( z)
 * @brief Rescale along Z
 *
 *  Rescale along Z-axis. It is equivalent to @code scale 1, 1, z @endcode
 *
 * @param z [real] scale factor.
 */
scalez ( z);

/**
 * @fn rotate ( ra, rx, ry, rz)
 * @brief Rotation along an arbitrary axis
 *
 *  Rotation of @p ra degrees arround the axis determined by the <TT> (rx, ry, rz) </TT> vector.
 *
 * @param ra [real] amount of degrees to rotate
 * @param rx [real] x-coordinate of the vector
 * @param ry [real] y-coordinate of the vector
 * @param rz [real] z-coordinate of the vector
 *
 * @note Rotations follows the right-hand rule, so if the vector (x,  y,  z)
 *      points toward the user, the rotation will be counterclockwise.
 */
rotate ( ra, rx, ry, rz);

/**
 * @fn scale ( x, y, z)
 * @brief Scaling along three axis
 *
 *  Scaling along three axis.
 *
 * @param x [real] x-scale factor.
 * @param y [real] y-scale factor.
 * @param z [real] z-scale factor.
 */
scale ( x, y, z);

/**
 * @fn rotatex ( rx)
 * @brief Rotate around X
 *
 *  Rotate around X axis. It is equivalent to @code rotate rx, 1, 0, 0 @endcode
 *
 * @param rx [real] The angle of rotation in degree.
 */
rotatex ( rx);

/**
 * @fn rotatey ( ry)
 * @brief Rotate around Y
 *
 *  Rotate around Y axis. It is equivalent to @code rotate ry, 0, 1, 0 @endcode
 *
 * @param ry [real] The angle of rotation in degree.
 */
rotatey ( ry);

/**
 * @fn rotatez ( rz)
 * @brief Rotate around Z
 *
 *  Rotate around Z axis : x-axis > 0 is 0 degree, y-axis > 0 is 90 degrees, etc...
 * It is equivalent to @code rotate rz, 0, 0, 1 @endcode
 * @param rz [real] The angle of rotation in degree.
 */
rotatez ( rz);

/**
 * @fn translatez ( z)
 * @brief Translate along Z
 *
 *  Translate along Z-axis. It is equivalent to @code translate 0, 0, z @endcode
 *
 * @param z [real] z translation.
 */
translatez ( z);

/**
 * @fn translatey ( y)
 * @brief Translate along Y
 *
 *  Translate along Y-axis. It is equivalent to @code translate 0, y, 0 @endcode
 *
 * @param y [real] y translation.
 */
translatey ( y);

/**
 * @fn translatex ( x)
 * @brief Translate along X
 *
 *  Translate along X-axis. It is equivalent to @code translate x, 0, 0 @endcode
 *
 * @param x [real] x translation.
 */
translatex ( x);

/**
 * @fn translate ( x, y, z)
 * @brief Translation along three axes
 *
 *  Translation along three axes
 *
 * @param x [real] x translation.
 * @param y [real] y translation.
 * @param z [real] z translation.
 */
translate ( x, y, z);

/**
 * @fn reset_transform ()
 * @brief Reset transform
 *
 *  Reset transform to original projection state.
 */
reset_transform ();

/** @} */
