/**
 * @defgroup Graphics Graphics
 * @ingroup TaoBuiltins
 *
 * Tao Presentations relies on OpenGL (http://www.opengl.org/) to provide
 * a true 3D drawing environment. Altough some GL functions are directly
 * available through Tao primitives, Tao Presentations also provides more
 * ready-to-use graphics functions to draw usual geometric objects,
 * whether 2D or 3D.
 */


/**
 * @defgroup graph_env Graphical environment
 * @ingroup Graphics
 * Opening environment to manipulate graphical objects.
 * This groups also holds attribute information like color, line width, etc... 
 * @{
 */
/**
 * Creates a selectable shape
 *
 *  Evaluate the child and mark the current shape.
 *  The modification done to the environment is only applicable inside this shape as @ref locally is doing.
 *  Make the shape sensible to mouse events. 
 *  It enables click action (@ref on), and handles for selection, motion, rotation, resize...
 */
shape (t:tree);


/**
 * Makes the widget clickable.
 *
 *  Create a context for active widgets (like buttons) or drawing (circle, etc...)
 *  Make the shape sensible to mouse events. 
 *  It enables click action (@ref on), but does not enables handles for mouse manipulations.
 */
active_widget (t:tree);

/**
 * Anchors a set of shapes to the current position
 */
anchor (t:tree);

/**
 * Selects the line width for shape outlines.
 *
 * @a lw is the width in pixels
 */
line_width (lw:integer);


/** @} */

/**
 * @defgroup graph_path Graphical Path
 * @ingroup Graphics
 * Creation of drawing in 2D or 3D vertex by vertex
 * @{
 */


/**
 * Starts a graphic path.
 *
 * A graphic path is a collection of segments of lines or curves
 * (quadrics or cubics).
 */
path (t:tree);

/**
 * Moves path cursor to a specific point.
 *
 * Add a 'moveTo' to the current path.
 */
move_to (x:real, y:real, z:real);

/**
 * Adds a quadric segment to the current path.
 *
 * (@a cx, @a cy, @a cz) defines the control point.
 * (@a x, @a y, @a z) is the end point.
 */
quad_to (cx:real, cy:real, cz:real, x:real, y:real, z:real);

/**
 * Adds a cubic segment to the current path.
 *
 * (@a c1x, @a c1y, @a c1z) defines the first control point.
 * (@a c2x, @a c2y, @a c2z) defines the second control point.
 * (@a x, @a y, @a z) is the end point.
 */
cubic_to (c1x:real, c1y:real, c1z:real, c2x:real, c2y:real, c2z:real,
          x:real, y:real, z:real);

/**
 * Adds a line segment to the current path relative to current position.
 */
line_relative (x:real, y:real, z:real);

/**
 * Moves current path cursor relative to current position.
 */
move_relative (x:real, y:real, z:real);

/**
 * Closes current path by a straight line to the start point.
 */
close_path ();

/**
 * Add a texture coordinate to the path.
 * NOT SUPPORTED YET.
 * @bug Not supported yet.
 */
path_texture (x:real, y:real, z:real);


/**
 * Sets the style of the path endpoints.
 *
 * Supported values are:
 *   - ARROWHEAD
 *   - POINTER
 *   - DIAMOND
 *   - CIRCLE
 *   - SQUARE
 *   - BAR
 *   - CUP
 *   - FLETCHING
 *   - HOLLOW_CIRCLE
 *   - HOLLOW_SQUARE
 */
endpoints_style (s:symbol, e:symbol);

/**
 * Adds a color element to the path.
 *
 * NOT SUPPORTED YET.
 * @bug Not supported yet.
 */
path_color (r:real, g:real, b:real, a:real);

/**
 * Adds a line segment to the current path.
 */
line_to (x:real, y:real, z:real);


/** @} */


/**
 * @defgroup graph_2D 2D shapes
 * @ingroup Graphics
 * Creating 2D shapes.
 *
 * All 2D primitives define two-dimensional shapes, but these shapes can be
 * further translated, rotated and scaled in 3D space. When a 2D shape is
 * defined, its z coordinate is set to 0.
 *
 * In all the 2D primitives,
 * -  @a x, and @a y represent the center of the shape. @a x > 0 goes toward
 *    the right, @a y > goes toward the top
 * -  @a h, and @a w represent the width and the height of the shape.
 *
 * @{
 */

