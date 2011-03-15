/**
 * @addtogroup Camera
 * @ingroup Graphics
 *
 * Controls the position, orientation and characteristics of the camera.
 *
 * The camera primitives allow to query and modify the point of view, and thus
 * the way the document appears. The parameters that have an influence on the
 * view are:
 *  - the location of the viewpoint (see @ref camera_position)
 *  - the aiming point (see @ref camera_target). Along with the
 *    location of the viewpoint, the aiming point defines the line of sight.
 *  - the "up vector" of the camera (see @ref camera_up_vector). This vector
 *    defines which direction is up (that is, the direction from the bottom to
 *    the top of the view). By default, the up vector of the camera is along
 *    the @p y axis of the scene.
 * @note The viewpoint location, the aiming point and the up vector are the
 * same as in the GLU library function gluLookat().
 *  - the zoom factor. Controls the "focal length" of the camera. Change the
 *    zoom to make objects appear bigger or smaller. See @ref zoom.
 * @{
 */

/**
 * Returns the current camera position.
 * The default position is (0, 0, 6000).
 * @return 3 real values, separated by commas.
 */
infix camera_position();

/**
 * Sets the current camera position.
 */
camera_position(x:real, y:real, z:real);

/**
 * Returns the point the camera is aiming at.
 * The default value is the origin, (0, 0, 0).
 * @return 3 real values, separated by commas.
 */
infix camera_target();

/**
 * Sets the point the camera is aiming at.
 */
camera_target(x:real, y:real, z:real);

/**
 * Returns the camera "up" direction.
 * The default value is (0, 1, 0), so that the y axis appears vertical.
 * @return 3 real values, separated by commas.
 */
infix camera_up_vector();

/**
 * Sets the camera "up" direction.
 */
camera_up_vector(x:real, y:real, z:real);

/**
 * Returns the current zoom factor.
 * The default value is 1.0, or 100%. By default, the "focal length" of the
 * camera is setup so that one pixel at the origin corresponds to one pixel in
 * the output image.
 */
real zoom();

/**
 * Sets the current zoom factor.
 * Higher values result in a higher magnification.
 */
zoom(real z);

/**
 * @}
 */
