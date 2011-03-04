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
 * @fn shape ( t)
 * @brief Make the shape selectable
 *
 *  Evaluate the child and mark the current shape.
 *  The modification done to the environment is only applicable inside this shape as @ref locally is doing.
 *  Make the shape sensible to mouse events. 
 *  It enables click action (@ref on), and handles for selection, motion, rotation, resize...
 *
 * @param t [tree] The code of the shape.
 * @returns The result of the child execution.
 */
shape ( t);


/**
 * @fn active_widget ( t)
 * @brief Make the widget clickable
 *
 *  Create a context for active widgets (like buttons) or drawing (circle, etc...)
 *  Make the shape sensible to mouse events. 
 *  It enables click action (@ref on), but does not enables handles for mouse manipulations.
 *
 * @param t [tree] The code that represent the widget.
 * @returns The result of the child execution.
 */
active_widget ( t);

/**
 * @fn anchor ( t)
 * @brief 
 *
 *  Anchor a set of shapes to the current position
 *
 * @param t [tree] The code that represent the shapes to be anchored.
 * @returns The result of the child execution.
 */
anchor ( t);

/**
 * @fn line_width ( lw)
 * @brief Select the line width for OpenGL
 *
 *  Set the line width for OpenGL
 *
 * @param lw [real] The line width to set.
 */
line_width ( lw);


/** @} */

/**
 * @defgroup graph_path Graphical Path
 * @ingroup Graphics
 * Creation of drawing in 2D or 3D vertex by vertex
 * @{
 */


/**
 * 
 * @fn move_to ( x, y, z)
 * @brief move to point
 *
 *  Add a 'moveTo' to the current path
 *
 * @param x [real] x-coordinate of the point to move to.
 * @param y [real] y-coordinate of the point to move to.
 * @param z [real] z-coordinate of the point to move to.
 */
move_to ( x, y, z);
/**
 * @fn quad_to ( cx, cy, cz, x, y, z)
 * @brief  quadric curveTo
 *
 *  Add a quadric curveTo to the current path.
 *
 * @param cx [real] x-coordinate of the control point.
 * @param cy [real] y-coordinate of the control point.
 * @param cz [real] z-coordinate of the control point.
 * @param x [real] x-coordinate of the point to finish to.
 * @param y [real] y-coordinate of the point to finish to.
 * @param z [real] z-coordinate of the point to finish to.
 */
quad_to ( cx, cy, cz, x, y, z);

/**
 * @fn line_relative ( x, y, z)
 * @brief relative line to
 *
 *  Add a relative 'lineTo' to the current path
 *
 * @param x [real] delta to line to on x-axis.
 * @param y [real] delta to line to on y-axis.
 * @param z [real] delta to line to on z-axis.
 * @returns 
 */
line_relative ( x, y, z);

/**
 * @fn path_texture ( x, y, z)
 * @brief Not supported yet
 *
 *   NOT SUPPORTED YET. Add a texture coordinate to the path. NOT SUPPORTED YET
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 */
path_texture ( x, y, z);


/**
 * @fn endpoints_style ( s, e)
 * @brief style of the path endpoints
 *
 *  Specify the style of the path endpoints. Supported values are : ARROWHEAD, ARROWHEAD, POINTER, DIAMOND, CIRCLE, SQUARE, BAR, CUP, FLETCHING, HOLLOW_CIRCLEHOLLOW_SQUARE.
 *
 * @param s [symbol] start endpoint style
 * @param e [symbol] end endpoint style
 * @returns 
 */
endpoints_style ( s, e);

/**
 * @fn path_color ( r, g, b, a)
 * @brief Not supported yet
 *
 *  NOT SUPPORTED YET. Add a color element to the path. NOT SUPPORTED YET. 
 *
 * @param r [real] The red componant of the color
 * @param g [real] The blue componant of the color
 * @param b [real] The green componant of the color
 * @param a [real] Alpha channel: the transparency of the color. 
 */
path_color ( r, g, b, a);