/**
 * Draws a point.
 *
 * This primitive draws an OpenGL point. @a s is the point size in
 * pixels. It is @b not affected by the scaling factor.
 */
point (x:real, y:real, z:real, s:real);

/**
 * Draws a rectangle.
 *
 * The rectangle is centered at (@a x, @a y), with width @a w and height @a h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top left corner is at coordinate (x-w/2, y+h/2)
 *  - top right corner is at coordinate (x+w/2, y+h/2)
 *
 * Example: <a href="examples/rectangle.ddd">rectangle.ddd</a>
 * @image html rectangle.png "Rectangle"
 */
rectangle (x:real, y:real, w:real, h:real);

/**
 * Draws a rounded rectangle.
 *
 * The rectangle is centered at (@a x, @a y), with width @a w and
 * height @a h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top left corner is at coordinate (x-w/2, y+h/2)
 *  - top right corner is at coordinate (x+w/2, y+h/2)
 * The rounded corners have radius @a r.
 *
 * Example: <a href="examples/rounded_rectangle.ddd">rounded_rectangle.ddd</a>
 * @image html rounded_rectangle.png "Rounded rectangle"
 */
rounded_rectangle (x:real, y:real, w:real, h:real, r:real);


/**
 * Draws a rectangle with elliptical sides.
 *
 * The rectangle is centered at (@a x, @a y), with width @a w and
 * height @a h.
 * The ratio @a r is a real between 0.0 and 1.0. With ratio 0.0 the elliptical
 * rectangle is an ellipse, and with ratio 1.0 the elliptical rectangle is a
 * rectangle.
 * Example:
 * <a href="examples/elliptical_rectangle.ddd">elliptical_rectangle.ddd</a>
 * @image html elliptical_rectangle.png "Elliptical rectangle"
 */
elliptical_rectangle (x:real, y:real, w:real, h:real, r:real);

/**
 * Draws an ellipse.
 *
 * The ellipse is centered at (@a x, @a y) with width @a w and height @a h.
 *
 * Example: <a href="examples/ellipse.ddd">ellipse.ddd</a>
 * @image html ellipse.png "Ellipse sample"
 */
ellipse (x:real, y:real, w:real, h:real);

/**
 * Draws an elliptic sector.
 *
 * The elliptic sector is inside a rectangle centered at (@a x, @a y).
 * It begins at an angle of @a start degrees and extends
 * @a sweep degrees counter-clockwise. Angles are specified in
 * degrees. Clockwise arcs can be specified using negative angles.
 *
 * Example: <a href="examples/ellipse_arc.ddd">ellipse_arc.ddd</a>
 * @image html ellipse_arc.png "Ellipse arc"
 */
ellipse_arc (x:real, y:real, w:real, h:real, start:real, sweep:real);

/**
 * Draws an isoceles triangle.
 *
 * The triangle is centered at (@a x, @a y), has width @a w and
 * height @a h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - Top corner is at coordinate (x, y+h/2)
 *
 * Example: <a href="examples/triangle.ddd">triangle.ddd</a>
 * @image html triangle.png "Isoceles triangle"
 */
triangle (x:real, y:real, w:real, h:real);

/**
 * Draws a right triangle.
 *
 * The middle of the hypotenuse is at @c (x,y). The width of the triangle is
 * @a w and its height is @a h.
 * The right angle is the bottom left one.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top corner is at coordinate (x-w/2, y+h/2)
 *
 * Example: <a href="examples/right_triangle.ddd">right_triangle.ddd</a>
 * @image html right_triangle.png "Right triangle sample"
 */
right_triangle (x:real, y:real, w:real, h:real);


/**
 * Draws an arrow.
 *
 * The arrow is centered at (@a x, @a y) and is contained in a bounding box of
 * @a w by @a h pixels. @a head is the length of the arrow head in pixels.
 * @a tail is the size of the the arrow relative to the overall witdh. @a tail
 * is comprised between 0 and 1.
 *
 * Example: <a href="examples/arrow.ddd">arrow.ddd</a>
 * @image html arrow.png "Arrow"
 *
 * @todo why head is in px and tail is a ratio?
 */
