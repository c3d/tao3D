/**
 * @~english
 * @defgroup Graphics Graphics
 * @ingroup TaoBuiltins
 *
 * Tao Presentations relies on OpenGL (http://www.opengl.org/) to provide
 * a true 3D drawing environment. Altough some GL functions are directly
 * available through Tao primitives, Tao Presentations also provides more
 * ready-to-use graphics functions to draw usual geometric objects,
 * whether 2D or 3D.
 *
 * @~french
 * @defgroup Graphics Graphisme
 * @ingroup TaoBuiltins
 *
 * Tao Presentations utilise OpenGL (http://www.opengl.org/) pour proposer
 * un environnement graphique 3D. Bien que certaines fonctions GL soient
 * disponibles par l'intermédiaire de primitives, Tao Presentations fournit
 * aussi des fonctions graphiques pour dessiner les formes géométriques
 * usuelles, en deux ou trois dimensions.
 */


/**
 * @~english
 * @defgroup graph_env Graphical environment (line and fill colors, textures)
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
 * For instance, the following code (<a href="examples/multitexturing.ddd">multitexturing.ddd</a>) draws a rectangle using the combinaison of
 * wall and spot textures in order to create a nice lighting effect.
 *
 * @include multitexturing.ddd
 * @image html multitexturing.png "Multitexturing demo: multitexturing.ddd"
 *
 * @~french
 * @defgroup graph_env Environnement graphique (couleur des lignes/de remplissage, textures)
 * @ingroup Graphics
 * Définition des attributs des objets graphiques.
 * Ce groupe définit des attributs comme la couleur, la largeur des lignes, les
 * textures, etc.
 *
 * Les composantes de couleurs sont exprimées en pourcentage, soit des valeurs
 * entre 0.0 et 1.0. La texture et la couleur peuvent être combinées.
 *
 * @par Multi-texture
 *
 * Le multi-texture est l'utilisation similtanée de plusieurs textures sur une
 * même forme géométrique.
 * Par exemple, le code qui suit (<a href="examples/multitexturing.ddd">multitexturing.ddd</a>)
 * affiche un rectangle et utilise une combinaison de deux textures, l'une
 * représentant un mur et l'autre un faisceau de lumière, afin de créer un
 * effet d'éclairage.
 *
 * @include multitexturing.ddd
 * @image html multitexturing.png "Démonstration : multitexturing.ddd"
 *
 * @{
 */

/**
 * @~english
 * Creates a local context.
 *
 * Evaluate the child tree while preserving the current graphical state.
 * Any state modification like color or translation done in the
 * locally block only affect the block.
 *
 * @~french
 * Crée un contexte local.
 *
 * Évalue l'arbre @p t en préservant l'état graphique courant. Par conséquent,
 * toute modification comme un changement de couleur ou une modification
 * géométrique (rotation, translation, changement de facteur d'échelle)
 * n'affecte que la suite du bloc locally.
 */
locally (t:tree);

/**
 * @~english
 * Creates a selectable shape.
 *
 *  Evaluate the child and mark the current shape.
 *  Modifications done to the environment is only applicable inside this shape as @ref locally does.
 *
 *  Make the shape sensible to mouse events.
 *  It enables click action (@ref on), and handles for selection, motion, rotation, resize...
 *
 * @~french
 * Crée une forme sélectionnable.
 *
 * Évalue l'arbre @p t en préservant l'état graphique courant, comme
 * @ref locally, mais rend en outre la forme sensible aux événements de la
 * la souris ce qui permet d'utiliser la primitive @ref on, et de manipuler
 * l'objet (déplacement, rotation, redimensionnement).
 */
shape (t:tree);


/**
 * @~english
 * Makes the widget clickable.
 *
 *  Create a context for active widgets (like buttons) or drawing (circle, etc...).
 *  Make the shape sensive to mouse events.
 *  It enables click action (@ref on), but does not enables handles for mouse handling.
 *
 * @~french
 * Rend une forme géométrique cliquable.
 *
 * Crée un contexte pour les widgets actifs (par exemple, les boutons) ou plus
 * généralement les formes géométriques (cercle, etc.) ce qui permet d'utiliser
 * @ref on pour déclencher une action. Contrairement à @ref shape,
 * active_widget ne permet pas de déplacer ou modifier la forme.
 */