/**
 * @fn cubic_to ( c1x, c1y, c1z, c2x, c2y, c2z, x, y, z)
 * @brief cubic curveTo
 *
 *  Add a cubic curveTo to the current path
 *
 * @param c1x [real] x-coordinate of the first control point.
 * @param c1y [real] y-coordinate of the first control point.
 * @param c1z [real] z-coordinate of the first control point.
 * @param c2x [real] x-coordinate of the second control point.
 * @param c2y [real] y-coordinate of the second control point.
 * @param c2z [real] z-coordinate of the second control point.
 * @param x [real] x-coordinate of the point to finish to.
 * @param y [real] y-coordinate of the point to finish to.
 * @param z [real] z-coordinate of the point to finish to.
 */
cubic_to ( c1x, c1y, c1z, c2x, c2y, c2z, x, y, z);

/**
 * @fn move_relative ( x, y, z)
 * @brief relative moveTo
 *
 *  Add a relative moveTo
 *
 * @param x [real] delta to move to on x-axis.
 * @param y [real] delta to move to on y-axis.
 * @param z [real] delta to move to on z-axis.
 */
move_relative ( x, y, z);

/**
 * @fn path ( t)
 * @brief Start a path
 *
 *  Evaluate the child tree within a polygon
 *
 * @param t [tree] The definition of the path.
 * @returns The result of the child evaluation.
 */
path ( t);


/**
 * @fn close_path ()
 * @brief close the path
 *
 *  Close the path back to its origin
 */
close_path ();

/**
 * @fn line_to ( x, y, z)
 * @brief line to point
 *
 *  Add a 'lineTo' to the current path
 *
 * @param x [real] x-coordinate of the point to line to.
 * @param y [real] y-coordinate of the point to line to.
 * @param z [real] z-coordinate of the point to line to.
 */
line_to ( x, y, z);


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
 * @fn point ( x, y, z, s)
 * @brief Draw a point
 *
 *  Draw a point with the given size. It is not affected by the scaling factor.
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param s [real] THe size of the point
 */
point ( x, y, z, s);

/**
 * @fn rectangle ( x, y, w, h)
 * @brief Draw a rectangle
 *
 *  Draw a rectangle centered in @c (x,y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top left corner is at coordinate (x-w/2, y+h/2)
 *  - top right corner is at coordinate (x+w/2, y+h/2)
 *
 *  @image html rectangle.png "Rectangle sample"
 *
 * @code
 * color "orange"
 * rectangle 0, 0, 100, 200
 * @endcode
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the rectangle
 * @param h [real] height of the rectangle
 */
rectangle ( x, y, w, h);

/**
 * @fn rounded_rectangle ( x, y, w, h, r)
 * @brief Rounded rectangle
 *
 *  Draw a rounded rectangle with radius r for the rounded corners.
 *  The rectangle is centered in @c (x,y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top left corner is at coordinate (x-w/2, y+h/2)
 *  - top right corner is at coordinate (x+w/2, y+h/2)
 *
 *  @image html rounded_rectangle.png "Rounded rectangle sample"
 *
 * @code
 * color "orange"
 * rounded_rectangle 0, 0, 100, 200, 25
 * @endcode
 *
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the rectangle
 * @param h [real] height of the rectangle
 * @param r [real] corner radius
 */
rounded_rectangle ( x, y, w, h, r);


/**
 * @fn elliptical_rectangle ( x, y, w, h, r)
 * @brief Rectangle with elliptical side
 *
 *  Elliptical rectangle with ratio r for the elliptic sides. The ratio is a real between 0.0 and 1.0. With ratio 0.0 the elliptical rectangle is an ellipse, and with ratio 1.0 the elliptical rectangle is a rectangle.
 *  @image html elliptical_rectangle.png "Elliptical rectangle sample"
 *
 * @code
 * color "orange"
 * rounded_rectangle 0, 0, 100, 200, 25
 * @endcode
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the rectangle
 * @param h [real] height of the rectangle
 * @param r [real] ratio of the ellpitic sides [0.0, 1.0]
 */