arrow (x:real, y:real, w:real, h:real, head:real, tail:real);

/**
 * Draws a double arrow.
 *
 * Similar to arrow (x:real, y:real, w:real, h:real, head:real, tail:real)
 * but with two heads.
 * Example: <a href="examples/double_arrow.ddd">double_arrow.ddd</a>
 * @image html double_arrow.png "Double arrow"
 */
double_arrow (x:real, y:real, w:real, h:real, ax:real, ay:real);

/**
 * Draws a star.
 *
 * This primitive defines a regular @a p-branch star centered at (@a x,@a y).
 * Width is @a w, height is
 * @a h. @a r is a real between 0 and 1 that defines the aspects of the
 * branches. When @a r approaches 0, the branches get thinner. When @a r
 * gets closer to 1, the star degenerates into a regular polygon.
 *
 *
 * Example: <a href="examples/star.ddd">star.ddd</a>
 * @image html star.png "Star sample"
 */
star (x:real, y:real, w:real, h:real, p:integer, r:real);

/**
 * Draws a star.
 *
 * This primitive draws a regular star polygon centered at (@a x,@a y).
 * The star is obtained by placing @a p vertices regularly spaced on a
 * circle, to form a regular polygon. Then every </a>q</a>th vertices are
 * connected together.
 *
 * Example: <a href="examples/star_polygon.ddd">star_polygon.ddd</a>
 * @image html star_polygon.png "Star polygon"
 */
star_polygon (x:real, y:real, w:real, h:real, p:integer, q:integer);

/**

 * Draws a speech balloon.
 *
 * A speech ballon is made of:
 *   - a rounded rectangle centered at (@a x, @a y), with width @a w,
 *     height @a h and radius @a r, and
 *   - a tail ending at point (@a ax, @a ay). The tail width is determined
 *     automatically. The tail is not made of straight lines; it has a
 *     rounded shape. It is directed from the end point to the center of
 *     the speech balloon.
 *
 * Example: <a href="examples/speech_balloon.ddd">speech_balloon.ddd</a>
 * @image html speech_balloon.png "Speech balloon"
 */
speech_balloon (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real);

/**
 * Draws a callout.
 *
 * A callout is made of:
 *   - a rounded rectangle centered at (@a x, @a y), with width @a w,
 *     height @a h and radius @a r, and
 *   - a triangular tail, which connects point (@a ax, @a ay) to the
 *     closest point on the rounded rectangle. The base of the tail has
 *     a width of @a tw pixels.
 *
 * Example: <a href="examples/callout.ddd">callout.ddd</a>
 * @image html callout.png "Callout"
 */
callout (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real, tw:real);


/** @} */

/**
 * @defgroup graph_3D 3D shapes
 * @ingroup Graphics
 * Creating 3D shapes.
 *
 * In all the 3D primitives,
 * -  @a x, @a y, and @a z represent the center of the shape, @a x > 0 goes
 *    toward the right, @a y > goes toward the top, and @a z > 0 goes toward
 *    the user.
 * -  @a h, @a w, and @a d represent the width, height and depth of the shape,
 *    respectively.
 *
 * @{
 */

/**
 * Draws a sphere.
 *
 * The sphere is divided in @a slices and @a stacks. The higher the vaue of
 * these parametres, the smoother the sphere.
 */
sphere (x:real, y:real, z:real, w:real, h:real, d:real, slices:integer, stacks:integer);

/**
 * Draws a cone.
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 */
cone (x:real, y:real, z:real, w:real, h:real, d:real);

/**
 * Draws a truncated cone.
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 * @param r [real] percentage of basis : 0.0 is a cone; 1.0 is a cylinder.
 */
truncated_cone (x:real, y:real, z:real, w:real, h:real, d:real, r:real);

/**
 * Draws a cube.
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 */
cube (x:real, y:real, z:real, w:real, h:real, d:real);

/**
 * Drwas a cyinder.
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 */
cylinder (x:real, y:real, z:real, w:real, h:real, d:real);


/** @} */
