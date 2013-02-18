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
 * @par Image sources
 *
 * There are several primitives to display images: @ref image, @ref image_px, @ref texture.
 * All of them accept several forms to designate the source of the image:
 * - An absolute or relative file path (relative to the directory from where the
 * document was opened)
 * - A qualified path (such as <tt>image:file.jpg</tt>)
 * - A URL, such as <tt>http://www.taodyne.com/shop/img/logo.jpg</tt>.
 * Only HTTP (<tt>http://</tt>) and HTTPS (<tt>https://</tt>) URLs are
 * supported.
 *
 * @par Texture cache
 *
 * 2D texture created by @ref texture, @ref image or @ref image_px are
 * normally transferred to the graphics card as soon as they are loaded from
 * the specified file. More precisely, they are transferred to the OpenGL
 * implementation, which is then responsible for doing the appropriate memory
 * management, depending on the card's memory size, and the amount of textures
 * and other graphics data used by the application.
 *
 * In certain circumstances, it is necessary to control how texture
 * data are sent to OpenGL. For instance, when dealing with large amounts of
 * textures (several gigabytes), the OpenGL implementation
 * may return a "GL out of memory" error. In this case, limiting the amount of
 * data sent to OpenGL, while keeping the rest of the picture data in main
 * memory may help solve the problem. This can be achieved by the texture cache
 * which can be configured from the preference page, or with the primitives:
 * @ref texture_cache_mem_size, @ref texture_cache_gl_size,
 * @ref texture_compress, and @ref texture_mipmap.
 *
 * When a file is changed on disk, it is reloaded automatically. If a file is
 * deleted or renamed, the texture is replaced by a placeholder. If a
 * non-existent texture file is created, the texture is also updated.
 *
 * Textures loaded from URLs are currently not monitored for change. They are
 * downloaded once when they are first needed, then are never reloaded.
 *
 * @~french
 * @defgroup graph_env Environnement graphique (couleur des lignes, du remplissage, textures)
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
 * Le multi-texture est l'utilisation simultanée de plusieurs textures sur une
 * même forme géométrique.
 * Par exemple, le code qui suit (<a href="examples/multitexturing.ddd">multitexturing.ddd</a>)
 * affiche un rectangle et utilise une combinaison de deux textures, l'une
 * représentant un mur et l'autre un faisceau de lumière, afin de créer un
 * effet d'éclairage.
 *
 * @include multitexturing.ddd
 * @image html multitexturing.png "Démonstration : multitexturing.ddd"
 *
 * @par Sources d'images
 *
 * Plusieurs primitives permettent d'afficher des images: @ref image, @ref image_px,
 * @ref texture. Toutes acceptent plusieurs formes pour désigner la source de
 * l'image:
 * - Un chemin absolu ou relatif (au répertoire depuis lequel le document a
 * été ouvert)
 * - Un chemin qualifié (tel que <tt>image:fichier.jpg</tt>)
 * - Une URL, telle que <tt>http://www.taodyne.com/shop/img/logo.jpg</tt>.
 * Seules les URLs de type HTTP (<tt>http://</tt>) et HTTPS (<tt>https://</tt>)
 * sont supportées.
 *
 * @par Cache de textures
 *
 * Les textures bi-dimensionnelles créées par @ref texture, @ref image ou
 * @ref image_px sont en principe transférées vers la carte graphique dès
 * qu'elles sont chargées depuis le fichier image. Plus précisément, elles sont
 * confiées à l'implémentation OpenGL qui les stocke dans la carte ou
 * en mémoire principale en fonction de la mémoire disponible sur la carte et
 * de la quantité de textures ou d'autres données graphiques utilisées par
 * l'application.
 *
 * Dans certains cas il est nécessaire de contrôler quelle quantité de textures
 * est gérée par OpenGL. Par exemple, lorsque de grandes quantités de textures
 * sont chargées (plusieurs giga-octets), l'implémentation OpenGL peut
 * déclencher une erreur "dépassement de mémoire GL". Dans ce cas, le fait
 * de limiter la quantité de données envoyée vers OpenGL tout en conservant
 * le reste en mémoire principale peut permettre de résoudre le problème.
 * Cela se fait grâce au cache de textures, qui peut être configuré depuis
 * la page de préférences, ou grâce aux primitives
 * @ref texture_cache_mem_size, @ref texture_cache_gl_size,
 * @ref texture_compress, et @ref texture_mipmap.
 *
 * Lorsqu'un fichier d'image est modifié sur le disque, il est rechargé
 * automatiquement. S'il est effacé, la texture est remplacée par une image
 * par défaut (une mire colorée). Si un fichier non existent est créé,
 * la texture est également mise à jour.
 *
 * Les textures chargés depuis une URL ne sont actuellement pas rechargées
 * automatiquement. Aucun délai de validité n'est associé à une telle image,
 * et Tao Presentations ne vérifie pas si l'image change au cours du temps.
 *
 * @~
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
 * @c line_width @c "default" resets the line width to the default value (4).
 * @p lw is the width in pixels.
 *
 * @~french
 * Définit la largeur du trait pour le tracé des formes.
 *
 * @c line_width @c "default" reprend la valeur par défaut (4).
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
 * ligne sont tracées. Le bit zéro est utilisé en premier. Pour tracer une ligne
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
 *
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
 * @param r Composante rouge, entre 0.0 et 1.0.
 * @param g Composante verte, entre 0.0 et 1.0.
 * @param b Composante bleue, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * @param s Saturation, entre 0.0 et 1.0.
 * @param l Luminosité, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * @param s Saturation, entre 0.0 et 1.0.
 * @param v Valeur, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * Choisit la couleur de remplissage (cyan, magenta, jaune, noir et alpha).
 *
 * @param c Composante cyan, entre 0.0 et 1.0.
 * @param y Composante jaune, entre 0.0 et 1.0.
 * @param m Composante magenta, entre 0.0 et 1.0.
 * @param k Composante noire, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * This primitive affects the default fill color of the main window, or of the
 * current rendering target (@ref frame, @ref frame_texture, @ref canvas).
 * - When this primitive is called outside a @ref frame, @ref frame_texture
 * or @ref canvas block, it sets the clear color for the main drawing area.
 * This color is used to fill the drawing area (screen or window) before
 * anything is drawn. As such it can be considered the background color.
 * The clear color is used only when Tao Presentations reaches the drawing
 * phase. Therefore, the last @ref clear_color that has been executed during
 * the evaluation phase "wins". For instance:
 * @code
clear_color 0, 0, 0, 1         // Black
color "blue"
rectangle 0, 0, 200, 100
clear_color 0.6, 0.6, 0.6, 1   // Gray
 * @endcode
 * In the above example, the rectangle is drawn on a gray background, not
 * on a black one because the value <tt>0, 0, 0, 1</tt> (black) has
 * been replaced with <tt>0.6, 0.6, 0.6, 1</tt> (gray). @n
 * Here is another example:
 * @code
page "White background",
    color "blue"
    rectangle 0, 0, 200, 100
page "Black background",
    clear_color 0, 0, 0, 1     // Black
    color "red"
    rectangle 0, 0, 200, 100
 * @endcode
 * The first page has a white backround (the default), because the
 * <tt>clear_color</tt> line is not executed when page 1 is shown.
 * The second page has a black background because the last @ref clear_color
 * instruction executed before drawing is
 * <tt>clear_color 0, 0, 0, 1</tt>, which replaces the default white
 * with black.
 * - When the primitive is called inside a @ref frame, @ref frame_texture
 * or @ref canvas
 * block, it sets the clear color of the local drawing buffer, which can
 * have subtle effects on the rendering when the buffer is later blended
 * with the main draw buffer. The default
 * clear color of a @ref frame_texture is transparent white,
 * RGBA (1, 1, 1, 0).
 * In some cases you may want to change this color. For instance, look
 * at the border of the characters in the following example, with and
 * without the @ref clear_color line in the @ref frame_texture block.
 * @code
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
 * Définit la couleur d'effacement (clear color).
 * Cette primitive définit la couleur de remplissage de la fenêtre principale
 * ou de la cible de rendu en cours (@ref frame, @ref frame_texture,
 * @ref canvas).
 * - Lorsqu'elle est appelée en-dehors de tout bloc @ref frame,
 * @ref frame_texture ou @ref canvas, elle
 * définit la couleur de fond pour la zone de dessin principale.
 * Cette couleur est utilisée pour remplir l'écran avant tout tracé (la couleur
 * de fond).
 * Cette couleur n'est utilisée que lorsque Tao Presentations atteint la phase
 * d'affichage. Par conséquent, le dernier appel à @ref clear_color qui a été
 * exécuté "gagne". Par exemple :
 * @code
clear_color 0, 0, 0, 1         // Noir
color "blue"
rectangle 0, 0, 200, 100
clear_color 0.6, 0.6, 0.6, 1   // Gris
 * @endcode
 * Ici le rectangle est affiché sur un fond gris, pas noir, car la valeur
 * <tt>0, 0, 0, 1</tt> (noir) a été remplacée par
 * <tt>0.6, 0.6, 0.6, 1</tt> (gris) durant l'exécution et avant l'affichage.@n
 * Voici un autre exemple:
 * @code
page "White background",
    color "blue"
    rectangle 0, 0, 200, 100
page "Black background",
    clear_color 0, 0, 0, 1     // Noir
    color "red"
    rectangle 0, 0, 200, 100
 * @endcode
 * La première page a un fond blanc (le défaut), car la ligne
 * <tt>clear_color</tt> n'est pas exécutée lorsqu'on affiche la page 1.
 * La deuxième page est sur fond noir car le dernier appel à @ref clear_color
 * exécuté avant l'affichage est
 * <tt>clear_color 0, 0, 0, 1</tt>, qui remplace le fond blanc par défaut par
 * du noir.
 * - Quand elle est appelée dans un bloc @ref frame ou
 * @ref frame_texture, elle définit la couleur d'effacement de la zone de
 * dessin locale (une texture), ce qui peut avoir des effets subtils sur
 * le rendu lorsque la texture est ensuite mélangée avec l'affichage principal.
 * La couleur d'effacement par défaut dans @ref frame,
 * @ref frame_texture ou @ref canvas est le blanc transparent, RGBA (1, 1, 1, 0).
 * Dans certains cas il peut être utile de changer cette couleur. Par
 * exemple, observez le contour des caractères dans l'exemple suivant,
 * avec et sans la ligne @ref clear_color dans le bloc @ref frame_texture.
 * @code
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
 * The default color is transparent black, so that no line is drawn by default.
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
 *
 * Les modules peuvent également définir des couleurs.
 *
 * La couleur par défaut est le noir transparent, de sorte qu'aucune contour
 * n'est tracé par défaut.
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
 * @param r Composante rouge, entre 0.0 et 1.0.
 * @param g Composante verte, entre 0.0 et 1.0.
 * @param b Composante bleue, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * @param s Saturation, entre 0.0 et 1.0.
 * @param l Luminosité, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * @param s Saturation, entre 0.0 et 1.0.
 * @param v Valeur, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * Choisit la couleur de ligne (cyan, magenta, jaune, noir et alpha).
 *
 * @param c Composante cyan, entre 0.0 et 1.0.
 * @param y Composante jaune, entre 0.0 et 1.0.
 * @param m Composante magenta, entre 0.0 et 1.0.
 * @param k Composante noire, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * @param pos Position du point, entre 0.0 et 1.0.
 * @param r Composante rouge, entre 0.0 et 1.0.
 * @param g Composante verte, entre 0.0 et 1.0.
 * @param b Composante bleue, entre 0.0 et 1.0.
 * @param a Canal de transparance alpha, entre 0.0 et 1.0, où 0.0 est transparent
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
 * @warning The gradient origin is situated on the upper left edge of the texture.
 *
 * @image html "images/linear_gradient.png"
 *
 * Code used to produce this image:
 *
 * @~french
 * Crée une texture formant un dégradé linéaire.
 * Dessine un dégradé linéaire avec les couleurs spécifiées dans le corps
 * du bloc @c linear_gradient grâce à @ref gradient_color.
 *
 *  - @p start_x et @p start_y sont les coordonnées de départ du dégradé.
 *  - @p end_x et @p end_y sont les coordonnées de fin du dégradé.
 *  - @p w et @p h sont la largeur et la hauteur de la texture de destination. 
 *
 * @warning L'origine du dégradé se situe sur le bord en haut à gauche de la texture.
 *
 * @image html "images/linear_gradient.png"
 *
 * Le code de cet exemple est le suivant :
 * @~
@code
linear_grad -> 
    color "white" 
    linear_gradient 0, 0, 700, 700, 700, 700,
        gradient_color 0, 0.0, 0.0, 1.0, 1.0 
        gradient_color 0.5, 0.0, 1.0, 0.0, 1.0 
        gradient_color 1, 1.0, 0.0, 0.0, 1.0
     
    rectangle 0, 0, 250, 250
 
linear_grad
@endcode
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
 * @warning The gradient origin is situated on the upper left edge of the texture.
 *
 * @image html "images/radial_gradient.png"
 *
 * Code used to produce this image:
 *
 * @~french
 * Crée une texture formant un dégradé radial.
 * Dessine un dégradé radial avec les couleurs spécifiées dans le corps
 * du bloc @c radial_gradient grâce à @ref gradient_color.
 *
 *  - @p cx et @p cy sont les coordonnées du centre du dégradé.
 *  - @p r est le rayon du dégradé.
 *  - @p w et @p h sont la largeur et la hauteur de la texture de destination. 
 *
 * @warning L'origine du dégradé se situe sur le bord en haut à gauche de la texture.
 *
 * @image html "images/radial_gradient.png"
 *
 * Le code de cet exemple est le suivant :
 *@~
@code
radial_grad -> 
    color "white" 
    radial_gradient 350, 350, 350, 700, 700, 
        gradient_color 0, 0.0, 0.0, 1.0, 1.0 
        gradient_color 0.5, 0.0, 1.0, 0.0, 1.0 
        gradient_color 1, 1.0, 0.0, 0.0, 1.0 

    rectangle 0, 0, 250, 250 

radial_grad
@endcode
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
 * @warning The gradient origin is situated on the upper left edge of the texture.
 *
 * @image html "images/conical_gradient.png"
 *
 * Code used to produce this image :
 *
 * @~french
 * Crée une texture formant un dégradé conique.
 * Dessine un dégradé conique avec les couleurs spécifiées dans le corps
 * du bloc @c conical_gradient grâce à @ref gradient_color.
 *
 *  - @p cx et @p cy sont les coordonnées du centre du dégradé.
 *  - @p teta est l'angle du déradé, en degrés, entre 0 and 360.
 *  - @p w et @p h sont la largeur et la hauteur de la texture de destination.
 *
 * @warning L'origine du dégradé se situe sur le bord en haut à gauche de la texture.
 *
 * @image html "images/conical_gradient.png"
 *
 * Le code de cet exemple est le suivant :
 *@~
@code
conical_grad -> 
    color "white" 
    conical_gradient 350, 350, 30, 700, 700, 
        gradient_color 0, 0.0, 0.0, 1.0, 1.0 
        gradient_color 0.5, 0.0, 1.0, 0.0, 1.0 
        gradient_color 1, 1.0, 0.0, 0.0, 1.0
     
    rectangle 0, 0, 250, 250
 
conical_grad
@endcode
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
 * Cache a complex drawing to avoid unnecessary computations
 * When a drawing is the result of a complex computation, it is
 * possible to cache the result of that computation so that the
 * drawing displays faster.
 * The @a version parameter is a real number indicating the version
 * number for the cached drawing. Whenever the version changes, the
 * computation described in @a body is re-evaluated.
 *
 * @~french
 * Mémorise un dessin complexe pour minimiser  les calculs
 * Quand un dessinest le résultat d'un calcul complexe, il est
 * possible de mémoriser ce résultat pour que l'affichage soit plus
 * rapide. Le paramètre @a version est un nombre réel indicant le
 * numéro de version pour le dessin. Lorsque cette version change, le
 * calcul décrit dans @a body est effectué de nouveau.
 *
 * @~
 * @code
page "Graphs",
    scale 100, 100, 100
    rotatey 20 * time
    cached integer page_time,
        color "transparent"
        line_color "black"
        path
            W -> 0.001 * (page_time mod 60)
            for T in 0..5000 loop
                line_to 0.01 * T * sin(W * T), 0.01 * T * cos(W * T), sin(7 * W * T) 
 * @endcode
 */
cached(version:real; body:tree);


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
 * Selects the texture magnification filter function.
 * The specified value overrides the settings in the preference dialog.
 * It applies to the current layout (including later child layouts) for the
 * current texture unit. Possible values are:
 *
 * @~french
 * Sélectionne la fonction du filtre de grossissement des textures.
 * La valeur passée à cette primitive remplace la valeur choisie dans la
 * boite de préférences. Elle s'applique au <em>layout</em> en cours (y compris
 * les <em>layouts</em> fils à venir), pour l'unité de texture courante.
 * Les valeurs possibles sont:
 *
 * @~
 * - @c "NEAREST"
 * - @c "LINEAR"
 *
 * @~english
 * Internally, this primitive sets the @c GL_TEXTURE_MAG_FILTER.
 * @c NEAREST stands for @c GL_NEAREST and @c LINEAR stands for @c GL_LINEAR.
 * @n
 * The following example shows an image in "pixel perfect" mode. That is,
 * the image is shown with no interpolation or anti-aliasing whatsoever,
 * and one pixel in the source image gives one pixel on the screen. This
 * is useful for instance to display a picture that contains small text,
 * making sure the text remains legible. @n
 * Note that when the image is not magnified nor minified, it is the
 * magnification filter that applies.
 *
 * @~french
 * En interne, cette primitive définit @c GL_TEXTURE_MAG_FILTER.
 * @c NEAREST correspond à @c GL_NEAREST, @c LINEAR correspond à @c GL_LINEAR.
 * @n
 * L'exemple qui suit affiche une image sans interpolation ou
 * anti-crénelage. À un pixel de l'image correspond un pixel à l'écran.
 * C'est utile par exemple pour afficher une image contenant du texte de
 * petite taille, afin de conserver une lisibilité parfaite. @n
 * Notez que lorsque l'image n'est ni grossie ni réduite, c'est le filtre
 * de grossissement qui s'applique.
 * @~
 * @code
