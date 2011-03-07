/**
 * @defgroup Graphics
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
 * @li ARROWHEAD
 * @li POINTER
 * @li DIAMOND
 * @li CIRCLE
 * @li SQUARE
 * @li BAR
 * @li CUP
 * @li FLETCHING
 * @li HOLLOW_CIRCLE
 * @li HOLLOW_SQUARE
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
 * @defgroup graph_2D 2D drawings
 * @ingroup Graphics
 * Creating 2D shapes.
 *
 * All 2D primitives define two-dimensional shapes, but these shapes can be
 * further translated, rotated and scaled in 3D space. When a 2D shape is
 * defined, its z coordinate is set to 0.
 *
 * In all the 2D primitives,
 * -  @p x, and @p y represent the center of the drawing, x > 0 goes toward the right, y > goes toward the top
 * -  @p h, and @p w represent the width and the height of the drawing
 *
 * @{
 */

/**
 * Draws a point.
 *
 * The point size is not affected by the scaling factor.
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param s [real] THe size of the point
 */
point (x:real, y:real, z:real, s:real);

/**
 * Draws a rectangle.
 *
 *  Draw a rectangle centered in @c (x,y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top left corner is at coordinate (x-w/2, y+h/2)
 *  - top right corner is at coordinate (x+w/2, y+h/2)
 *
 * Example: <a href="examples/rectangle.ddd">rectangle.ddd</a>
 * @image html rectangle.png "Rectangle"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the rectangle
 * @param h [real] height of the rectangle
 */
rectangle (x:real, y:real, w:real, h:real);

/**
 * Draws a rounded rectangle.
 *
 *  Draw a rounded rectangle with radius r for the rounded corners.
 *  The rectangle is centered in @c (x,y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top left corner is at coordinate (x-w/2, y+h/2)
 *  - top right corner is at coordinate (x+w/2, y+h/2)
 *
 * Example: <a href="examples/rounded_rectangle.ddd">rounded_rectangle.ddd</a>
 * @image html rounded_rectangle.png "Rounded rectangle"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the rectangle
 * @param h [real] height of the rectangle
 * @param r [real] corner radius
 */
rounded_rectangle (x:real, y:real, w:real, h:real, r:real);


/**
 * Draws a rectangle with elliptical sides.
 *
 * The ratio @a r is a real between 0.0 and 1.0. With ratio 0.0 the elliptical
 * rectangle is an ellipse, and with ratio 1.0 the elliptical rectangle is a
 * rectangle.
 * Example:
 * <a href="examples/elliptical_rectangle.ddd">elliptical_rectangle.ddd</a>
 * @image html elliptical_rectangle.png "Elliptical rectangle"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the rectangle
 * @param h [real] height of the rectangle
 * @param r [real] ratio of the ellpitic sides [0.0, 1.0]
 */
elliptical_rectangle (x:real, y:real, w:real, h:real, r:real);

/**
 * Draws an ellipse.
 *
 *  Draw an ellipse centered around @c (x,y) with size <tt>w * h</tt>.
 *
 * Example: <a href="examples/ellipse.ddd">ellipse.ddd</a>
 * @image html ellipse.png "Ellipse sample"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the ellipse
 * @param h [real] height of the ellipse
 */
ellipse (x:real, y:real, w:real, h:real);

/**
 * Draws an elliptic sector.
 *
 * Elliptic sector centered around (@a x, @a y) that occupies the given
 * rectangle, beginning at the specified @a startAngle and extending
 * @a sweepLength degrees counter-clockwise. Angles are specified in
 * degrees. Clockwise arcs can be specified using negative angles.
 *
 * Example: <a href="examples/ellipse_arc.ddd">ellipse_arc.ddd</a>
 * @image html ellipse_arc.png "Ellipse arc"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the ellipse
 * @param h [real] height of the ellipse
 * @param start [real] start angle express in degrees
 * @param sweep [real] sweep angle express in degrees
 */
ellipse_arc (x:real, y:real, w:real, h:real, start:real, sweep:real);