elliptical_rectangle ( x, y, w, h, r);

/**
 * @fn ellipse ( x, y, w, h)
 * @brief Ellipse
 *
 *  Draw an ellipse centered around @c (x,y) with size <tt>w * h</tt>.
 *
 *  @image html ellipse.png "Ellipse sample"
 *
 * @code
 * color "orange"
 * ellipse 0, 0, 100, 200
 * @endcode
 *
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the ellipse
 * @param h [real] height of the ellipse
 */
ellipse ( x, y, w, h);

/**
 * @fn ellipse_arc ( x, y, w, h, start, sweep)
 * @brief Elliptic sector
 *
 *  Elliptic sector centered around @c (x,y) that occupies the given rectangle, beginning at the specified startAngle and extending sweepLength degrees counter-clockwise. Angles are specified in degrees. Clockwise arcs can be specified using negative angles.
 *
 *  @image html ellipse_arc.png "Ellipse arc sample"
 *
 * @code
 * color "orange"
 * ellipse_arc 0, 0, 200, 290, 20, 135
 * @endcode
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the ellipse
 * @param h [real] height of the ellipse
 * @param start [real] start angle express in degrees
 * @param sweep [real] sweep angle express in degrees
 */
ellipse_arc ( x, y, w, h, start, sweep);

/**
 * @fn triangle ( x, y, w, h)
 * @brief Isoceles triangle
 *
 *  Draw an isoceles triangle centered in @c (x,y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top corner is at coordinate (x, y+h/2)
 *
 *  @image html triangle.png "Isoceles triangle sample"
 *
 * @code
 * color "orange"
 * triangle 0, 0, 150, 200
 * @endcode
 *
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] base of the triangle
 * @param h [real] height of the triangle
 */
triangle ( x, y, w, h);

/**
 * @fn right_triangle ( x, y, w, h)
 * @brief Right triangle
 *
 *  Draw a right triangle with hypotenuse centered in @c (x,y), with width @p w and height @p h.
 *  Right angle is the bottom left one.
 *  - Bottom left corner is at coordinate (x-w/2, y-h/2)
 *  - Bottom right corner is at coordinate (x+w/2, y-h/2)
 *  - top corner is at coordinate (x-w/2, y+h/2)
 *
 *  @image html right_triangle.png "Right triangle sample"
 *
 * @code
 * color "orange"
 * right_triangle 0, 0, 150, 200
 * @endcode
 *
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] base of the triangle
 * @param h [real] height of the triangle
 */
right_triangle ( x, y, w, h);


/**
 * @fn arrow ( x, y, w, h, head, bodyRatio)
 * @brief Draw an arraw
 *
 *  Draw an arraw with adjustable head and depth and body width.
 *
 *  @image html arrow.png "Arrow sample"
 *
 * @code
 * color "orange"
 * arrow 0, 0, 250, 200, 60, 0.5
 * @endcode
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of surrounding rectangle
 * @param h [real] height of surrounding rectangle
 * @param head [real] length of the arrow's head
 * @param bodyRatio [real] ratio between the full width and the body width [0.0, 1.0]
 */
arrow ( x, y, w, h, head, bodyRatio);

/**
 * @fn double_arrow ( x, y, w, h, ax, ay)
 * @brief Double arraw
 *
 *  Draw a symetric double arraw
 *
 *  @image html double_arrow.png "Double arrow sample"
 *
 * @code
 * color "orange"
 * double_arrow 0, 0, 250, 200, 60, 0.5
 * @endcode
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of surrounding rectangle
 * @param h [real] height of surrounding rectangle
 * @param head [real] length of the arrow's head
 * @param bodyRatio [real] ratio between the full width and the body width [0.0, 1.0]
 */
double_arrow ( x, y, w, h, ax, ay);