color "white"
texture "image.png"
texture_mag_filter "NEAREST"
rectangle 0, 0, texture_width, texture_height
 * @endcode
 */
texture_mag_filter(filter:text);

/**
 * @~english
 * Selects the texture minifying filter function.
 * The specified value overrides the settings in the preference dialog.
 * It applies to the current texture only.
 * Possible values are:
 *
 * @~french
 * Sélectionne la fonction du filtre de réduction des textures.
 * La valeur passée à cette primitive remplace la valeur choisie dans la
 * boite de préférences. Elle s'applique seulement à la texture en cours.
 * Les valeurs possibles sont:
 *
 * @~
 * - @c "NEAREST"
 * - @c "LINEAR"
 * - @c "NEAREST_MIPMAP_NEAREST"
 * - @c "LINEAR_MIPMAP_NEAREST"
 * - @c "NEAREST_MIPMAP_LINEAR"
 * - @c "LINEAR_MIPMAP_LINEAR"
 *
 * @~english
 * Mipmap modes apply only to static textures (@ref texture), not to
 * dynamic ones (@ref frame). @n
 * Internally, this primitive sets @c GL_TEXTURE_MIN_FILTER. The above
 * values correspond to the OpenGL modes of similar name (@c NEAREST stands
 * for @c GL_NEAREST, and so on). @n
 *
 * @~french
 * Les modes mipmap ne s'appliquent qu'aux textures statiques (@ref
 * texture) et non aux textures dynamiques (@ref frame). @n
 * En interne, cette primitive définit @c GL_TEXTURE_MIN_FILTER.
 * Chaque valeur correspond aux mode OpenGL qui porte un nom similaire
 * (@c NEAREST correspond à @c GL_NEAREST, etc.). @n
 */