active_widget (t:tree);

/**
 * @~english
 * Anchors a set of shapes to the current position
 * @todo An example is missing for anchor.
 *
 * @~french
 * Arrime un ensemble de formes à la position courante.
 */
anchor (t:tree);

/**
 * @~english
 * Selects the line width for shape outlines.
 *
 * @c line_width @c "default" reset the line width to the default value.
 * @p lw is the width in pixels.
 *
 * @~french
 * Définit la largeur du trait pour le tracé des formes.
 *
 * @c line_width @c "default" reprend la valeur par défaut.
 * @p lw est la largeur en pixels.
 */
line_width (lw:real);

/**
 * @~english
 * Specify the line stipple pattern.
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
 *
 * @~french
 * Définit le type de pointillé pour le tracé des formes.
 *
 * @param pattern
 * Un entier 16 bits dont les bits (0 ou 1) définit quelles portions de la
 * ligne est tracée. Le bit zéro est utilisé en premier. Pour tracer une ligne
 * continue, tous les bits doivent être à 1 (pattern = 65535).
 *
 * @param factor
 * Un multiplicateur pour chaque bit dans @p pattern.
 * Par exemple, si @p factor vaut 3, chaque bit de @p pattern est utilisé 3
 * fois avant que le bit suivant ne soit utilisé.
 * factor est limité à l'intervalle [1, 256].
 *
 * @see Fonction OpenGL <a href="http://www.opengl.org/sdk/docs/man/xhtml/glLineStipple.xml">glLineStipple</a>
 */
line_stipple(pattern:integer, factor:integer);

/**
 * @~english
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
 * @param alpha is the transparency factor. 0 is transparent and 1 is opaque. The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de remplissage courante.
 *
 * Le nom de la couleur peut être :
 *   - un nom de couleur de Qt::GlobalColor,
 *   - un nom de couleur définit par le W3C : http://www.w3.org/TR/SVG/types.html#ColorKeywords
 *   - une description rouge, vert, bleu sous la forme "#RRGGBB"
 *   - une couleur définie par Tao presentations :
 *       - color "transparent"
 *       - color "default"
 *       - color "fill"
 *       - color "font"
 *       - color "line"
 * Les modules peuvent également définir des couleurs.
 *
 * @param colorname Le nom de la couleur.
 * @param alpha Le facteur de transparence. 0.0 est transparent et 1.0 est
 * opaque. La valeur par défaut est 1.0.
 */
color (colorname:text, alpha:real);

/**
 * @~english
 * Selects the color.
 * @~french
 * Choisit la couleur de remplissage.
 * @~
 * @see @ref graph.doxy.h::color(colorname:text,alpha:real) "color colorname:text, alpha:real"
 */
color (colorname:text);

/**
 * @~english
 * Selects a color by its red, green, blue and alpha-channel value.
 *
 * @param r red component of the color, in the range 0.0 to 1.0.
 * @param g green component of the color, in the range 0.0 to 1.0.
 * @param b blue component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de remplissage (rouge, vert, bleu et alpha).
 *
 * @param r Composante rouge, entre 0.0 to 1.0.
 * @param g Composante verte, entre 0.0 to 1.0.
 * @param b Composante bleue, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
color (r:real, g:real, b:real, a:real);

/**
 * @~english
 * Selects the color.
 * @~french
 * Choisit la couleur de remplissage.
 * @~
 * @see @ref graph.doxy.h::color(r:real,g:real,b:real,a:real) "color r:real, g:real, b:real, a:real"
 */
color (r:real, g:real, b:real);

