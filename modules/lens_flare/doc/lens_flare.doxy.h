/**
 * @~english
 * @taomoduledescription{LensFlare, Lens Flare}
 *
 * <tt>import LensFlare</tt> - Generate a lens flare.
 *
 * This module allows to create a dynamic lens flare, either using default model,
 * or adding new flares thanks to specified textures.
 *
 * A lens flare is an optical phenomenon that occurs when light reflects inside the lens of the camera.
 * The light bounces off of the camera lens and components inside of the lens and
 * camera to create streaks, haze, and other effects.
 *
 * Typically a lens flare is something to avoid, however some types of flare may actually enhance
 * the artistic meaning of a photo when it is used on purpose.
 *
 * It can help to add realism to the scene, or make an otherwise boring photo
 *  stand out with the artful use of light.
 *
 *
 * For instance, the following code defines the same model of a lens flare created by the sun,
 * than the built-in form @ref sun_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real).
 * @include lens_flare.ddd
 *
 * @image html lens_flare.png "Lens flare effect"
 *
 *
 * @note In order to use correctly the depth test, the lens flare have to be drawn after the others elements.
 * @note This module integrate already some lens flare textures, which can be used to create differents effects like in the previous example.
 * @note It is also possible to create these textures in Tao thanks to integrated fonctions (@p frame_texture, @p radial_gradient, etc.).
 *
 * @endtaomoduledescription{LensFlare}
 *
 * @~french
 * @taomoduledescription{LensFlare, Halo}
 *
 * <tt>import LensFlare</tt> - Génère un effet de halo.
 *
 * Ce module permet de créer des effets dynamiques de halo
 * (<em>lens flare</em>), en utilisant les images par défaut, ou bien en
 * utilisant des images fournies.
 *
 * Un halo se produit lorsque la lumière se réfléchit à l'intérieur de
 * l'objectif de l'appareil photo ou de la caméra. Cela crée des traînées, du
 * flou, et d'autres défauts.
 *
 * En principe, on essaie d'éviter l'effet de halo, mais il peut aussi
 * améliorer le côté artistique d'une image lorsqu'il est utilisé
 * volontairement.
 *
 * Cet effet peut aider à ajouter du réalisme à une scène, voire transformer
 * une image quelconque en image intéressante.
 *
 * Par exemple, le code qui suit définit un effet de halo provoqué par le
 * soleil, similaire à l'effet produit par la forme @a builtin
 * @ref sun_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real).
 * @include lens_flare.ddd
 *
 * @image html lens_flare.png "Effet de halos"
 *
 *
 * @note Afin de s'afficher correctement, l'effet de halo a besoin de connaître
 *       tous les éléments de la scène. Par conséquent il convient de
 *       l'utiliser @a après les autres éléments.
 * @note Ce module fournit des textures de reflet de lumière, qui peuvent être
 *       utilisées individuellement comme dans l'exemple ci-dessus. Mais vous
 *       pouvez charger vos propres textures si les images par défaut ne vous
 *       conviennent pas.
 * @note Il est également possible d'utiliser l'effet halo avec des textures
 *       générées dynamiquement par Tao Presentations (cf. @p frame_texture,
 *       @p radial_gradient, etc.).
 *
 * @endtaomoduledescription{LensFlare}
 * @~
 * @{
 */

/**
 * @~english
 * Directory of lens flare textures.
 *
 * This folder contains some images, which
 * can be used to generate differents type of lens flares.
 *
 * Its default value is the images folder in the lens flare module.
 *
 * @~french
 * Le répertoire où se trouvent les textures de halos.
 *
 * Ce répertoire contient des images qui peuvent être utilisées pour faire
 * différentes sortes de halos. Sa valeur par défaut est le répertoire
 * "images" du module.
 */
text FLARES_DIR;

/**
 * @~english
 * Creates a local lens flare.
 *
 * Define and evaluate the lens flare tree while
 * preserving the current graphical state like a @p locally.
 *
 * @param contents defines code applied on the current lens flare.
 *
 * @~french
 * Crée un effet de halo.
 * Le code contenu dans @p contents est évalué en préservant les
 * trasformations géométriques (comme @p locally).
 * Utilisez @ref lens_flare_source, @ref lens_flare_target
 * et @ref add_flare pour construire l'effet.
 */
lens_flare(contents:tree);