texture_min_filter(filter:text);

/**
 * @~english
 * Defines the amount of main memory allocated to textures.
 * The application will limit the amount of main memory used to cache textures
 * loaded from files to the specified size, in bytes. When the cache is full
 * and a new file has to be loaded, the least recently used textures are
 * purged from main memory. 20% of @p bytes are freed.
 * @~french
 * Définit la quantité de mémoire allouée aux textures.
 * L'application limite l'utilisation de mémoire pour le cache de textures à
 * la taille spécifiée, en octets. Lorsque le cache est plein et qu'un nouveau
 * fichier doit être chargé, les textures qui n'ont pas été utilisées récemment
 * sont supprimées.
 * 20% de @p bytes sont libérés.
 */
texture_cache_mem_size(bytes:integer);

/**
 * @~english
 * Defines the amount of texture data sent to OpenGL.
 * The application will limit the amount of texture data sent to OpenGL to
 * the specified size, in bytes. When the specified size is reached and a new
 * texture has to be sent to OpenGL, the least recently used textures are
 * purged from GL memory. 20% of @p bytes are freed.
 * @~french
 * Définit la quantité de textures envoyées à OpenGL.
 * L'application limite la quantité de textures envoyées à OpenGL à
 * la taille spécifiée, en octets. Lorsque la taille est atteinte et qu'une
 * nouvelle texture doit être envoyée à OpenGL, les textures qui n'ont pas été
 * utilisées récemment sont supprimées de la mémoire GL.
 * 20% de @p bytes sont libérés.
 */