/**
 * @~english
 * Selects the color by its hue, saturation, lightness, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color,  in degrees (in the range 0.0 to 360.0).
 * @param s saturation component of the color, in the range 0.0 to 1.0.
 * @param l lightness component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de remplissage (teinte, saturation, luminosité et alpha).
 *
 * @param h Teinte, en degrés (entre 0.0 et 360.0).
 * @param s Saturation, entre 0.0 to 1.0.
 * @param l Luminosité, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
color_hsl (h:real, s:real, l:real, a:real);

/**
 * @~english
 * Selects the color.
 * @~french
 * Choisit la couleur de remplissage.
 * @~
 * @see @ref graph.doxy.h::color_hsl(h:real, s:real, l:real, a:real) "color_hsl h:real, s:real, l:real, a:real"
 */
color_hsl (h:real, s:real, l:real);

/**
 * @~english
 * Selects the color by its hue, saturation, value, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color, in degrees (in the range 0.0 to 360.0).
 * @param s saturation component of the color, in the range 0.0 to 1.0.
 * @param v value component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de remplissage (teinte, saturation, valeur et alpha).
 *
 * @param h Teinte, en degrés (entre 0.0 et 360.0).
 * @param s Saturation, entre 0.0 to 1.0.
 * @param v Valeur, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
color_hsv (h:real, s:real, v:real, a:real);

/**
 * @~english
 * Selects the color.
 * @~french
 * Choisit la couleur de remplissage.
 * @~
 * @see @ref graph.doxy.h::color_hsv(h:real, s:real, v:real, a:real) "color_hsv h:real, s:real, v:real, a:real"
 */
color_hsv (h:real, s:real, v:real);

/**
 * @~english
 * Selects the color by its cyan, yellow, magenta, black, and alpha-channel (transparency) components.
 *
 * @param c cyan component of the color, in the range 0.0 to 1.0.
 * @param y yellow component of the color, in the range 0.0 to 1.0.
 * @param m magenta component of the color, in the range 0.0 to 1.0.
 * @param k black component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0 is transparent and 1 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de remplissage (cyan, magenta, jaune, noir).
 *
 * @param c Composante cyan, entre 0.0 to 1.0.
 * @param y Composante jaune, entre 0.0 to 1.0.
 * @param m Composante magenta, entre 0.0 to 1.0.
 * @param k Composante noire, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
color_cymk (c:real, y:real, m:real, k:real, a:real);

/**
 * @~english
 * Selects the color.
 * @~french
 * Choisit la couleur de remplissage.
 * @~
 * @see @ref graph.doxy.h::color_cymk(c:real, y:real, m:real, k:real, a:real) "color_cymk c:real, y:real, m:real, k:real, a:real"
 */
color_cymk (c:real, y:real, m:real, k:real);

/**
 * @~english
 * Selects transparent black.
 * Shortcut for @c color 0,0,0,0.
 * @~french
 * Choisit la couleur de remplissage : noir transparent.
 * Raccourci pour @c color 0,0,0,0.
 */
no_color ();