/**
 * Draws an isoceles triangle.
 *
 * Draws an isoceles triangle centered at (@a x, @a y), with width @a w and
 * height @a h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - Top corner is at coordinate (x, y+h/2)
 *
 * Example: <a href="examples/triangle.ddd">triangle.ddd</a>
 * @image html triangle.png "Isoceles triangle"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] base of the triangle
 * @param h [real] height of the triangle
 */
triangle (x:real, y:real, w:real, h:real);

/**
 * Draws a right triangle.
 *
 *  Draw a right triangle with hypotenuse centered in @c (x,y), with width @p w and height @p h.
 *  Right angle is the bottom left one.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top corner is at coordinate (x-w/2, y+h/2)
 *
 * Example: <a href="examples/right_triangle.ddd">right_triangle.ddd</a>
 * @image html right_triangle.png "Right triangle sample"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] base of the triangle
 * @param h [real] height of the triangle
 */
right_triangle (x:real, y:real, w:real, h:real);


/**
 * Draws an arraw.
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
 * Creates a double arraw.
 *
 *  Draw a symetric double arraw
 *
 * Example: <a href="examples/double_arrow.ddd">double_arrow.ddd</a>
 * @image html double_arrow.png "Double arrow"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of surrounding rectangle
 * @param h [real] height of surrounding rectangle
 * @param head [real] length of the arrow's head
 * @param bodyRatio [real] ratio between the full width and the body width [0.0, 1.0]
 */
double_arrow (x:real, y:real, w:real, h:real, ax:real, ay:real);

/**
 * Draws a star.
 *
 * GL regular p-side star centered around (x,y) with inner radius ratio r.
 *
 * Example: <a href="examples/star.ddd">star.ddd</a>
 * @image html star.png "Star sample"
 *
 * @param x [real] center x-coordinate
 * @param y [real] center y-coordinate
 * @param w [real] width of the star
 * @param h [real] height of the star
 * @param p [integer] Number of branch
 * @param r [real] inner radius ratio
 */
star (x:real, y:real, w:real, h:real, p:integer, r:real);

/**
 * Draws a star.
 *
 * GL regular p-side star polygon centered around (x,y).
 * @p p is the number of branch. Each side of a branch is drawn by aimed at
 * another vertex. This other vertex is the @p qth starting from the current one.
 *
 * Example: <a href="examples/star_polygon.ddd">star_polygon.ddd</a>
 * @image html star_polygon.png "Star polygon"
 *
 * @param x [real] center x-coordinate
 * @param y [real] center y-coordinate
 * @param w [real] width of the star
 * @param h [real] height of the star
 * @param p [integer] Number of branch
 * @param q [integer] Number of vertex to skip for sighting
 */
star_polygon (x:real, y:real, w:real, h:real, p:integer, q:integer);

/**

 * Creates a speech balloon.
 *
 * Speech balloon with radius @a r for rounded corners, and point a for the tail.
 * The tail width is computed.
 *
 * Example: <a href="examples/speech_balloon.ddd">speech_balloon.ddd</a>
 * @image html speech_balloon.png "Speech balloon"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the balloon
 * @param h [real] height of the balloon
 * @param r [real] rounded rectangle radius
 * @param ax [real] tail end x-coordinate
 * @param ay [real] tail end y-coordinate
 */
speech_balloon (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real);

/**
 * Creates a callout.
 *
 * Callout with radius r for corners, and point a for the head of the tip and, tw as the tip basis width.
 *
 * Example: <a href="examples/callout.ddd">callout.ddd</a>
 * @image html callout.png "Callout"
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the callout
 * @param h [real] height of the callout
 * @param r [real] radius
 * @param ax [real] x-coordinate of point a
 * @param ay [real] y-coordinate of point a
 * @param tw [real] tip width
 */
callout (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real, tw:real);


/** @} */

/**
 * @defgroup graph_3D 3D drawings
 * @ingroup Graphics
 * Creating 3D shapes.
 *
 * In all the 3D primitives,
 * -  @p x, @p y, and @p z represent the center of the drawing, x > 0 goes toward the right, y > goes toward the top, and z > 0 goes toward the user.
 * -  @p h, @p w, and @p d represent the width, the height and the depth of the drawing
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