texture_cache_gl_size(bytes:integer);

/**
 * @~english
 * Invalidates stale network images.
 * Causes the texture cache to check all network images (the ones which have
 * been downloaded over HTTP or HTTPS), and purge the images that
 * have changed on the server.
 * @note Currently, all network images are purged, whether they are stale
 * or not.
 * @~french
 * Invalide les images réseau qui ont changé.
 * Le cache de textures vérifie si les images chargées depuis le réseau
 * (HTTP ou HTTPS) ont changé sur le serveur, et supprime du cache celles
 * qui sont devenues invalides.
 * @note Actuellement, toutes les images réseau sont supprimées du cache.
 */
texture_cache_refresh();

/**
 * @~english
 * Enables or disables creation of mipmaps for new textures.
 * @~french
 * Active ou désactive la création de mipmaps pour les nouvelles textures.
 */
texture_mipmap(enable:boolean);

/**
 * @~english
 * Enables or disables compression of new textures.
 * Controls whether textures are to be used in their
 * compressed form (lower quality but reduced GL memory usage and possibly
 * higher performance). Changing this setting does not cause
 * existing textures to be re-created. @n
 * When texture compression is enabled, compressed texture files
 * are loaded preferably over ther uncompressed version to speedup the
 * loading phase. See @ref texture_save_compressed.
 * @~french
 * Active ou désactive la compression des nouvelles textures.
 * Permet de contrôler si les textures sont utilisées sous forme compressée
 * (qualité moindre, mais utilisation de mémoire plus faible et
 * possiblement meilleures performances). Les textures déjà chargées ne
 * sont pas affectées pas un changement de ce paramètre. @n
 * Lorsque la compression est activée, le programme charge de préférence
 * une version pré-compressée de la texture si elle existe. Cf.
 * @ref texture_save_compressed.
 */
texture_compress(enable:boolean);

/**
 * @~english
 * Enables or disables creation of compressed texture files.
 * When set to true, any texture loaded from disk and used in compressed
 * form (see  @ref texture_compress) will be cached to disk.
 * The file name is the original texture file with the <tt>.compressed</tt>
 * suffix appended. Changing this setting does not cause
 * existing textures to be re-created.
 *
 * @~french
 * Active ou désactive la création de fichiers de texture compressés.
 * Lorsque ce mode est activé, toute nouvelle texture chargée depuis un
 * fichier et utilisée sous forme compressée (cf. @ref texture_compress)
 * sera mise en cache sur disque.
 * Le nom du fichier compressé est celui de la texture originale, avec
 * l'extension supplémentaire <tt>.compressed</tt>.
 * Les textures déjà chargées ne sont pas affectées pas un changement de
 * ce paramètre.
 */
texture_save_compressed(enable:boolean);

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
 * When @p s is @c true, @c GL_TEXTURE_WRAP_S is set to @c GL_REPEAT,
 * otherwise @c GL_CLAMP_TO_EDGE is used. The same goes for @p t which
 * controls @c GL_TEXTURE_WRAP_T.
 * @param s true to activate GL_TEXTURE_WRAP_S.
 * @param t true to activate GL_TEXTURE_WRAP_T.
 *
 * Default values are false, false.
 * @see OpenGL documentation about @c GL_TEXTURE_WRAP_S and @c GL_TEXTURE_WRAP_T.
 *
 * @~french
 * Continuité des textures.
 * Contrôle la continuité de la texture courante le long des axes @c S et @c T.
 * Lorsque @p s vaut @c true, @c GL_TEXTURE_WRAP_S vaut @c GL_REPEAT,
 * sinon il vaut @c GL_CLAMP_TO_EDGE. De la même manière,  t permet de
 * changer la valeur de @c GL_TEXTURE_WRAP_T.
 * @param s true pour activer GL_TEXTURE_WRAP_S.
 * @param t true pour activer GL_TEXTURE_WRAP_T.
 *
 * Par défaut, la répétition est désactivée (false, false).
 * @see La documentation OpenGL sur @c GL_TEXTURE_WRAP_S and @c GL_TEXTURE_WRAP_T.
 */
texture_wrap (s:boolean, t:boolean);

/** @} */

/**
 * @~english
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
 * Textures are handled in the @ref graph_env module.
 *
 * @~french
 * @defgroup graph_img Gestion des images
 * @ingroup Graphics
 * Les primitives de ce groupe permettent de lire des fichiers d'images et
 * d'obtenir des informations sur ces images.
 * Les formats supportés sont les suivants :
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
 * Les images sont combinées avec la couleur de remplissage courante.
 * Les fichiers d'images animées (GIF, SVG) sont fixes lorsqu'elles sont
 * affichées par la primitive @ref image. Si vous devez afficher de telles
 * animations, utilisez les primitives de textures @ref animated_texture ou
 * @ref svg.
 *
 * Les textures sont décrites dans le groupe @ref graph_env.
 *
 * @~
 * @{
 */

/**
 * @~english
 * Display an image.
 * Load an image from file @p filename.
 *  - @p x and @p y coordinate of the image center are specified in pixel
 *  - @p w and @p h are the width and height relative to the original size
 *    (for instance, set @p w to 1.0 to keep the original width)
 * @~french
 * Affiche une image.
 * Charge une image depuis le fichier @p filename.
 *  - @p x et @p y sont les coordonnées du centre de l'image
 *  - @p w et @p h sont la largeur et la hauteur relativement à l'image
 *    originale (1.0 pour conserver la taille d'origine)
 * @~
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
 * @~english
 * Display an image.
 * Load an image from file @p filename.
 *  - @p x and @p y coordinate of the image center are specified in pixel
 *  - @p w and @p h are the width and height of the image in pixel. If
 *    @p w or @p h is null (but not both), the image is scaled
 *    proportionally so that it keeps its original aspect ratio. If both
 *    parameters are null, nothing is displayed.
 * @~french
 * Affiche une image.
 * Charge une image depuis le fichier @p filename.
 *  - @p x et @p y sont les coordonnées du centre de l'image
 *  - @p w et @p h sont la largeur et la hauteur de l'image en pixels. Si
 *    @p w ou @p h est nul (mais pas les deux), l'image est redimensionnée de
 *    manière proportionnelle. Si les deux paramètres sont nuls, rien ne
 *    s'affiche.
 * @~
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
 * @~english
 * Get image size.
 * Return the image size of the @p img image.
 * @return [infix] a tree containing <tt>w, h</tt>
 * @~french
 * Renvoie la taille de l'image.
 * @return [infix] la largeur et la hauteur de l'image, en pixels, sous la
 * forme <tt>w, h</tt>.
 */
