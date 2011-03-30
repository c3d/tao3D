/**
 * @addtogroup WindowsAndFrame Windows and frames
 * @ingroup TaoBuiltins
 * Window size and frames.
 * Window size is the available space for drawing.
 * A frame texture is like a piece of paper where one can draw, 
 * and a frame is this piece of paper paste in a specific location.
 *
 * @image html "images/frame.png"
 * Example of primitives used to produce here after image : 
 * <a href="examples/frame.ddd">frame.ddd</a>
 *
 * @warning @ref frame_width and @ref frame_height are not returning the provided
 * width and height to the @ref frame_texture command, but they are returning the
 * computed value of the bounding box of the frame's layout.
 *
 * @bug Computation of frame bounding box (requesting by @ref frame_width,
 * @ref frame_height, @ref frame_depth) is damaging the frame itself. 
 * <a href="http://nano.taodyne.com:8080/redmine/issues/860">bug#860</a>
 *
 * @{
 */
/**
 * Returns the height of the document window in pixels.
 */
window_height ();

/**
 * Returns the width of the document window in pixels.
 */
window_width ();

/**
 * Create a frame
 * Draw a frame with the described content in @p body. This frame as no displayed border.
 * It makes a texture of size (@p w, @p h) using @ref frame_texture, and map it on a rectangle
 * at (@p x, @p y) location.
 *
 */
frame( x:real, y:real, w:real, h:real, body:tree);

/**
 * Create a texture
 * Make a texture out of the described content
 */
frame_texture( w:real, h:real, body:tree);

/**
 * Frame depth
 * Returns the depth of the current layout frame.
 *
 * @bug frame is a 2D object...
 */
frame_depth ();

/**
 * Frame height
 * Returns the height of the current layout frame.
 *
 */
frame_height ();

/**
 * Frame width.
 * Returns the witdh of the current layout frame.
 *
 */
frame_width ();

/** @} */