/**
 * @fn star ( x, y, w, h, p, r)
 * @brief Draw a star
 *
 *  GL regular p-side star centered around (x,y) with inner radius ratio r.
 *
 *  @image html star.png "Star sample"
 *
 * @code
 * color "orange"
 * star 0, 0, 250, 200, 8, 0.5 
 * @endcode
 *
 *
 * @param x [real] center x-coordinate
 * @param y [real] center y-coordinate
 * @param w [real] width of the star
 * @param h [real] height of the star
 * @param p [integer] Number of branch
 * @param r [real] inner radius ratio
 */
star ( x, y, w, h, p, r);

/**
 * @fn star_polygon ( x, y, w, h, p, q)
 * @brief Draw a star
 *
 *  GL regular p-side star polygon centered around (x,y).
 *  @p p is the number of branch. Each side of a branch is drawn by aimed at another vertex. This other vertex is the @p qth starting from the current one.
 *
 *  @image html star_polygon.png "Star polygon sample"
 *
 * @code
 * color "orange"
 * star_polygon 0, 0, 250, 200, 16, 6 
 * @endcode
 *
 * @param x [real] center x-coordinate
 * @param y [real] center y-coordinate
 * @param w [real] width of the star
 * @param h [real] height of the star
 * @param p [integer] Number of branch
 * @param q [integer] Number of vertex to skip for sighting
 */
star_polygon ( x, y, w, h, p, q);

/**
 * @fn speech_balloon ( x, y, w, h, r, ax, ay)
 * @brief Speech balloon
 *
 *  Speech balloon with radius r for rounded corners, and point a for the tail. The tail width is computed.
 *
 *  @image html speech_balloon.png "Speech balloon sample"
 *
 * @code
 * color "orange"
 * speech_balloon  0, 0, 200, 100, 25, 35, -130
 * @endcode
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param w [real] width of the balloon
 * @param h [real] height of the balloon
 * @param r [real] rounded rectangle radius
 * @param ax [real] tail end x-coordinate
 * @param ay [real] tail end y-coordinate
 */
speech_balloon ( x, y, w, h, r, ax, ay);

/**
 * @fn callout ( x, y, w, h, r, ax, ay, tw)
 * @brief callout
 *
 *  Callout with radius r for corners, and point a for the head of the tip and, tw as the tip basis width.
 *
 *  @image html callout.png "Callout sample"
 *
 * @code
 * color "orange"
 * callout 0, 0, 200, 100, 25, 35, -130, 25
 * @endcode
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
callout ( x, y, w, h, r, ax, ay, tw);


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
 * @fn object ( x, y, z, w, h, d, f)
 * @brief Load a 3D object
 *
 *  Load a 3D object
 * @todo Est-ce deja documente ?
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 * @param f [text] filename
 */
object ( x, y, z, w, h, d, f);

/**
 * @fn sphere ( x, y, z, w, h, d, slices, stacks)
 * @brief sphere like
 *
 *  GL sphere [TODO or ovoid ? Add the incidence of slices and stacks on the sphere rendering time.]
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 * @param slices [integer] number of slices
 * @param stacks [integer] [TODO]
 */
sphere ( x, y, z, w, h, d, slices, stacks);

/**
 * @fn cone ( x, y, z, w, h, d)
 * @brief cone
 *
 *  cone[TODO]
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 */
cone ( x, y, z, w, h, d);

/**
 * @fn truncated_cone ( x, y, z, w, h, d, r)
 * @brief truncated cone
 *
 *  truncated cone[TODO]
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 * @param r [real] percentage of basis : 0.0 is a cone; 1.0 is a cylinder.
 */
truncated_cone ( x, y, z, w, h, d, r);

/**
 * @fn cube ( x, y, z, w, h, d)
 * @brief cube like
 *
 *  parallelepiped rectangle
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 */
cube ( x, y, z, w, h, d);

/**
 * @fn cylinder ( x, y, z, w, h, d)
 * @brief cylinder
 *
 *  cylinder[TODO]
 *
 * @param x [real] x-coordinate
 * @param y [real] y-coordinate
 * @param z [real] z-coordinate
 * @param w [real] width
 * @param h [real] height
 * @param d [real] depth
 */
cylinder ( x, y, z, w, h, d);


/** @} */