image_size(img:text);

/** @} */

/**
 * @~english
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
 * @~french
 * @defgroup graph_path Segments et courbes
 * @ingroup Graphics
 * Création de segments et de courbes 2D ou 3D, sommet par sommet, sous forme
 * de chemin graphique.
 *
 * Un chemin graphique est une forme géométique faite de segments de droites et
 * de courbes. Pour le construire, vous précisez où vous posez le crayon
 * (@ref move_to), vous tracez un segment avec @ref line_to, ou une courbe avec
 * @ref quad_to ou @ref cubic_to. Les coordonnées peuvent être absolues ou
 * relatives au point précédent.
 *
 * Lorsque le chemin graphique se trouve dans un bloc @ref shape, vous pouvez
 * manipuler les sommets grâce à la souris.
 *
 * Exemple :
 * <a href="examples/path_samples.ddd">path_samples.ddd</a>
 *
 * @~
 * @{
 */

/**
 * @~english
 * Opens a graphic path environment.
 *
 * A graphic path is a collection of segments of lines or curves
 * (quadrics or cubics).
 *
 * @~french
 * Crée un chemin graphique.
 *
 * Un chemin graphique est une suite d'un nombre variable de segments de droites, de
 * quadriques et de cubiques.
 */
path (t:tree);

/**
 * @~english
 * Moves path cursor to a specific point without drawing.
 *
 * Shortcut for 2D is also available:
 * @~french
 * Déplace le curseur de chemin graphique sans tracer.
 *
 * Un raccourci pour la 2D est disponible :
 * @~
 * @code move_to x:real, y:real @endcode
 */
move_to (x:real, y:real, z:real);

/**
 * @~english
 * Draws a straight line by moving path cursor to the specified point.
 *
 * Shortcut for 2D is also available:
 * @~french
 * Trace un segment de droite en déplaçant le curseur de chemin graphique.
 *
 * Un raccourci pour la 2D est disponible :
 * @~
 * @code line_to x:real, y:real @endcode
 */
line_to (x:real, y:real, z:real);

/**
 * @~english
 * Adds a quadric segment to the current path.
 *
 * A quadric segment of path is a curve where you specify the two
 * endpoints and one control point that controls the curve itself.
 * (@p cx, @p cy, @p cz) defines the control point.
 * (@p x, @p y, @p z) is the end point.
 *
 * Shortcut for 2D is also available :
 * @~french
 * Trace une quadrique en déplaçant le curseur de chemin graphique.
 *
 * Une quadrique est une courbe définie par deux sommets et un point de
 * contrôle qui en définit la courbure.
 * (@p cx, @p cy, @p cz) définit le point de contrôle.
 * (@p x, @p y, @p z) est la fin de la courbe.
 *
 * Un raccourci pour la 2D est disponible :
 * @~
 * @code quad_to cx:real, cy:real, x:real, y:real @endcode
 * @image html quad_path.png
 */
quad_to (cx:real, cy:real, cz:real, x:real, y:real, z:real);

/**
 * @~english
 * Adds a cubic segment to the current path.
 *
 * A cubic segment of path is a curve where you specify the two endpoints
 * and two control points that control two curves (like S).
 * (@p c1x, @p c1y, @p c1z) defines the first control point.
 * (@p c2x, @p c2y, @p c2z) defines the second control point.
 * (@p x, @p y, @p z) is the end point.
 *
 * Shortcut for 2D is also available :
 * @~french
 * Trace une cubique en déplaçant le curseur de chemin graphique.
 *
 * Une cubique est une courbe définie par deux sommets et deux points de
 * contrôle qui en définissent la courbure.
 * (@p c1x, @p c1y, @p c1z) définit le premier point de contrôle.
 * (@p c2x, @p c2y, @p c2z) définit le deuxième point de contrôle.
 * (@p x, @p y, @p z) est la fin de la courbe.
 *
 * Un raccourci pour la 2D est disponible :
 * @~
 * @code cubic_to c1x:real, c1y:real, c2x:real, c2y:real, x:real, y:real @endcode
 * @image html cubic_path.png
 */
cubic_to (c1x:real, c1y:real, c1z:real, c2x:real, c2y:real, c2z:real,
          x:real, y:real, z:real);

/**
 * @~english
 * Adds a line segment to the current path relative to current position.
 * @~french
 * Trace un segment de droite relativement à la position courante.
 */
line_relative (x:real, y:real, z:real);

/**
 * @~english
 * Moves without tracing current path cursor relative to current position.
 * @~french
 * Déplace sans tracer le curseur de chemin graphique relativement à la position courante.
 */
move_relative (x:real, y:real, z:real);

/**
 * @~english
 * Closes current path by a straight line to the start point.
 *
 * It creates a loop in the path by drawing a straight line from
 * current point to last move_to point or the start point.
 * @note It does not close the path environment. One can continue to add segment to current path.
 *
 * @~french
 * Ferme le chemin graphique.
 *
 * Crée une boucle dans le chemin graphique en dessinant une ligne droite
 * entre la position courante du curseur et le dernier @ref move_to ou
 * @ref move_relative, ou le point de départ.
 * @note Cette primitive ne termine pas l'environnement @ref path.
 * Vous pouvez continuer à ajouter des segments ou des courbes au chemin.
 */
close_path ();

/**
 * @~english
 * Sets the style of the path endpoints.
 *
 * @p s is the start point and @p e is the end point.
 *
 * Only very beginning and very end of the path can be styled. If a
 * text is added at the end of the path, the end style won't be applied
 * to the line end (because end of the line is not the end of the path).
 *
 * Supported symbols are:
 * @~french
 * Définit le style des extrémités d'un chemin graphique.
 *
 * @p s est le point de départ et @p e est le point terminal.
 *
 * Seuls les véritables débuts et fins d'un chemin peuvent recevoir un
 * symbole. Si du texte est ajouté à la fin du chemin, le style de fin n'est
 * pas appliqué (puisque la fin de la ligne n'est pas la fin du chemin).
 *
 * Les symboles possibles sont :
 * @~
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

#if 0
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
#endif

/** @} */


