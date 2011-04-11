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
 * Opening environment to handle graphical objects and their attributes.
 * This groups also holds attribute information like color, line width, texture, etc...
 *
 * All color component are express as a percentage, so values are between 0.0 and 1.0 inclusive.
 * Texture and color are combined (they are not exclusive).
 *
 * @par Multitexturing
 *
 * Multitexturing is the use of more than one texture at a time on a same primitive.
 * For instance, the following code (<a href="examples/multitexturing.ddd">multitexturing.ddd</a>) draws a rectangle using the combinaison of a
 * texture of a wall and a second one of a spot to create the effect of the following figure.
 *
 * @image html multitexturing.png
 * @include multitexturing.ddd
 *
 * @{
 */
/**
 * Creates a selectable shape
 *
 *  Evaluate the child and mark the current shape.
 *  Modifications done to the environment is only applicable inside this shape as @ref locally does.
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
 *  It enables click action (@ref on), but does not enables handles for mouse handling.
 */
active_widget (t:tree);

/**
 * Anchors a set of shapes to the current position
 * @todo An example is missing for anchor.
 */
anchor (t:tree);

/**
 * Selects the line width for shape outlines.
 *
 * @c line_width @c "default" reset the line width to its default value.
 * @p lw is the width in pixels
 */
line_width (lw:real);

/**
 * specify the line stipple pattern.
 *
 * @param pattern
 * Specifies a 16-bit integer whose bit pattern determines
 * which fragments of a line will be drawn when the line is rasterized.
 * Bit zero is used first; the default pattern is all 1's.
 *
 * @param factor
 * Specifies a multiplier for each bit in the line stipple pattern.
 * If @p factor is 3, for example, each bit in the pattern is used three times
 * before the next bit in the pattern is used.
 * factor is clamped to the range [1, 256] and defaults to 1.
 *
 * @see OpenGL <a href="http://www.opengl.org/sdk/docs/man/xhtml/glLineStipple.xml">documentation</a>
 */
line_stipple(pattern:integer, factor:integer);

/**
 * Selects the color.
 *
 * The color name can be one of the following :
 *   - a color name from Qt::GlobalColor,
 *   - a color name from the W3C http://www.w3.org/TR/SVG/types.html#ColorKeywords
 *   - a color description in the "#RRGGBB" system
 *   - a color defined by Tao presentations
 *       - color "transparent"
 *       - color "default"
 *       - color "fill"
 *       - color "font"
 *       - color "line"
 *   - a color defined by a module (like @ref tao_visuals.doxy.h::color "Tao Visuals")
 *
 * @param colorname is the name of the color
 * @param alpha is the transparency factor. 0 is transparent and 1 is opaque. Its default value is 1.0.
 */
color (colorname:text, alpha:real);

/**
 * Selects the color.
 * @see @ref graph.doxy.h::color(colorname:text,alpha:real) "color colorname:text, alpha:real"
 */
color (colorname:text);

/**
 * Selects a color by its red, green, blue and alpha-channel value.
 *
 * @param r red component of the color
 * @param g green component of the color
 * @param b blue component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
color (r:real, g:real, b:real, a:real);

/**
 * Selects the color.
 * @see @ref graph.doxy.h::color(r:real,g:real,b:real,a:real) "color r:real, g:real, b:real, a:real"
 */
color (r:real, g:real, b:real);

/**
 * Selects the color by its hue, saturation, lightness, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color
 * @param s saturation component of the color
 * @param l lightness component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
color_hsl (h:real, s:real, l:real, a:real);

/**
 * Selects the color.
 * @see @ref graph.doxy.h::color_hsl(h:real, s:real, l:real, a:real) "color_hsl h:real, s:real, l:real, a:real"
 */
color_hsl (h:real, s:real, l:real);

/**
 * Selects the color by its hue, saturation, value, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color
 * @param s saturation component of the color
 * @param v value component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
color_hsv (h:real, s:real, v:real, a:real);

/**
 * Selects the color.
 * @see @ref graph.doxy.h::color_hsv(h:real, s:real, v:real, a:real) "color_hsv h:real, s:real, v:real, a:real"
 */
color_hsv (h:real, s:real, v:real);

/**
 * Selects the color by its cyan, yellow, magenta, black, and alpha-channel (transparency) components.
 *
 * @param c cyan component of the color
 * @param y yellow component of the color
 * @param m magenta component of the color
 * @param k black component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
color_cymk (c:real, y:real, m:real, k:real, a:real);

/**
 * Selects the color.
 * @see @ref graph.doxy.h::color_cymk(c:real, y:real, m:real, k:real, a:real) "color_cymk c:real, y:real, m:real, k:real, a:real"
 */
color_cymk (c:real, y:real, m:real, k:real);

/**
 * Reset color value.
 * shortcut for @c color 0,0,0,0 i.e. a transparent black.
 */
no_color ();

/**
 * Set the background color.
 * Set the RGB clear (background) color
 */
clear_color (r:real, g:real, b:real, a:real);

/**
 * Reset line_color value.
 * shortcut for @c line_color 0,0,0,0 i.e. a transparent black.
 */
no_line_color ();

/**
 * Selects the line color.
 *
 * The color name can be one of the following :
 *   - a color name from Qt::GlobalColor,
 *   - a color name from the W3C http://www.w3.org/TR/SVG/types.html#ColorKeywords
 *   - a color description in the "#RRGGBB" system
 *   - a color defined by Tao presentations
 *       - line_color "transparent"
 *       - line_color "default"
 *       - line_color "fill"
 *       - line_color "font"
 *       - line_color "line"
 *   - a color defined by a module (like @ref tao_visuals.doxy.h::color "Tao Visuals")
 *
 * @note @c color @c "font" and @c line_color @c "font" are not the same color,
 * but relevent colors for the line of a glyph and the fill of a glyph.
 *
 * @param colorname is the name of the color
 * @param alpha is the transparency factor. 0 is transparent and 1 is opaque. Its default value is 1.0.
 */
line_color (colorname:text, alpha:real);

/**
 * Selects the color.
 * @see @ref graph.doxy.h::color(colorname:text,alpha:real) "color colorname:text, alpha:real"
 */
line_color (colorname:text);

/**
 * Selects a line color by its red, green, blue and alpha-channel value.
 *
 * @param r red component of the color
 * @param g green component of the color
 * @param b blue component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
line_color (r:real, g:real, b:real, a:real);

/**
 * Selects the line_color.
 * @see @ref graph.doxy.h::line_color(r:real,g:real,b:real,a:real) "line_color r:real, g:real, b:real, a:real"
 */
line_color (r:real, g:real, b:real);

/**
 * Selects the line_color by its hue, saturation, lightness, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color
 * @param s saturation component of the color
 * @param l lightness component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
line_color_hsl (h:real, s:real, l:real, a:real);

/**
 * Selects the line_color.
 * @see @ref graph.doxy.h::line_color_hsl(h:real, s:real, l:real, a:real) "line_color_hsl h:real, s:real, l:real, a:real"
 */
line_color_hsl (h:real, s:real, l:real);

/**
 * Selects the line_color by its hue, saturation, value, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color
 * @param s saturation component of the color
 * @param v value component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
line_color_hsv (h:real, s:real, v:real, a:real);

/**
 * Selects the line_color.
 * @see @ref graph.doxy.h::line_color_hsv(h:real, s:real, v:real, a:real) "line_color_hsv h:real, s:real, v:real, a:real"
 */
line_color_hsv (h:real, s:real, v:real);

/**
 * Selects the line_color by its cyan, yellow, magenta, black, and alpha-channel (transparency) components.
 *
 * @param c cyan component of the color
 * @param y yellow component of the color
 * @param m magenta component of the color
 * @param k black component of the color
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * Its default value is 1.0.
 */
line_color_cymk (c:real, y:real, m:real, k:real, a:real);

/**
 * Selects the line_color.
 * @see @ref graph.doxy.h::line_color_cymk(c:real, y:real, m:real, k:real, a:real) "line_color_cymk c:real, y:real, m:real, k:real, a:real"
 */
line_color_cymk (c:real, y:real, m:real, k:real);

/**
 * Selects the texture.
 * Build a GL texture out of an image file @p filename.
 * Supported format include:
 *   - BMP  Windows Bitmap
 *   - GIF  Graphic Interchange Format (optional)
 *   - JPG  Joint Photographic Experts Group
 *   - JPEG Joint Photographic Experts Group
 *   - PNG  Portable Network Graphics
 *   - PBM  Portable Bitmap
 *   - PGM  Portable Graymap
 *   - PPM  Portable Pixmap
 *   - TIFF Tagged Image File Format
 *   - XBM  X11 Bitmap
 *   - XPM  X11 Pixmap
 *
 */
texture(filename:text);

/**
 * Selects the texture by its id.
 * Build a GL texture out of an @p id.
 * @note @c texture @c 0 deactivate the texture binded on the current unit.
 */
texture(id:integer);

/**
 * Selects the texture unit.
 * Set the current texture unit to @p unit.
 *
 * Default value is 0.
 *
 * @note Only the fourth texture units can be used without shaders. The maximum number of texture units and coordinates depends on the graphic card.
 * @note @c shader_set can be used to link the texture unit with a sampler inside shaders.
 * @see OpenGL documentation about GL_MAX_TEXTURE_COORDS, GL_MAX_TEXTURE_IMAGE_UNITS and GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
 */
texture_unit (unit:integer);

/**
 * Create a GL animated texture.
 * Build a GL texture images from an animated image file. Supported format are:
 *   - GIF	Animated Graphic Interchange Format
 *   - MNG	Animated portable Network Graphics
 *   - others [TODO:  description is troncated]
 */
animated_texture(filename:text);
/**
 * Creates a texture from an SVG file.
 * Build a GL texture out of an SVG file @p svg.
 */
svg(svg:text);

/**
 * Geometric transformation for texture
 * Allows transformation to be applied to the current texture.
 * Transformation are specified in the body and can be a combination of
 *  - rotate
 *  - scale
 *  - translate
 *
 * @see transforms
 */
texture_transform (body:tree);

/**
 * Texture wrapping
 * Controls the wrapping of the texture along @c S and @c T axis.
 * @param s is the pre-transformation horizontal axis (like X).
 * @param t is the pre-transformation vertical axis (like Y).
 *
 * Default values are false, false.
 * @see OpenGL documentation about GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T
 */
texture_wrap (s:boolean, t:boolean);

/**
 * Resets the texture.
 * Clears the texture.
 */
no_texture();

/** @} */

/**
 * @defgroup graph_img Image handling
 * @ingroup Graphics
 * Handles images.
 * Images are loaded from the local file system. The supported format are those
 * supported by Qt, including :
 *   - BMP  Windows Bitmap
 *   - GIF  Graphic Interchange Format (optional)
 *   - JPG  Joint Photographic Experts Group
 *   - JPEG Joint Photographic Experts Group
 *   - PNG  Portable Network Graphics
 *   - PBM  Portable Bitmap
 *   - PGM  Portable Graymap
 *   - PPM  Portable Pixmap
 *   - TIFF Tagged Image File Format
 *   - XBM  X11 Bitmap
 *   - XPM  X11 Pixmap
 *
 * Images are combined with the current color.
 *
 * Texture are handled in the @ref graph_env module.
 * @{
 */

/**
 * Display an image.
 * Load an image from file @p filename.
 *  - @p x and @p y coordinate of the image center are specified in pixel
 *  - @p w and @p h are size express in percentage
 * @image html "images/image.png"
@code
page "image",
    color "white"
    image 0, 0, 100%, 100%, "../images/tortue.jpg"
    axes
@endcode
 */
image (x:real, y:real, w:real, h:real, filename:text);

/**
 * Display an image.
 * Load an image from file @p filename.
 *  - @p x and @p y coordinate of the image center are specified in pixel
 *  - @p w and @p h are size express in pixel
 * @image html "images/image_px.png"

@code
page "image_px",
    color "white"
    image_px 0, 0, 200, 200, "../images/tortue.jpg"
    axes
    line_color "black"
    width_and_height 0, 0, 200, 200
@endcode

 */
image_px (x:real, y:real, w:real, h:real, filename:text);

/**
 * Get image size.
 * Return the image size of the @p img image.
 * @return [infix] a tree containing <tt>w, h</tt>
 */
image_size(img:text);

/** @} */

/**
 * @defgroup graph_path Graphical Path
 * @ingroup Graphics
 * Creation of drawing in 2D or 3D vertex by vertex.
 *
 * A path is a drawing made of straight lines and curves. To build a path you specify where you
 * place the pencil (move), where you want to line (line) to, and what kind of curve (quad, cubic)
 * you want to draw. Coordinates can be given obsolutely or relatively to the previous location.
 *
 * When placed in a @ref shape environment, each vertex and control point can be handled by the mouse.
 *
 * Example of path primitives used to produce here after images :
 * <a href="examples/path_samples.ddd">path_samples.ddd</a>
 *
 * @{
 */

/**
 * Opens a graphic path environment.
 *
 * A graphic path is a collection of segments of lines or curves
 * (quadrics or cubics).
 */
path (t:tree);

/**
 * Moves path cursor to a specific point.
 *
 * Add a 'moveTo' to the current path.
 *
 * Shortcut for 2D is also available :
 * @code move_to x:real, y:real @endcode
 */
move_to (x:real, y:real, z:real);

/**
 * Adds a line segment to the current path.
 *
 * Shortcut for 2D is also available :
 * @code line_to x:real, y:real @endcode
 */
line_to (x:real, y:real, z:real);

/**
 * Adds a quadric segment to the current path.
 *
 * A quadric segment of path is a curve where you specify the two
 * endpoints and one control point that controls the curve itself.
 * (@p cx, @p cy, @p cz) defines the control point.
 * (@p x, @p y, @p z) is the end point.
 *
 * Shortcut for 2D is also available :
 * @code quad_to cx:real, cy:real, x:real, y:real @endcode
 * @image html quad_path.png
 */
quad_to (cx:real, cy:real, cz:real, x:real, y:real, z:real);

/**
 * Adds a cubic segment to the current path.
 *
 * A cubic segment of path is a curve where you specify the two endpoints
 * and two control points that control two curves (like S).
 * (@p c1x, @p c1y, @p c1z) defines the first control point.
 * (@p c2x, @p c2y, @p c2z) defines the second control point.
 * (@p x, @p y, @p z) is the end point.
 *
 * Shortcut for 2D is also available :
 * @code cubic_to c1x:real, c1y:real, c2x:real, c2y:real, x:real, y:real @endcode
 * @image html cubic_path.png
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
 *
 * It creates a loop in the path by drawing a straight line from
 * current point to last move_to point or the start point.
 * @note It does not close the path environment. One can continue to add segment to current path.
 */
close_path ();

/**
 * Sets the style of the path endpoints.
 *
 * @p s is the start point and @p e is the end point.
 *
 * Only very beginning and very end of the path can be styled. If a
 * text is added at the end of the path, the end style won't be applied
 * to the line end (because end of the line is not the end of the path).
 *
 * Supported symbols are:
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
 *
 * @image html endpoints_style.png
 *
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
 *  The rectangle is centered in (@p x, @p y), with width @p w and height @p h.
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
 *  Draw a right triangle with hypotenuse centered in (@p x, @p y), with width @p w and height @p h.
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
 * @note why head is in pixel and tail is a ratio ? Because when you want to
 * resize the arrow width you generaly want to increase the tail size, but not
 * the head's one, and when you resize the height, you want to keep the aspect ratio.
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
 * @param q [integer] Number of vertex to skip for sighting. q is in range [1..p/2].
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

/**
 * Draws a regular polygon
 *
 * This primitives draws a regular polygon with @p p vertex.
 * @image html polygon.png
 * Shortcuts has been defined for polygon from 3 to 20 vertexes.
 *  -# none
 *  -# none
 *  -# equilateral_triangle
 *  -# tetragon
 *  -# pentagon
 *  -# hexagon
 *  -# heptagon
 *  -# octagon
 *  -# nonagon
 *  -# decagon
 *  -# hendecagon
 *  -# dodecagon
 *  -# tridecagon
 *  -# tetradecagon
 *  -# pentadecagon
 *  -# hexadecagon
 *  -# heptadecagon
 *  -# octadecagon
 *  -# enneadecagon
 *  -# icosagon
 *
 *
 */
polygon (x:real, y:real, w:real, h:real, p:integer);

/**
 * @copydoc star_polygon
 * @image html polygram.png
 * Shortcuts has been defined for polygram from 5 to 10 vertexes.

 *  - pentagram       p = 5, q = 2
 *  - hexagram        p = 6, q = 2
 *  - star_of_david   p = 6, q = 2
 *  - heptagram       p = 7, q = 2
 *  - star_of_lakshmi p = 8, q = 2
 *  - octagram        p = 8, q = 3
 *  - nonagram        p = 9, q = 3
 *  - decagram        p =10, q = 3

 */
polygram (x:real, y:real, w:real, h:real, p:integer, q:integer);

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
 * Other 3D samples are available in @ref Lighting module.
 * @{
 */

/**
 * Draws a sphere.
 *
 * The sphere is divided in @p slices and @p stacks. The higher the value of
 * these parametres are, the smoother the sphere is (and longer the drawing is).
 * The sphere's @p w, @p h and @p d are not aimed to be equals. One can draw a
 * water-melon with the sphere primitive
 * @image html sphere.png

 */
sphere (x:real, y:real, z:real, w:real, h:real, d:real, slices:integer, stacks:integer);

/**
 * Draws a sphere.
 *
 * Sphere with diameter @p d located at (@p x, @p y, @p z).
 * Shorcut to
@code
sphere x, y, z, d, d, d, 25, 25
@endcode
 */
sphere (x:real, y:real, z:real, r:real);

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
 * The diameter of the top of the cone is a ratio @p r of its basis. @p r is a real between 0.0 and 1.0.
 * If @p r is 0 the drawing is a cone, if @p r is 1 the drawing is a cylinder.
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
