/**
 * @defgroup stereoscopy
 * @ingroup TaoBuiltins
 *
 * Stereoscopy abilities.
 * 
 * @{
 */

/**
 * Controls stereoscopy
 * @param mode is @c ON or @c OFF.
 */
enable_stereoscopy(mode:name);

/**
 * Controls the number of plane for stereoscopy.
 * To adapt to different stereoscopy technology, Tao presentation is 
 * able to adapt the number of stereoscopic plane.
 */
stereoscopy_planes(cnt:integer);

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