/**
 * @~english
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
 * @~french
 * @defgroup graph_2D Formes géométriques 2D
 * @ingroup Graphics
 * Crée des formes géométriques en deux dimensions.
 *
 * Toutes les primitives 2D définissent des formes à deux dimensions, mais ces
 * formes peuvent être ensuite déplacées, tournées et redimensionnées dans
 * l'espace 3D. Quand une forme 2D est définie, sa coordonnée Z est 0.
 *
 * Pour toutes les primitives 2D :
 * -  @p x et @p y représentent le centre de la forme,
 *   - L'axe [0, x) x > 0 est orienté vers la droite,
 *   - L'axe [0, y) y > 0 est orienté vers le haut,
 * -  @p h et @p w est la largeur et la hauteur en pixels.
 *
 * Voici des exemples de formes 2D :
 * <a href="examples/2D_samples.ddd">2D_samples.ddd</a>
 *
 * Dans tous les exemple de cette page :
 * - en orange foncé, le résultat de la primitive documentée,
 * - en noir, des illustrations des paramètres de la primitive,
 * - en jaune, des points de repère.
 *
 * @~
 * @{
 */

/**
 * @~english
 * Draws a point.
 *
 * This primitive draws an OpenGL point. @p s is the point size in
 * pixels. It is @b not affected by the scaling factor.
 *
 * @~french
 * Affiche un point.
 *
 * Cette primitive dessine un point OpenGL. @p s est la taille du point en
 * pixels. Elle n'est @b pas affectée par les facteurs d'échelle.
 */
point (x:real, y:real, z:real, s:real);

/**
 * @~english
 * Draws a rectangle.
 *
 *  The rectangle is centered at (@p x, @p y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate <tt>(x+w/2, y-h/2)</tt>
 *  - top left corner is at coordinate <tt>(x-w/2, y+h/2)</tt>
 *  - top right corner is at coordinate <tt>(x+w/2, y+h/2)</tt>
 *
 * @~french
 * Affiche un rectangle.
 *
 * Le rectangle est centré en (@p x, @p y), la largeur est @p w et la hauteur
 * est @p h.
 *  - Le coin inférieur gauche est en <tt>(x-w/2, y-h/2)</tt>
 *  - Le coin inférieur droit est en <tt>(x+w/2, y-h/2)</tt>
 *  - Le coin supérieur gauche est en <tt>(x-w/2, y+h/2)</tt>
 *  - Le coin supérieur droit est en <tt>(x+w/2, y+h/2)</tt>
 * @~
 * @image html rectangle.png
 */
rectangle (x:real, y:real, w:real, h:real);

/**
 * @~english
 * Draws a rounded rectangle.
 *
 *  Draw a rounded rectangle with radius @p r for the rounded corners.
 *  The rectangle is centered in (@p x, @p y), with width @p w and height @p h.
 *  - Bottom left corner is at coordinate <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate <tt>(x+w/2, y-h/2)</tt>
 *  - top left corner is at coordinate <tt>(x-w/2, y+h/2)</tt>
 *  - top right corner is at coordinate <tt>(x+w/2, y+h/2)</tt>
 *
 * @~french
 * Affiche un rectangle à coins arrondis.
 *
 * @p r est le rayon des coins arrondis.
 * Le rectangle est centré en (@p x, @p y), la largeur est @p w et la hauteur
 * est @p h.
 *  - Le coin inférieur gauche est en <tt>(x-w/2, y-h/2)</tt>
 *  - Le coin inférieur droit est en <tt>(x+w/2, y-h/2)</tt>
 *  - Le coin supérieur gauche est en <tt>(x-w/2, y+h/2)</tt>
 *  - Le coin supérieur droit est en <tt>(x+w/2, y+h/2)</tt>
 * @~
 * @image html rounded_rectangle.png
 */
rounded_rectangle (x:real, y:real, w:real, h:real, r:real);


/**
 * @~english
 * Draws a rectangle with elliptical sides.
 *
 * The ratio @p r is a real between 0.0 and 1.0. With ratio 0.0 the elliptical
 * rectangle is an ellipse, and with ratio 1.0 the elliptical rectangle is a
 * rectangle.
 *
 * @~french
 * Affiche un rectangle à côtés elliptiques.
 *
 * Le rapport @p r est un réel entre 0.0 et 1.0. Lorsque @p r vaut 0.0 le
 * rectangle elliptique est une ellipse, lorsqu'il vaut 1.0 on obtient un
 * rectangle.
 * @~
 * @image html elliptical_rectangle.png
 */
elliptical_rectangle (x:real, y:real, w:real, h:real, r:real);

/**
 * @~english
 * Draws an ellipse.
 *
 * The ellipse is centered at (@a x, @a y) with width @a w and height @a h.
 *
 * @~french
 * Affiche une ellipse.
 *
 * L'ellipse est inscrite dans le rectangle centré en (@a x, @a y), de largeur
 * @a w et de hauteur @a h.
 * @~
 * @image html ellipse.png
 *
 */
ellipse (x:real, y:real, w:real, h:real);

/**
 * @~english
 * Draws a circle.
 * Same as <tt>ellipse x, y, r, 2*r</tt>.
 *
 * @~french
 * Affiche un cercle.
 * Équivalent à <tt>ellipse x, y, r, 2*r</tt>.
 */
circle (x:real, y:real, r:real);

/**
 * @~english
 * Draws an elliptic sector.
 *
 * Elliptic sector centered around (@p x, @p y) that occupies the given
 * rectangle, beginning at the specified @p start angle and extending
 * @p sweep degrees counter-clockwise. Angles are specified in
 * degrees. Clockwise arcs can be specified using negative angles.
 *
 * @~french
 * Affiche un secteur elliptique.
 *
 * Le secteur elliptique est inscrit dans le rectangle centré en (@p x, @p y).
 * Il commence à l'angle @p start (en degrés) et s'étend sur @p sweep degrés
 * dans le sens contraire des aiguilles d'une montre. Des arcs qui
 * s'étendent dans l'autre sens peuvent être tracés en utilisant des angles
 * négatifs.
 * @~
 * @image html ellipse_arc.png
 */
ellipse_arc (x:real, y:real, w:real, h:real, start:real, sweep:real);

/**
 * @~english
 * Draws an isoceles triangle.
 *
 * Draws an isoceles triangle centered at (@p x, @p y), with width @p w and
 * height @p h.
 *  - Bottom left corner is at coordinate  <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate  <tt>(x+w/2, y-h/2)</tt>
 *  - Top corner is at coordinate  <tt>(x, y+h/2)</tt>
 *
 * @~french
 * Affiche un triangle isocèle.
 *
 * Affiche un triangle isocèle centré en (@p x, @p y), de largeur @p w et
 * de hauteur @p h.
 *  - L'angle inférieur gauche est en <tt>(x-w/2, y-h/2)</tt>
 *  - L'angle inférieur droit est en <tt>(x+w/2, y-h/2)</tt>
 *  - Le sommet est en <tt>(x, y+h/2)</tt>
 * @~
 * @image html triangle.png
 */
triangle (x:real, y:real, w:real, h:real);

