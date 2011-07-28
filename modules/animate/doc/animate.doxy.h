/**
 * @defgroup Animate Animation utilities for slides
 * @ingroup Modules
 *
 * Commands to create animations and transitions within a slide
 *
 * The Animate module defines commands that you can use to animate elements
 * within a slide or across slides, either as a function of time or
 * in response to keyboard events.
 * 
 * A simple example:
 @code
// Move a circle left or right in response to left and right keys
import Animate 1.0
key "Right" -> skip 1
key "Left"  -> skip -1
compute_smooth_step
circle 100 * smooth_step, 0, 20
refresh 0.01
 @endcode
 * @{
 */


/**
 * Interpolate a variable
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change @a S.
 */
interpolate(real S, real TargetX, real X);

/**
 * Interpolate three variables
 *
 * Interpolate variable @a X, @a Y and @a Z to reach values
 * @a TargetX, @a TargetY and @a TargetZ with speed of change @a S.
 */
interpolate(real S, real TargetX, real TargetY, real TargetZ, real X, real Y, real Z);

/**
 * Interpolate a position
 *
 * Interpolate variable @a X, @a Y and @a Z to reach values
 * @a TargetX, @a TargetY and @a TargetZ with speed of change @a S, and use the
 * result as a position. After the call, the coordinate systems is
 * centered on that new position.
 */
interpolate_position(real S, real TargetX, real TargetY, real TargetZ, real X, real Y, real Z);

/**
 * Interpolate a scale along X, Y and Z axis
 *
 * Interpolate variable @a Scale to reach value @a TargetScale with
 * speed of change @a S, and use the result as a global scale along X,
 * Y and Z axis.
 *
 * The @ref interpolate_scale2 performs the same operation on X and Y only
 */
interpolate_scale(real S, real TargetScale, real Scale)

/**
 * Interpolate a scale along X and Y axis
 *
 * Interpolate variable @a Scale to reach value @a TargetScale with
 * speed of change @a S, and use the result as a global scale along X,
 * and Y axis.
 *
 * The @ref interpolate_scale performs the same operation on X, Y and Z
 */
interpolate_scale2(real S, real TargetScale, real Scale)

/**
 * Interpolate a rotation around X axis
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the X axis by
 * @a X degrees.
 */
interpolate_xangle(real S, real TargetX, real X);

/**
 * Interpolate a rotation around Y axis
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the Y axis by
 * @a X degrees.
 */
interpolate_yangle(real S, real TargetX, real X);

/**
 * Interpolate a rotation around Z axis
 *
 * Interpolate variable @a X to reach value @a TargetX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the Z axis by
 * @a X degrees.
 */
interpolate_zangle(real S, real TargetX, real X);


/**
 * Compute value to be used for fade-in effects
 *
 * Return a value that fades approximately from 0 to 1 over @a Duration
 * The @a Value argument is typically a time computed using @ref page_time
 * The typical usage will be something like:
 @code
 // Fade-in for approximately 4 seconds
 show fade_in(page_time, 4)
 circle 0, 0, 100
 @endcode
*/
real fade_in(real Value, real Duration);


/**
 * Compute value to be used for fade-out effects
 *
 * Return a value that fades approximately from 1 to 0 over @a Duration
 * The @a Value argument is typically a time computed using @ref page_time
 * The typical usage will be something like:
 @code
 // Fade-out for approximately 4 seconds
 show fade_out(page_time, 4)
 circle 0, 0, 100
 @endcode
*/
real fade_out(real Value, real Duration);


/**
 * Current animation step
 * 
 * This is the animation step set by @ref skip or @ref skip_to.
 */
real step = 0.0;

/**
 * Smoothed value of current animation step
 *
 * This is a smoothed value of the animation step set by
 * @ref skip or @ref skip_to.
 * This value is only computed after you call @ref compute_smooth_step
 */
real smooth_step = 0.0;

/**
 * Skip in the animation
 *
 * Skip by the given amount in the animation. Typically, positive values
 * are used to move forward in the animation, and negative values to move
 * backwards.
 */
skip(real Amount);

/**
 * Skip directly to a given step in the animation
 *
 * Skip to the given position in the animation. @see skip
 * backwards.
 */
skip_to(real Position);


/**
 * Zoom around a particular position in the animation
 *
 * Return a scaling factor that depends on how far @a N is from the current
 * position of the animation (as given by @ref step).
 */
step_scale(integer N);


/**
 * Compute the value of @ref smooth_step
 *
 */

compute_smooth_step();

/**
 * @}
 */
