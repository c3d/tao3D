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
 *  The modification done to the environment is only applicable inside this shape as @ref locally does.
 *
 *  Make the shape sensible to mouse events. 
 *  It enables click action (@ref on), and handles for selection, motion, rotation, resize...
 */
shape (t:tree);


/**
 * Makes the widget clickable.
 *
 *  Create a context for active widgets (like buttons) or drawing (circle, etc...).
 *  Make the shape sensive to mouse events. 
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
 * @p lw is the width in pixels
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
 * Adds a line segment to the current path.
 */
line_to (x:real, y:real, z:real);

/**
 * Adds a quadric segment to the current path.
 *
 * (@p cx, @p cy, @p cz) defines the control point.
 * (@p x, @p y, @p z) is the end point.
 */
quad_to (cx:real, cy:real, cz:real, x:real, y:real, z:real);

/**
 * Adds a cubic segment to the current path.
 *
 * (@p c1x, @p c1y, @p c1z) defines the first control point.
 * (@p c2x, @p c2y, @p c2z) defines the second control point.
 * (@p x, @p y, @p z) is the end point.
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
 * Sets the style of the path endpoints.
 *
 * @p s is the start point and @p e is the end point. Only beginning and very end of the path can be styled. If a text is added at the end of the path, the end style won't be applied to the line end (because end of the line is not the end of the path).
 *
 * Supported values are:
 *   - NONE
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
 * Add a texture coordinate to the path.
 * NOT SUPPORTED YET.
 * @bug Not supported yet.
 */
path_texture (x:real, y:real, z:real);

/**
 * Adds a color element to the path.
 *
 * NOT SUPPORTED YET.
 * @bug Not supported yet.
 */