/**
 * @~english
 * Draws a right triangle.
 *
 *  Draw a right triangle with hypotenuse centered in (@p x, @p y), with width @p w and height @p h.
 *  Right angle is the bottom left one.
 *  - Bottom left corner is at coordinate <tt>(x-w/2, y-h/2)</tt>
 *  - Bottom right corner is at coordinate  <tt>(x+w/2, y-h/2)</tt>
 *  - top corner is at coordinate  <tt>(x-w/2, y+h/2)</tt>
 *
 * @~french
 * Affiche un triangle rectangle.
 * L'hypoténuse est centrée en (@p x, @p y), les autres côtés sont de dimension @p w
 * et @p h.
 * @~
 * @image html right_triangle.png
 */
right_triangle (x:real, y:real, w:real, h:real);


/**
 * @~english
 * Draws an arrow.
 *
 * The arrow is centered at (@p x, @p y) and is contained in a bounding box of
 * @p w by @p h pixels. @p head is the length of the arrow head in pixels.
 * @p tail is the size of the the arrow relative to the overall witdh. @p tail
 * is a real comprised between 0 and 1.
 *
 * @bug why head is in pixel and tail is a ratio ? Because when you want to
 * resize the arrow width you generaly want to increase the tail size, but not
 * the head's one, and when you resize the height, you want to keep the aspect ratio.
 *
 * @~french
 * Affiche une flèche.
 *
 * La flèche est inscrite dans un rectangle centré en (@p x, @p y), de largeur
 * @p w et de hauteur @p h pixels. @p head est la longueur de la tête de la
 * flèche en pixels.
 * @p tail est la taille de la queue de la flèche, relativement à la taille
 * totale. C'est un réel compris entre 0.0 and 1.0.
 *
 * @~
 * @image html arrow.png
 */
arrow (x:real, y:real, w:real, h:real, head:real, tail:real);

/**
 * @~english
 * Creates a double arrow.
 *
 *  Draw a symetric double arraw. Similar to arrow (x:real, y:real, w:real, h:real, head:real, tail:real)
 * but with two heads.
 * @see See arrow for more details.
 *
 * @~french
 * Affiche une flèche à deux pointes.
 * @see @ref arrow
 *
 * @~
 * @image html double_arrow.png
 *
 */
double_arrow (x:real, y:real, w:real, h:real, head:real, body:real);

/**
 * @~english
 * Draws a star.
 *
 * This primitive defines a regular @p p-branch star centered at (@p x,@p y).
 * Width is @p w, height is
 * @p h. @p r is a real between 0 and 1 that defines the aspects of the
 * branches. When @p r approaches 0, the branches get thinner. When @p r
 * gets closer to 1, the star degenerates into a regular polygon.
 *
 * @~french
 * Affiche une étoile.
 *
 * Cette primitive définit une étoile régulière à p branches inscrite dans le
 * rectangle centré en (@p x, @p y), de largeur @p w, et de hauteur @p h.
 * @p r est un réel entre 0.0 et 1.0 qui définit l'aspect des branches.
 * Lorsque @p r est proche de 0.0, les branches deviennent plus fines.
 * Lorsque @p r est proche de 1.0, l'étoile dégénère en polygone régulier.
 *
 * @~
 * @image html star.png
 */
star (x:real, y:real, w:real, h:real, p:integer, r:real);

/**
 * @~english
 * Draws a star polygon.
 *
 * This primitive draws a regular star polygon centered at (@p x, @p y).
 * The star is obtained by placing @p p vertices regularly spaced on a
 * circle, to form a regular polygon. Then every @p q th vertices are
 * connected together.
 *
 * @~french
 * Affiche un polygone étoilé.
 *
 * Cette primitive définit une étoile régulière centrée en (@p x, @p y).
 * L'étoile est obtenue en plaçant @p p sommets régulièrement espacés sur un
 * cercle, pour former un polygone régulier. Puis, les sommets sont reliés,
 * tous les @p q sommets.
 *
 * @~
 * @image html star_polygon.png
 */
star_polygon (x:real, y:real, w:real, h:real, p:integer, q:integer);

/**
 * @~english
 * Draws a speech balloon.
 *
 * A speech ballon is made of:
 *   - a rounded rectangle centered at (@p x, @p y), with width @p w,
 *     height @p h and radius @p r, and
 *   - a tail ending at point (@p ax, @p ay). The tail width is determined
 *     automatically. The tail is not made of straight lines; it has a
 *     rounded shape. It is directed from the end point to the center of
 *     the speech balloon.
 *
 * @~french
 * Affiche une bulle de texte.
 *
 * La bulle est composée des éléments suivants :
 * - un rectangle à coins arrondis, centré en (@p x, @p y), de largeur @p w et
 *   de hauteur @p h. Le rayon des coins est @p r.
 * - une queue qui commence au point (@p ax, @p ay). La largeur est déterminée
 *   automatiquement.
 * @~
 * @image html speech_balloon.png
 *
 */
speech_balloon (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real);

/**
 * @~english
 * Draws a callout.
 *
 * A callout is made of:
 *   - a rounded rectangle centered at (@p x, @p y), with width @a w,
 *     height @p h and radius @p r, and
 *   - a triangular tail, which connects point (@p ax, @p ay) to the
 *     closest point on the rounded rectangle. The base of the tail has
 *     a width of @p tw pixels.
 *
 * @~french
 * Affiche une bulle de texte.
 *
 * La bulle est composée des éléments suivants :
 * - un rectangle arrondi, centré en (@p x, @p y), de largeur @p w et
 *   de hauteur @p h. Le rayon des coins est @p r.
 * - une queue triangulaire, qui relie le point (@p ax, @p ay) au point le plus
 *   proche sur le rectangle arrondi. La base de la queue a une largeur de
 *   @p tw pixels.
 * @~
 * @image html callout.png
 */
callout (x:real, y:real, w:real, h:real, r:real, ax:real, ay:real, tw:real);

/**
 * @~english
 * Draws a regular polygon.
 *
 * This primitives draws a regular polygon with @p p vertex.
 * @image html polygon.png
 * Shortcuts have been defined for polygon from 3 to 20 vertices.
 *
 * @~french
 * Affiche un polygone régulier.
 *
 * Cette primitive affiche un polygone régulier à @p p sommets.
 * @image html polygon.png
 * Des raccourcis existent pour les polygones de 3 à 20 sommets :
 * @~
 * -# N/A
 * -# N/A
 * -# equilateral_triangle
 * -# tetragon
 * -# pentagon
 * -# hexagon
 * -# heptagon
 * -# octagon
 * -# nonagon
 * -# decagon
 * -# hendecagon
 * -# dodecagon
 * -# tridecagon
 * -# tetradecagon
 * -# pentadecagon
 * -# hexadecagon
 * -# heptadecagon
 * -# octadecagon
 * -# enneadecagon
 * -# icosagon
 */
polygon (x:real, y:real, w:real, h:real, p:integer);