/**
 * @~english
 * Specifies the clear values for the color buffers.
 * When this primitive is called in the root layout (that is, outside of
 * any @ref locally block or similar), it sets the clear color for the
 * main drawing area.
 * This color is used to fill the drawing area (screen or window) before
 * anything is drawn. As such it can be considered the background color.
 *
 * When the primitive is called inside a @ref frame or @ref frame_texture
 * block, it sets the clear color of the local drawing buffer, which can
 * have subtle effects on the rendering. The default
 * clear color of a @ref frame_texture is transparent white,
 * RGBA (1, 1, 1, 0).
 * In some cases you may want to change this color. For instance, look
 * at the border of the characters in the following example, with and
 * without the @ref clear_color line in the @ref frame_texture block.
 * @code
clear_color 1, 1, 1, 1
color "black"
rectangle 0, 0, 600, 200
color "white"
frame_texture 600, 200,
    // Try commenting out the following line
    clear_color 0, 0, 0, 0
    text_box 0, 0, 600, 200,
        color "blue"
        font "Times", 96
        align_center
        vertical_align_center
        text "Hello"
    color 1, 0, 0, 75%
    circle 200, 0, 50
rectangle 0, 0, 600, 200
 * @endcode
 *
 * @~french
 * Définit la couleur d'effacement (clear color)
 * Cette primitive définit la couleur qui est utilisée pour remplir la zone
 * de dessin avant d'afficher quoique ce soit. Par conséquent, on peut
 * considérer qu'il s'agit de la couleur de fond.
 * Quand cette primitive est appelée dans le @a layout principal
 * (c'est-à-dire en dehors de tout bloc @ref locally ou similaire), elle
 * définit la couleur de fond pour la zone de dessin principale.
 *
 * Quand elle est appelée dans un bloc @ref frame ou
 * @ref frame_texture, elle définit la couleur d'effacement de la zone de
 * dessin locale (une texture), ce qui peut avoir des effets subtils sur
 * le rendu. La couleur d'effacement par défaut dans @ref frame ou
 * @ref frame_texture est le blanc transparent, RGBA (1, 1, 1, 0).
 * Dans certains cas il peut être utile de changer cette couleur. Par
 * exemple, observez le contour des caractères dans l'exemple suivant,
 * avec et sans la ligne @ref clear_color dans le bloc @ref frame_texture.
 * @code
clear_color 1, 1, 1, 1
color "black"
rectangle 0, 0, 600, 200
color "white"
frame_texture 600, 200,
    // Essayez de mettre la ligne suivante en commentaire
    clear_color 0, 0, 0, 0
    text_box 0, 0, 600, 200,
        color "blue"
        font "Times", 96
        align_center
        vertical_align_center
        text "Bonjour"
    color 1, 0, 0, 75%
    circle 200, 0, 50
rectangle 0, 0, 600, 200
 * @endcode
 */
clear_color (r:real, g:real, b:real, a:real);

/**
 * @~english
 * Selects transparent black for line color.
 * Shortcut for @c line_color 0,0,0,0.
 * @~french
 * Choisit la couleur de ligne : noir transparent.
 * Raccourci pour @c line_color 0,0,0,0.
 */
no_line_color ();

/**
 * @~english
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
 *
 * @note @c color @c "font" and @c line_color @c "font" are not the same color,
 * but relevent colors for the line of a glyph and the fill of a glyph.
 *
 * @param colorname is the name of the color
 * @param alpha is the transparency factor. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de ligne courante.
 *
 * Le nom de la couleur peut être :
 *   - un nom de couleur de Qt::GlobalColor,
 *   - un nom de couleur définit par le W3C : http://www.w3.org/TR/SVG/types.html#ColorKeywords
 *   - une description rouge, vert, bleu sous la forme "#RRGGBB"
 *   - une couleur définie par Tao presentations :
 *       - color "transparent"
 *       - color "default"
 *       - color "fill"
 *       - color "font"
 *       - color "line"
 * Les modules peuvent également définir des couleurs.
 *
 * @note @c color @c "font" et @c line_color @c "font" ne sont pas la même
 * couleur.
 *
 * @param colorname Le nom de la couleur.
 * @param alpha Le facteur de transparence. 0.0 est transparent et 1.0 est
 * opaque. La valeur par défaut est 1.0.
 */
line_color (colorname:text, alpha:real);

/**
 * @~english
 * Selects the line color.
 * @~french
 * Choisit la couleur de ligne.
 * @~
 * @see @ref graph.doxy.h::color(colorname:text,alpha:real) "color colorname:text, alpha:real"
 */
line_color (colorname:text);

/**
 * @~english
 * Selects a line color by its red, green, blue and alpha-channel value.
 *
 * @param r red component of the color, in the range 0.0 to 1.0.
 * @param g green component of the color, in the range 0.0 to 1.0.
 * @param b blue component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de ligne (rouge, vert, bleu et alpha).
 *
 * @param r Composante rouge, entre 0.0 to 1.0.
 * @param g Composante verte, entre 0.0 to 1.0.
 * @param b Composante bleue, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
line_color (r:real, g:real, b:real, a:real);

/**
 * @~english
 * Selects the line color.
 * @~french
 * Choisit la couleur de ligne.
 * @~
 * @see @ref graph.doxy.h::line_color(r:real,g:real,b:real,a:real) "line_color r:real, g:real, b:real, a:real"
 */
line_color (r:real, g:real, b:real);

