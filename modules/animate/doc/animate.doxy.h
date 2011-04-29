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
 * Interpolate variable @a X to reach value @a TX with speed of change @a S.
 */
interpolate(S:real, TX:real, X:real);

/**
 * Interpolate three variables
 *
 * Interpolate variable @a X, @a Y and @a Z to reach values
 * @a TX, @a TY and @a TZ with speed of change @a S.
 */
interpolate(S:real, TX:real, TY:real, TZ:real, X:real, Y:real, Z:real);

/**
 * Interpolate a position
 *
 * Interpolate variable @a X, @a Y and @a Z to reach values
 * @a TX, @a TY and @a TZ with speed of change @a S, and use the
 * result as a position. After the call, the coordinate systems is
 * centered on that new position.
 */
interpolate_position(S:real, TX:real, TY:real, TZ:real, X:real, Y:real, Z:real);

/**
 * Interpolate a rotation around X axis
 *
 * Interpolate variable @a X to reach value @a TX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the X axis by
 * @a X degrees.
 */
interpolate_xangle(S:real, TX:real, X:real);

/**
 * Interpolate a rotation around Y axis
 *
 * Interpolate variable @a X to reach value @a TX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the Y axis by
 * @a X degrees.
 */
interpolate_yangle(S:real, TX:real, X:real);

/**
 * Interpolate a rotation around Z axis
 *
 * Interpolate variable @a X to reach value @a TX with speed of change @a S,
 * then use the resulting value as a rotation angle. After the call,
 * the coordinate systems will have been rotated along the Z axis by
 * @a X degrees.
 */
interpolate_zangle(S:real, TX:real, X:real);


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
real fade_in(Value:real, Duration:real);


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
real fade_out(Value:real, Duration:real);


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
skip(Amount:real);

/**
 * Skip directly to a given step in the animation
 *
 * Skip to the given position in the animation. @see skip
 * backwards.
 */
skip_to(Position:real);


/**
 * Zoom around a particular position in the animation
 *
 * Return a scaling factor that depends on how far @a N is from the current
 * position of the animation (as given by @ref step).
 */
step_scale(N:integer);


/**
 * Compute the value of @ref smooth_step
 *
 */

compute_smooth_step();

/**
 * @}
 */