/**
 * @~english
 * Synonym for @ref star_polygon.
 * Shortcuts has been defined for polygram from 5 to 10 vertexes.
 *
 * @~french 
 * Synonyme de @ref star_polygon.
 * Des raccourcis existent pour des polygrams de 5 à 10 sommets.
 *
 * @~
 * - pentagram       p = 5, q = 2
 * - hexagram        p = 6, q = 2
 * - star_of_david   p = 6, q = 2
 * - heptagram       p = 7, q = 2
 * - star_of_lakshmi p = 8, q = 2
 * - octagram        p = 8, q = 3
 * - nonagram        p = 9, q = 3
 * - decagram        p =10, q = 3
 */
polygram (x:real, y:real, w:real, h:real, p:integer, q:integer);

/** @} */

/**
 * @~english
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
 * - shapes' size including those that got round part, is given by width, heigt and depth of the bounding box.
 *
 * Examples: <a href="examples/3D_samples.ddd">3D_samples.ddd</a>
 *
 * Other 3D samples are available in @ref Lighting module.
 *
 * @~french
 * @defgroup graph_3D Formes géométriques 3D
 * @ingroup Graphics
 * Crée des formes géométriques en trois dimensions.
 *
 * Pour toutes les primitives 3D :
 * -  @p x, @p y, et @p z sont les coordonnées du centre de la forme
 *   - L'axe [0, x) x > 0 est orienté vers la droite,
 *   - L'axe [0, y) y > 0 est orienté vers le haut,
 *   - L'axe [0, z) z > 0 est orienté vers l'utilisateur (hors de l'écran).
 * -  @p h, @p w et @p d sont respectivement la hauteur, la largeur et la profondeur de la
 *    forme,
 * - Lorsque la forme a des parties arondies, les dimensions sont celles du
 *   pavé englobant.
 *
 * Exemples : <a href="examples/3D_samples.ddd">3D_samples.ddd</a>
 *
 * D'autres exemples sont disponibles dans @ref Lighting.
 *
 * @~
 * @{
 */

/**
 * @~english
 * Draws a sphere.
 *
 * The sphere is divided in @p slices and @p stacks. The higher the value of
 * these parametres are, the smoother the sphere is (and longer the drawing is).
 * The sphere's @p w, @p h and @p d are not aimed to be equals. One can draw a
 * water-melon with the sphere primitive.
 * The @p w, @p h and @p d parameters are the dimensions along the @p x, @p y and
 * @p z axis, respectively.
 *
 * @~french
 * Affiche une sphère.
 *
 * La sphères est divisée en @p slices tranches horizontales et @p stacks
 * tranches verticales. Plus ces paramètres ont une valeur élevée et plus la
 * sphère apparait lisse (mais plus elle demande du temps pour s'afficher).
 * On peut tracer une sphère déformée en donnant des valeurs différentes à
 * @p w, @p h et @p d.
 * @~
 * @image html sphere.png
 */
sphere (x:real, y:real, z:real, w:real, h:real, d:real, slices:integer, stacks:integer);

/**
 * @~english
 * Draws a sphere.
 *
 * Sphere with diameter @p d located at (@p x, @p y, @p z).
 * Equivalent to:
 *
 * @~french
 * Affiche une sphère.
 *
 * Sphère de diamètre @p d centrée en (@p x, @p y, @p z). Équivalent à :
 * @~
@code
sphere x, y, z, d, d, d, 25, 25
@endcode
 */
sphere (x:real, y:real, z:real, d:real);

/**
 * @~english
 * Draws a sphere.
 *
 * Sphere with diameter @p d located at (0, 0, 0).
 * Equivalent to:
 *
 * @~french
 * Affiche une sphère.
 *
 * Sphère de diamètre @p d centrée en (0, 0, 0). Équivalent à :
 * @~
@code
sphere 0, 0, 0, d
@endcode
 */
sphere (d:real);

/**
 * @~english
 * Draws a torus.
 *
 * The torus is divided in @p slices and @p stacks. The higher the value of
 * these parametres are, the smoother the torus is (and longer the drawing is).
 * The minor radius of the torus is a ratio @p r of its basis. @p r is a real between 0.0 and 1.0.
 * If @p r is 0 the drawing is a sort of opened cylinder, if @p r is 1 the torus is closed.
 *
 * @~french
 * Affiche un tore.
 *
 * Le tore est divisé en @p slices tranches horizontales et @p stacks
 * tranches verticales. Plus ces paramètres ont une valeur élevée et plus le
 * tore apparait lisse (mais plus il demande du temps pour s'afficher).
 * Le petit diamètre du tore est un rapport @p r de son grand diamètre.
 * @p r est compris entre 0.0 et 1.0.
 * @~
 * @image html torus.png
 */
torus (x:real, y:real, z:real, w:real, h:real, d:real, slices:integer, stacks:integer, r:real);

/**
 * @~english
 * Draws a torus.
 *
 * Torus with diameter @p d, inner radius ratio @p r, located at (@p x, @p y, @p z).
 * Equivalent to:
 *
 * @~french
 * Affiche un tore.
 *
 * Tore de diamètre @p d, de petit diamètre (@p d * @p r), centré en
 * (@p x, @p y, @p z). Équivalent à :
 * @~
@code
torus x, y, z, d, d, d, 25, 25, r
@endcode
 */
torus (x:real, y:real, z:real, d:real, r:real);

/**
 * @~english
 * Draws a cone.
 *
 * It draws a cone with a fixed number of polygon.
 * @~french
 * Affiche un cône.
 * @~
 * @image html cone.png
 */
cone (x:real, y:real, z:real, w:real, h:real, d:real);

/**
 * @~english
 * Draws a truncated cone.
 *
 * The diameter of the top of the cone is a ratio @p r of its basis. @p r is a real between 0.0 and 1.0.
 * If @p r is 0 the drawing is a cone, if @p r is 1 the drawing is a cylinder.
 *
 * @~french
 * Affiche un cône tronqué.
 *
 * Le diamètre en haut du cône est dans un rapport @p r de sa base. @p r est un
 * réel compris entre 0.0 et 1.0. Si @p r vaut 0.0 on obtient un cône complet,
 * si @p r vaut 1.0 on obtient un cylindre.
 * @~
 * @image html truncated_cone.png
 */
truncated_cone (x:real, y:real, z:real, w:real, h:real, d:real, r:real);

/**
 * @~english
 * Draws a cube.
 *
 * It draws a right parallelepiped, and in a particular case a cube.
 *
 * @~french
 * Affiche un parallélépipède.
 * On obtient un cube lorsque @p w, @p h et @p d sont égaux.
 * @~
 * @image html right_parallelepiped.png
 */
cube (x:real, y:real, z:real, w:real, h:real, d:real);

/**
 * @~english
 * Draws a cylinder.
 *
 * It draws the "skin" of a cylinder.
 *
 * @~french
 * Affiche un cylindre creux.
 * @~
 * @image html cylinder.png
 */
cylinder (x:real, y:real, z:real, w:real, h:real, d:real);


/** @} */
