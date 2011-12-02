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
 * @note When a stereoscopic display mode is active, the viewpoint location
 * and the aiming point are adjusted by the amount of the eye separation.
 * Tao Presentation implements an <em>asymmetric frustum parallel axis</em>
 * projection method, which allows the creation of comfortable 3D content
 * that minimize eye strain for the viewer.
 * Cameras (eyes) lie on a straight line, not on an arc, and are spaced evenly
 * by the @ref eye_distance distance. The center of the camera segment is the
 * @ref camera_position. Cameras are globbaly aimed at the @ref camera_target
 * point, but they are parallel: there is no inward rotation or vergence point.
 *  - the zoom factor. Controls the "focal length" of the camera. Change the
 *    zoom to make objects appear bigger or smaller. See @ref zoom.
 * @{
 */

/**
 * Returns the current camera position.
 * The default position is (0, 0, 6000).
 * To read one component you may write:
 * @code
cam_x -> camera_position at 1
cam_y -> camera_position at 2
cam_z -> camera_position at 3
 * @endcode
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
 * Sets the distance from the viewer to the near clipping plane (zNear).
 * The zNear value is always positive and non-zero.
 */
z_near(zn:real);

/**
 * Returns the distance from the viewer to the near clipping plane (zNear).
 * The zNear value is always positive and non-zero. The default value
 * is 1000.0.
 * @return [real] current zNear value.
 */
z_near();

/**
 * Sets the distance from the viewer to the far clipping plane (zFar).
 * The zFar value is always positive and non-zero.
 */
z_far(zf:real);

/**
 * Returns the distance from the viewer to the far clipping plane (zFar).
 * The zFar value is always positive and non-zero. The default value is
 * 56000.0.
 * @return [real] current zFar value.
 */
z_far();

/**
 * Convert a screen position to world coordinates.
 * Transform a screen position defined by (@p x, @p y) to world
 * coordinates @p wx, @p wy and @p wz. The depth at the given location
 * is given by the current value of the OpenGL depth buffer.
 * The parameters @p wx, @p wy and @p wz are modified.
 * To simply obtain the depth, use the @ref depth_at function.
 *
 * @return a real number corresponding to the Z position
 */
real world_coordinates(x:real, y:real, wx:real, wy:real, wz:real);

/**
 * Get the Z position corresponding to the given coordinates @p x and @p y.
 * The Z position is computed from the OpenGL depth buffer.
 * To obtain the three coordinates, use @ref world_coordinates.
 *
 * @return the Z position at the given coordinates
 */
real depth_at(x:real, y:real);

/**
 * Configure the camera as in early versions of Tao Presentations
 * (up to 1.0-beta11).
 * - Camera position is (0, 0, 6000) [now (0, 0, 3000)]
 * - Camera target is (0, 0, 0) [unchanged]
 * - Camera up vector is (0, 1, 0) [unchanged]
 * - zNear is 1000 [now 1500]
 * - zFar is 56000 [now 1e6]
 * - Default eye separation for stereoscopy is 10 [now 100]
 * - Polygon offset settings are:
 * @code
polygon_offset 0, -0.001, 0, -1
 * @endcode
 * [now 0, -0.1, 0, -1]
 */
old_camera_settings();

/**
 * @}
 */
