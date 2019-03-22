// *****************************************************************************
// frame.doxy.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2012,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2014, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************
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
 *
 * @~
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
 * Sets the dimensions of the display window in pixels.
 * @~french
 * Définit les dimensions de la fenêtre d'affichage en pixels.
 */
window_size(w:integer, h:integer);

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
 * @ref frame_texture, puis la texture est affichée sur un rectangle
 * centré en (@p x, @p y).
 */
frame( x:real, y:real, w:real, h:real, body:tree);

/**
 * @~english
 * Create a dynamic texture.
 * Make a dynamic texture described by the supplied content. The
 * texture is cleared each time.
 *
 * @return Return the texture id.
 *
 * @~french
 * Crée une texture dynamique.
 * Crée une texture dynamique décrite par le code fourni. La texture
 * est effacée à  chaque fois.
 *
 * @return Retourne l'identifiant de la texture.
 */
integer frame_texture( w:real, h:real, body:tree);


/**
 * @~english
 * Create a dynamic texture with a specific name.
 * Make a dynamic texture with the size (@p w, @p h) and named @p name.
 * The code @p body is then executed and drawn in this one.
 * This function assures that the texture ID is the same for a given @p name,
 * no matter the objects drawn before.
 *
 * @return Return the texture ID.
 *
 * @~french
 * Crée une texture dynamique avec un nom particulier.
 * Crée une texture dynamique de la taille spécifié et appelée @p name.
 * Le code @p body est alors executé et affiché dans cette texture.
 * Cette fonction garantit que l'identifiant de la texture retournée est le même
 * pour une valeur de @p name donnée, quelque soit les objets tracés avant.
 *
 * @return Retourne l'identifiant de la texture.
 */
integer frame_texture( name:text, w:real, h:real, body:tree);


/**
 * @~english
 * Create a persistent dynamic texture.
 * Make a dynamic texture that persists over time, and can therefore
 * be used to create the texture incrementally. The texture is
 * cleared if the canvas is resized.
 *
 * @return Return the texture id.
 *
 * @~french
 * Crée une texture dynamique.
 * Crée une texture dynamique décrite par le code fourni, persistante
 * au cours du temps. La texture est effacée lorsque le canevas est
 * redimensionné.
 *
 * @return Retourne l'identifiant de la texture.
 */
integer canvas( w:real, h:real, body:tree);


/**
 * @~english
 * Count non-transparent pixels in the current canvas or texture.
 * Return the number of non-transparent pixels in the current canvas
 * or dynamic texture. The minimum alpha indicates the alpha value above which
 * a pixel will be considered non-transparent, between 0.0 and 1.0.
 * This function can only be used within a @ref canvas or @ref frame_texture.
 *
 * @~french
 * Compte le nombre de pixels non transparents dans une texture dynamique
 * Retuourne le nombre de pixels non-transparents dans un canevas ou
 * texture dynamique. La valeur de canal alpha donnée en argument
 * spécifie le seuil d'opacité minimum, entre 0.0 et 1.0.
 */
integer frame_pixel_count(alphaMin:real);


/**
 * @~english
 * Create a texture.
 * Make a color texture and a depth texture out of the described content.
 *
 * Create two textures by executing the specified code (@p body). The first
 * one is the color information, the second one is a depth map. The color
 * texture is active after execution, its ID is also returned by the primitive.
 * The output parameter @p d receives the identifier of the depth texture.
 * @p w and @p h are the dimensions of both textures.
 * Here is an example:
 *
 * @~french
 * Crée deux textures (couleur et profondeur) à partir de code.
 *
 * Exécute le code de @p body, et crée deux textures : la première contient
 * l'information de couleur, la seconde est une carte de profondeur
 * (<em>depth map</em>). La texture de couleur est active après l'exécution de
 * la primitive, son identifiant est également renvoyé. Le paramètre @p d
 * reçoit le numéro de la texture de profondeur.
 * @p w et @p h sont les dimensions de chacune des textures.
 * Exemple :
 *
 * @~
@code
ww -> window_width
wh -> window_height
DepthTex -> 0
frame_texture_with_depth ww, wh, DepthTex,
    light 0
    color "pink"
    rotatey 10 * page_time
    T -> ww * 40%
    sphere -ww/4, 0, ww/4, T
    locally
        rotatex -10
        rotatey 20
        cube ww/4, 0, -ww/4, T, T, T
color "white"
rectangle -ww/4, 0, ww/2, wh/2
texture DepthTex
rectangle ww/4, 0, ww/2, wh/2
@endcode
 *
 * @image html "frame_texture_with_depth.png"
 */
integer frame_texture_with_depth( w:real, h:real, d:integer, body:tree);


/**
 * @~english
 * Frame height.
 * Returns the height of the current layout frame.
 *
 * @~french
 * Hauteur du cadre.
 * Renvoie la hauteur du cadre courant.
 */
real frame_height ();

/**
 * @~english
 * Frame width.
 * Returns the width of the current layout frame.
 *
 * @~french
 * Largeur du cadre.
 * Renvoie la largeur du cadre courant.
 */
real frame_width ();

/** @} */