/**
 * @~english
 * Selects the line_color by its hue, saturation, lightness, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color, in degrees (in the range 0.0 to 360.0).
 * @param s saturation component of the color, in the range 0.0 to 1.0.
 * @param l lightness component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de ligne (teinte, saturation, luminosité et alpha).
 *
 * @param h Teinte, en degrés (entre 0.0 et 360.0).
 * @param s Saturation, entre 0.0 to 1.0.
 * @param l Luminosité, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
line_color_hsl (h:real, s:real, l:real, a:real);

/**
 * @~english
 * Selects the line color.
 * @~french
 * Choisit la couleur de ligne.
 * @~
 * @see @ref graph.doxy.h::line_color_hsl(h:real, s:real, l:real, a:real) "line_color_hsl h:real, s:real, l:real, a:real"
 */
line_color_hsl (h:real, s:real, l:real);

/**
 * @~english
 * Selects the line_color by its hue, saturation, value, and alpha-channel (transparency) components.
 *
 * @param h hue component of the color, in degrees (in the range 0.0 to 360.0).
 * @param s saturation component of the color, in the range 0.0 to 1.0.
 * @param v value component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de ligne (teinte, saturation, valeur et alpha).
 *
 * @param h Teinte, en degrés (entre 0.0 et 360.0).
 * @param s Saturation, entre 0.0 to 1.0.
 * @param v Valeur, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
line_color_hsv (h:real, s:real, v:real, a:real);

/**
 * @~english
 * Selects the line color.
 * @~french
 * Choisit la couleur de ligne.
 * @~
 * @see @ref graph.doxy.h::line_color_hsv(h:real, s:real, v:real, a:real) "line_color_hsv h:real, s:real, v:real, a:real"
 */
line_color_hsv (h:real, s:real, v:real);

/**
 * @~english
 * Selects the line_color by its cyan, yellow, magenta, black, and alpha-channel (transparency) components.
 *
 * @param c cyan component of the color, in the range 0.0 to 1.0.
 * @param y yellow component of the color, in the range 0.0 to 1.0.
 * @param m magenta component of the color, in the range 0.0 to 1.0.
 * @param k black component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 * The default value is 1.0.
 *
 * @~french
 * Choisit la couleur de ligne (cyan, magenta, jaune, noir).
 *
 * @param c Composante cyan, entre 0.0 to 1.0.
 * @param y Composante jaune, entre 0.0 to 1.0.
 * @param m Composante magenta, entre 0.0 to 1.0.
 * @param k Composante noire, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
line_color_cymk (c:real, y:real, m:real, k:real, a:real);

/**
 * @~english
 * Selects the line color.
 * @~french
 * Choisit la couleur de ligne.
 * @~
 * @see @ref graph.doxy.h::line_color_cymk(c:real, y:real, m:real, k:real, a:real) "line_color_cymk c:real, y:real, m:real, k:real, a:real"
 */
line_color_cymk (c:real, y:real, m:real, k:real);

/**
 * @~english
 * Creates a stop point in the current gradient at the given position with the given color.
 *
 * @param pos position of stop point in the current gradient, in the range 0.0 to 1.0.
 * @param r red component of the color, in the range 0.0 to 1.0.
 * @param g green component of the color, in the range 0.0 to 1.0.
 * @param b blue component of the color, in the range 0.0 to 1.0.
 * @param a alpha-channel, transparency of the color. 0.0 is transparent and 1.0 is opaque.
 *
 * @~french
 * Crée un point fixe de la couleur donnée, dans le dégradé courant à la position donnée.
 *
 * @param pos Position du point, entre 0.0 to 1.0.
 * @param r Composante rouge, entre 0.0 to 1.0.
 * @param g Composante verte, entre 0.0 to 1.0.
 * @param b Composante bleue, entre 0.0 to 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0. 0.0 est transparent
 * et 1.0 est opaque. La valeur par défaut est 1.0.
 */
gradient_color (pos:real, r:real, g:real, b:real, a:real);