path_color (r:real, g:real, b:real, a:real);


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
 * -  @p x, and @p y represent the center of the drawing.
 *   - [0, x) x > 0 goes toward the right,
 *   - [0, y) y > 0 goes toward the top,
 * -  @p h, and @p w represent the width and the height in pixel of the drawing
 *
 * Example of 2D primitives : <a href="examples/2D_samples.ddd">2D_samples.ddd</a>
 *
 * Images keys:
 * - dark orange is the result of the documented primitive.
 * - black parts are the representations of the provided parameters.
 * - yellow parts are useful marks.
 * 
 * @{
 */

/**
 * Draws a point.
 *
 * This primitive draws an OpenGL point. @p s is the point size in
 * pixels. It is @b not affected by the scaling factor.
 */
point (x:real, y:real, z:real, s:real);

/**
 * Draws a rectangle.
 *
 *  The rectangle is centered at (@p x, @p y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate <tt>(x+w/2, y-h/2)</tt>
 *  - top left corner is at coordinate <tt>(x-w/2, y+h/2)</tt>
 *  - top right corner is at coordinate <tt>(x+w/2, y+h/2)</tt>
 *
 * @image html rectangle.png
 *
 */
rectangle (x:real, y:real, w:real, h:real);

/**
 * Draws a rounded rectangle.
 *
 *  Draw a rounded rectangle with radius @p r for the rounded corners.
 *  The rectangle is centered in @c (x,y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate <tt>(x+w/2, y-h/2)</tt>
 *  - top left corner is at coordinate <tt>(x-w/2, y+h/2)</tt>
 *  - top right corner is at coordinate <tt>(x+w/2, y+h/2)</tt>
 *
 * @image html rounded_rectangle.png 
 *
 */
rounded_rectangle (x:real, y:real, w:real, h:real, r:real);


/**
 * Draws a rectangle with elliptical sides.
 *
 * The ratio @p r is a real between 0.0 and 1.0. With ratio 0.0 the elliptical
 * rectangle is an ellipse, and with ratio 1.0 the elliptical rectangle is a
 * rectangle.
 * @image html elliptical_rectangle.png
 *
 * @param r [real] ratio of the ellpitic sides [0.0, 1.0]
 */
elliptical_rectangle (x:real, y:real, w:real, h:real, r:real);

/**
 * Draws an ellipse.
 *
 * The ellipse is centered at (@a x, @a y) with width @a w and height @a h.
 *
 * @image html ellipse.png
 *
 */
ellipse (x:real, y:real, w:real, h:real);

/**
 * Draws an elliptic sector.
 *
 * Elliptic sector centered around (@p x, @p y) that occupies the given
 * rectangle, beginning at the specified @p startAngle and extending
 * @p sweepLength degrees counter-clockwise. Angles are specified in
 * degrees. Clockwise arcs can be specified using negative angles.
 *
 * @image html ellipse_arc.png
 *
 * @param start [real] start angle express in degrees
 * @param sweep [real] sweep angle express in degrees
 */
ellipse_arc (x:real, y:real, w:real, h:real, start:real, sweep:real);

/**
 * Draws an isoceles triangle.
 *
 * Draws an isoceles triangle centered at (@p x, @p y), with width @p w and
 * height @p h.
 *  - Bottom left corner is at coordinate  <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate  <tt>(x+w/2, y-h/2)</tt>
 *  - Top corner is at coordinate  <tt>(x, y+h/2)</tt>
 *
 * @image html triangle.png
 *
 */
triangle (x:real, y:real, w:real, h:real);

/**
 * Draws a right triangle.
 *
 *  Draw a right triangle with hypotenuse centered in @c (x,y), with width @p w and height @p h.
 *  Right angle is the bottom left one.
 *  - Bottom left corner is at coordinate <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate  <tt>(x+w/2, y-h/2)</tt>
 *  - top corner is at coordinate  <tt>(x-w/2, y+h/2)</tt>
 *
 * @image html right_triangle.png
 *
 */
right_triangle (x:real, y:real, w:real, h:real);


/**
 * Draws an arrow.
 *
 * The arrow is centered at (@p x, @p y) and is contained in a bounding box of
 * @p w by @p h pixels. @p head is the length of the arrow head in pixels.
 * @p tail is the size of the the arrow relative to the overall witdh. @p tail
 * is a real comprised between 0 and 1.
 *
 * @image html arrow.png
 *
 * @note why head is in pixel and tail is a ratio ? Because when you want to resize the arrow width you generaly want to increase the tail size, but not the head's one, and when you resize the height, you want to keep the aspect ratio.
 */
arrow (x:real, y:real, w:real, h:real, head:real, tail:real);

/**
 * Creates a double arrow.
 *
 *  Draw a symetric double arraw. Similar to arrow (x:real, y:real, w:real, h:real, head:real, tail:real)
 * but with two heads.
 * @see See arrow for more details.
 *
 * @image html double_arrow.png
 *
 */
double_arrow (x:real, y:real, w:real, h:real, head:real, body:real);

/**
 * Draws a star.
 *
 * This primitive defines a regular @p p-branch star centered at (@p x,@p y).
 * Width is @p w, height is
 * @p h. @p r is a real between 0 and 1 that defines the aspects of the
 * branches. When @p r approaches 0, the branches get thinner. When @p r
 * gets closer to 1, the star degenerates into a regular polygon.
 *
 * @image html star.png
 *
 * @param p [integer] Number of branch
 * @param r [real] inner radius ratio
 */
star (x:real, y:real, w:real, h:real, p:integer, r:real);

/**
 * Draws a star.
 *
 * This primitive draws a regular star polygon centered at (@p x,@p y).
 * The star is obtained by placing @p p vertices regularly spaced on a
 * circle, to form a regular polygon. Then every @p q th vertices are
 * connected together.
 * @image html star_polygon.png
 *
 * @param p [integer] Number of branch
 * @param q [integer] Number of vertex to skip for sighting
 */
star_polygon (x:real, y:real, w:real, h:real, p:integer, q:integer);

/**

 * Draws a speech balloon.
 *
 * A speech ballon is made of:
 *   - a rounded rectangle centered at (@p x, @p y), with width @p w,
 *     height @p h and radius @p r, and
 *   - a tail ending at point (@p ax, @p ay). The tail width is determined
 *     automatically. The tail is not made of straight lines; it has a
 *     rounded shape. It is directed from the end point to the center of
 *     the speech balloon.
 * @image html speech_balloon.png
 *
 */
speech_balloon (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real);

/**
 * Draws a callout.
 *
 * A callout is made of:
 *   - a rounded rectangle centered at (@p x, @p y), with width @a w,
 *     height @p h and radius @p r, and
 *   - a triangular tail, which connects point (@p ax, @p ay) to the
 *     closest point on the rounded rectangle. The base of the tail has
 *     a width of @p tw pixels.
 *
 * @image html callout.png
 */
callout (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real, tw:real);


/** @} */

/**
 * @defgroup graph_3D 3D shapes
 * @ingroup Graphics
 * Creating 3D shapes.
 *
 * In all the 3D primitives,
 * -  @p x, @p y, and @p z represent the center of the drawing
 *   - [0, x) x > 0 goes toward the right,
 *   - [0, y) y > 0 goes toward the top,
 *   - [0, z) z > 0 goes toward the user.
 * -  @p h, @p w, and @p d represent the width, the height and the depth of the shape respectively
 * - shapes' size including those that got round part, is given by width, heigt and depth of the bouding box. 
 *
 * Examples: <a href="examples/3D_samples.ddd">3D_samples.ddd</a>
 *
 * Images keys:
 * - dark orange is the result of the documented primitive.
 * - black parts are the representations of the provided parameters.
 * - yellow parts are useful marks.
 *
 * Other 3D samples are available in @ref Lighting module.
 * @{
 */

/**
 * Draws a sphere.
 *
 * The sphere is divided in @p slices and @p stacks. The higher the value of
 * these parametres are, the smoother the sphere is (and longer the drawing is).
 * The sphere's @p w, @p h and @p d are not aimed to be equals. One can draw a water-melon with the sphere primitive
 * @image html sphere.png

 */
sphere (x:real, y:real, z:real, w:real, h:real, d:real, slices:integer, stacks:integer);

/**
 * Draws a cone.
 *
 * It draws a cone with a fixed number of polygon.
 * @image html cone.png
 */
cone (x:real, y:real, z:real, w:real, h:real, d:real);

/**
 * Draws a truncated cone.
 *
 * The diameter of the top of the cone is a ratio @p r of its basis. @p r is a real between 0.0 and 1.0. If @p r is 0 the drawing is a cone, if @p r is 1 the drawing is a cylinder.
 * @image html truncated_cone.png
 */
truncated_cone (x:real, y:real, z:real, w:real, h:real, d:real, r:real);

/**
 * Draws a cube.
 *
 * It draws a right parallelepiped, and in a particular case a cube.
 *
 * @image html right_parallelepiped.png
 *
 */
cube (x:real, y:real, z:real, w:real, h:real, d:real);

/**
 * Draws a cylinder.
 *
 * It draws the "skin" of a cylinder
 * @image html cylinder.png 
 */
cylinder (x:real, y:real, z:real, w:real, h:real, d:real);


/** @} */