/**
 * @~english
 * Enable or disable the depth test for the lens flare.
 * A depth test is performed to determine if the main light source of the
 * lens flare effet is visible (that is, not hidden by another object).
 * If the source is visible, the lens flares are blended on top of the
 * scene. Otherwise no flares are added. This adds realism when a moving
 * object temporarily hides the light source. However, if the source
 * is never hidden the depth test becomes useless. In such a case, it might
 * be useful to disable the test, in order to increase performance on some
 * graphic cards.
 *
 * @param mode is @c ON or @c OFF. Its default value is ON.
 *
 * @~french
 * Active ou désactive le test de visibilité de la source lumineuse.
 * Afin de générer un effet réaliste, les halos lumineux ne sont
 * superposés à la scène que si le centre de la source lumineuse principale
 * est visible (si elle n'est pas cachée par un objet plus proche).
 * Ce test peut s'avérer coûteux en performances sur
 * certaines cartes graphiques. Lorsque la source lumineuse est toujours
 * visible, le test devient inutile, et il peut alors être judicieux de
 * désactiver ce test. Pour cela, il suffit de mettre le paramètre @p mode
 * à OFF.
 * La valeur par défaut est @a ON.
 */
lens_flare_depth_test(mode:name);

/**
 * @~english
 * Define lens flare source.
 * Set the target of the lens source at (@p x, @p y, @p z).
 * @~french
 * Définit la position de la source de la lumière qui provoque les halos.
 */
lens_flare_source(x:real, y:real, z:real);

/**
 * @~english
 * Define lens flare target.
 * Set the target of the lens flare at (@p x, @p y, @p z).
 * @~french
 * Définit la direction de l'axe qui porte les reflets.
 * (@p x, @p y, @p z) est la position du point qui définit l'extrémité du
 * segment portant les images des reflets, l'autre extrémité étant la source.
 */
lens_flare_target(x:real, y:real, z:real);

/**
 * @~english
 * Add a new flare to the current lens flare.
 * @param id index of the flare texture.
 * @param loc relative location of the flare. 0.0 is the source position and 1.0 is the target one.
 * @param scale scaling rate of the texture. 1.0 is its normal size.
 * @param r red component of the flare color.
 * @param g green component of the flare color.
 * @param b blue component of the flare color.
 * @param a alpha-channel, transparency of the flare color. 0.0 is transparent and 1.0 is opaque.
 *
 * @note Flares, which are located on the source, are always drawn.
 * @~french
 * Ajoute un nouveau reflet à l'effet de halos.
 * Positionne une texture sur le chemin de l'effet de halos. Il est possible de
 * jouer sur la coloration de l'image ajoutée grâce à la couleur et à la
 * transparence du filtre : (@p r, @p g, @p bm @p a).
 * @param id Index de la texture à ajouter.
 * @param loc Position relative du reflet sur un segment, 0.0 est la source
 *        (@ref lens_flare_source) et 1.0 est la cible.
 * @param scale Permet de redimensionner la texture. 1.0 pour conserver sa taille.
 * @param r Composante rouge de la couleur du filtre.
 * @param g Composante verte de la couleur du filtre.
 * @param b Composante bleue de la couleur du filtre.
 * @param a Transparence de la couleur du filtre.
 */
add_flare(id:integer, loc:real, scale:real, r:real, g:real, b:real, a:real);

/**
 * @~english
 * Create a specific lens flare due to the sun.
 * (@p sx, @p sy, @p sz) is the position of the light source.
 * The target of the lens flare is located at (@p tx, @p ty, @p tz).
 * @~french
 * Crée un effet de halos dû au soleil.
 * (@p sx, @p sy, @p sz) est la position du soleil, (@p tx, @p ty, @p tz)
 * définit la direction de l'effet.
 */
sun_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real);

/**
 * @~english
 * Create a simple white lens flare.
 * (@p sx, @p sy, @p sz) is the position of the light source.
 * The target of the lens flare is located at (@p tx, @p ty, @p tz).
 * @~french
 * Crée un effet de halos dû à une lumière blanche.
 * (@p sx, @p sy, @p sz) est la position de la source lumineuse,
 * (@p tx, @p ty, @p tz) définit la direction de l'effet.
 */
white_flare(sx:real, sy:real, sz:real, tx:real, ty:real, tz:real);
/**
* @}
*/