/**
 * @~english
 * Creates a texture with a linear gradient.
 * Draw a linear gradient with colors specified in the body
 * thanks to @ref gradient_color.
 *
 *  - @p start_x and @p start_y are the coordinates of the gradient start.
 *  - @p end_x and @p end_y are the coordinates of the gradient end.
 *  - @p w and @p h are the width and height of the resulting texture.
 *
 * @image html "images/linear_gradient.png"
 *
 * Code used to produce this image:
 * <a href="examples/linear_gradient.ddd">linear_gradient.ddd</a>
 *
 * @~french
 * Crée une texture formant un dégradé linéaire.
 * Dessine un dégradé linéaire avec les couleurs spécifiées dans le corps
 * du bloc @c linear_gradient grâce à @ref gradient_color.
 *
 *  - @p start_x and @p start_y sont les coordonnées de départ du dégradé.
 *  - @p end_x and @p end_y sont les coordonnées de fin du dégradé.
 *  - @p w and @p h sont la largeur et la hauteur de la texture de destination.
 *
 * @image html "images/linear_gradient.png"
 *
 * Le code de cet exemple est le suivant :
 * <a href="examples/linear_gradient.ddd">linear_gradient.ddd</a>
 */
linear_gradient (start_x:real, start_y:real, end_x:real, end_y:real, w:real, h:real, body:tree);

/**
 * @~english
 * Creates a texture with a radial gradient.
 * Draw a radial gradient with colors specified in the body
 * thanks to @ref gradient_color.
 *
 *  - @p cx and @p cy are the coordinates of the gradient center.
 *  - @p r is the gradient radius.
 *  - @p w and @p h are the width and height of the resulting texture.
 *
 * @image html "images/radial_gradient.png"
 *
 * Code used to produce this image:
 * <a href="examples/radial_gradient.ddd">radial_gradient.ddd</a>
 *
 * @~french
 * Crée une texture formant un dégradé radial.
 * Dessine un dégradé radial avec les couleurs spécifiées dans le corps
 * du bloc @c radial_gradient grâce à @ref gradient_color.
 *
 *  - @p cx and @p cy sont les coordonnées du centre du dégradé.
 *  - @p r est le rayon du dégradé.
 *  - @p w and @p h sont la largeur et la hauteur de la texture de destination.
 *
 * @image html "images/radial_gradient.png"
 *
 * Le code de cet exemple est le suivant :
 * <a href="examples/radial_gradient.ddd">radial_gradient.ddd</a>
 */
radial_gradient (cx:real, cy:real, r:real, w:real, h:real, body:tree);

/**
 * @~english
 * Creates a texture with a conical gradient.
 * Draw a conical gradient with colors specified in the body
 * thanks to @ref gradient_color.
 *
 *  - @p cx and @p cy are the coordinates of the gradient center.
 *  - @p teta is the gradient angle. This one must be specified in degrees between 0 and 360.
 *  - @p w and @p h are the width and height of the resulting texture.
 *
 * @image html "images/conical_gradient.png"
 *
 * Code used to produce this image :
 * <a href="examples/conical_gradient.ddd">conical_gradient.ddd</a>
 *
 * @~french
 * Crée une texture formant un dégradé conique.
 * Dessine un dégradé conique avec les couleurs spécifiées dans le corps
 * du bloc @c conical_gradient grâce à @ref gradient_color.
 *
 *  - @p cx and @p cy sont les coordonnées du centre du dégradé.
 *  - @p teta est l'angle du déradé, en degrés, entre 0 and 360.
 *  - @p w and @p h sont la largeur et la hauteur de la texture de destination.
 *
 * @image html "images/conical_gradient.png"
 *
 * Le code de cet exemple est le suivant :
 * <a href="examples/conical_gradient.ddd">conical_gradient.ddd</a>
 */
conical_gradient (cx:real, cy:real, teta:real, w:real, h:real, body:tree);

