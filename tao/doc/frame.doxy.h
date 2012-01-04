/**
 * @~english
 * @addtogroup WindowsAndFrame Window and frames
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
 * @bug #860 Computation of frame bounding box (requesting by @ref frame_width,
 * @ref frame_height, @ref frame_depth) is damaging the frame itself. 
 *
 * @~french
 * @addtogroup WindowsAndFrame Fenêtre et cadres
 * @ingroup TaoBuiltins
 * Dimension de la fenêtre et cadres.
 * La taille de la fenêtre principale définit l'espace disponible pour tracer.
 * Une texture de cadre (@ref frame_texture) est comme une feuille de papier
 * sur laquelle on peut dessiner, un cadre permet de placer cette feuille à un
 * endroit précis.
 *
 * @image html "images/frame.png"
 * Code source de cet exemple :
 * <a href="examples/frame.ddd">frame.ddd</a>
 * @{
 */
/**
 * @~english
 * Returns the height of the document window in pixels.
 * @~french
 * La hauteur de la zone d'affichage du document en pixels.
 */
integer window_height ();

/**
 * @~english
 * Returns the width of the document window in pixels.
 * @~french
 * La largeur de la zone d'affichage du document en pixels.
 */
integer window_width ();

/**
 * @~english
 * Create a frame.
 * Draw a frame with the described content in @p body. This frame as no displayed border.
 * It makes a texture of size (@p w, @p h) using @ref frame_texture, and map it on a rectangle
 * at (@p x, @p y) location.
 *
 * @~french
 * Crée un cadre.
 * Crée un cadre qui contient les éléments graphique de @p body. Ce cadre n'a
 * pas de bordure. Une texture de taille (@p w, @p h) est créée par
 * @ref frame_texture, puis la texture est affichée sur un rectangle à la
 * centré en (@p x, @p y).
 */
frame( x:real, y:real, w:real, h:real, body:tree);

/**
 * @~english
 * Create a texture.
 * Make a texture out of the described content
 *
 * @~french
 * Crée une texture.
 * Crée une texture à partir du code fourni
 */
frame_texture( w:real, h:real, body:tree);


/**
 * @~english
 * Frame height.
 * Returns the height of the current layout frame.
 *
 * @~french
 * Hauteur du cadre.
 * Renvoie la hauteur du cadre courant.
 */
frame_height ();

/**
 * @~english
 * Frame width.
 * Returns the width of the current layout frame.
 *
 * @~french
 * Largeur du cadre.
 * Renvoie la largeur du cadre courant.
 */
frame_width ();

/** @} */