/**
 * @~english
 * Selects the texture.
 * Build a GL texture out of image file @p filename, and make it the current
 * texture.
 * Supported format include:
 *   - BMP  Windows Bitmap
 *   - GIF  Graphic Interchange Format (optional)
 *   - JPG  Joint Photographic Experts Group
 *   - JPEG Joint Photographic Experts Group
 *   - PNG  Portable Network Graphics
 *   - PBM  Portable Bitmap
 *   - PGM  Portable Graymap
 *   - PPM  Portable Pixmap
 *   - SVG  Scalable Vector Graphics
 *   - TIFF Tagged Image File Format
 *   - XBM  X11 Bitmap
 *   - XPM  X11 Pixmap
 *
 * @note Use @ref animated_texture to render animated bitmaps and @ref svg
 * to render animated SVG files.
 *
 * @~french
 * Crée une texture à partir d'un fichier image.
 * Les formats supportés incluent :
 *   - BMP  Windows Bitmap
 *   - GIF  Graphic Interchange Format (optional)
 *   - JPG  Joint Photographic Experts Group
 *   - JPEG Joint Photographic Experts Group
 *   - PNG  Portable Network Graphics
 *   - PBM  Portable Bitmap
 *   - PGM  Portable Graymap
 *   - PPM  Portable Pixmap
 *   - SVG  Scalable Vector Graphics
 *   - TIFF Tagged Image File Format
 *   - XBM  X11 Bitmap
 *   - XPM  X11 Pixmap
 *
 * @note Utilisez @ref animated_texture pour afficher des @a bitmaps animés, et
 * @ref svg pour afficher des fichiers SVG animés.
 */
texture(filename:text);

/**
 * @~english
 * Makes a previously defined texture active.
 * @note @c texture @c 0 deactivates the texture bound to the current texture
 * unit.
 *
 * @~french
 * Active une texture définie précédemment.
 * @note @c texture @c 0 désactive la texture associée à l'unité de texture
 * courante.
 */
texture(id:integer);

/**
 * @~english
 * Get current texture id.
 *
 * @~french
 * Renvoie l'identifiant de la texture courante.
 */
integer texture();

/**
 * @~english
 * Copy the current texture to another one.
 *
 * @~french
 * Copie la texture courante dans une autre texture.
 */
copy_texture();

/**
 * @~english
 * Check if a texture is bound at the specified unit.
 * @return true if a texture is bound, false otherwise.
 *
 * @~french
 * Teste si l'unité de texture courante a une texture.
 * @return vrai si une texture est présente, faux sinon.
 */
has_texture();

/**
 * @~english
 * Get current texture width.
 *
 * @~french
 * Renvoie la largeur de la texture courante.
 */
texture_width();

/**
 * @~english
 * Get current texture height.
 *
 * @~french
 * Renvoie la hauteur de la texture courante.
 */
texture_height();

/**
 * @~english
 * Get current texture type.
 * Returned type is defined in the
 * <a href="http://www.opengl.org/registry/#specfiles">OpenGL specification</a>.
 *
 * @~french
 * Renvoie le type de la texture courante.
 * Le type est définit dans la
 * <a href="http://www.opengl.org/registry/#specfiles">spécification OpenGL</a>.
 *
 */
texture_type();


/**
 * @~english
 * Selects current the texture unit.
 * Set the current texture unit to @p unit.
 *
 * Default value is 0.
 *
 * @note Only the fourth texture units can be used without shaders. The maximum number of texture units and coordinates depend on the graphic card.
 * @note @c shader_set can be used to link texture unit with a sampler inside shaders.
 * @see OpenGL documentation about GL_MAX_TEXTURE_COORDS, GL_MAX_TEXTURE_IMAGE_UNITS and GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
 *
 * @~french
 * Sélectionne l'unité de texture active.
 */
texture_unit (unit:integer);

/**
 * @~english
 * Get current texture unit.
 * @return [integer] current texture unit
 *
 * @~french
 * Renvoie l'unité de texture active.
 */
texture_unit();

/**
 * @~english
 * Create a GL animated texture.
 * Build a GL texture images from an animated image file and make it the
 * current texture. Supported format are:
 *   - GIF	Animated Graphic Interchange Format
 *   - MNG	Animated portable Network Graphics
 *
 * @note This primitive does not support animated SVGs. You may use the
 * @ref svg primitive to render animated SVGs.
 *
 * @~french
 * Crée une texture animée.
 * Les formats supportés sont :
 *   - GIF	Animated Graphic Interchange Format
 *   - MNG	Animated portable Network Graphics
 *
 * @note Pour les SVG animés, utilisez la primitve @ref svg.
 */
animated_texture(filename:text);

/**
 * @~english
 * Creates an animated texture from an SVG file.
 * Build a GL texture out of SVG file @p svg. This primitive supports
 * animated SVGs, in addition to non-animated ones. For non-animated SVGs,
 * however, it is recommended to use the @ref texture primitive, which uses
 * less system resources.
 *
 * @~french
 * Crée une texture animée à partir d'un fichier SVG.
 * Pour les fichiers SVG non animés, il est recommendé d'utiliser la primitive
 * @ref texture primitive qu iutilise moins de ressources système.
 */
svg(svg:text);

/**
 * @~english
 * Geometric transformation for texture.
 * Allows transformation to be applied to the current texture.
 * Transformation are specified in the body and can be a combination of
 *  - rotate
 *  - scale
 *  - translate
 *
 * @~french
 * Transformations géométriques des textures.
 * Permet d'appliquer des transformations géométriques à la
 * texture courante. Les transformations sont spécifiées dans @p body
 * et sont une combinaison de @ref rotate, @ref scale et @ref translate.
 *
 * @~
 * @see transforms
 */
texture_transform (body:tree);

/**
 * @~english
 * Texture wrapping.
 * Controls the wrapping of the texture along @c S and @c T axis.
 * @param s is the pre-transformation horizontal axis (like X).
 * @param t is the pre-transformation vertical axis (like Y).
 *
 * Default values are false, false.
 * @see OpenGL documentation about GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T
 *
 * @~french
 * Continuité des textures.
 * Contrôle la continuité de la texture courante le long des axes @c S et @c T.
 * @param s true pour activer GL_TEXTURE_WRAP_S.
 * @param t true pour activer GL_TEXTURE_WRAP_T.
 *
 * Par défaut, la répétition est désactivée (false, false).
 * @see La documentation OpenGL sur GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T.
 */
texture_wrap (s:boolean, t:boolean);

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
 *   - SVG  Scalable Vector Graphics
 *   - TIFF Tagged Image File Format
 *   - XBM  X11 Bitmap
 *   - XPM  X11 Pixmap
 *
 * Images are combined with the current color. Animated image files (bitmap or
 * SVG) appear fixed when displayed by the image primitives. If you need to
 * show animated files, use the texture primitives @ref animated_texture or
 * @ref svg.
 *
 * Texture are handled in the @ref graph_env module.
 * @{
 */

/**
 * Display an image.
 * Load an image from file @p filename.
 *  - @p x and @p y coordinate of the image center are specified in pixel
 *  - @p w and @p h are the width and height relative to the original size
 *    (for instance, set @p w to 1.0 to keep the original width)
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
 *  - @p w and @p h are the width and height of the image in pixel. If
 *    @p w or @p h is null (but not both), the image is scaled
 *    proportionally so that it keeps its original aspect ratio. If both
 *    parameters are null, nothing is displayed.
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
 * water-melon with the sphere primitive.
 * The @p w, @p h and @p d parameters are the dimensions along the @p x, @p y and
 * @p z axis, respectively.
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
 * Draws a torus.
 *
 * The torus is divided in @p slices and @p stacks. The higher the value of
 * these parametres are, the smoother the torus is (and longer the drawing is).
 * The torus's @p w, @p h and @p d are not aimed to be equals.
 * The minor radius of the torus is a ratio @p r of its basis. @p r is a real between 0.0 and 1.0.
 * If @p r is 0 the drawing is a sort of opened cylinder, if @p r is 1 the torus is closed.

 */
torus (x:real, y:real, z:real, w:real, h:real, d:real, slices:integer, stacks:integer, r:real);

/**
 * Draws a torus.
 *
 * Torus with diameter @p d located at (@p x, @p y, @p z).
 * Shorcut to
@code
torus x, y, z, d, d, d, 25, 25, r
@endcode
 */
torus (x:real, y:real, z:real, d:real, r:real);

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
