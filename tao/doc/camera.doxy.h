/**
 * @~english
 * @addtogroup Camera Camera
 * @ingroup Graphics
 *
 * Controls the position, orientation and characteristics of the camera.
 *
 * The camera primitives allow to query and modify the point of view, and thus
 * the way the document appears. The parameters that have an influence on the
 * view are:
 *  - the location of the viewpoint (see @ref camera_position)
 *  - the aiming point (see @ref camera_target). Along with the
 *    location of the viewpoint, the aiming point defines the line of sight.
 *  - the "up vector" of the camera (see @ref camera_up_vector). This vector
 *    defines which direction is up (that is, the direction from the bottom to
 *    the top of the view). By default, the up vector of the camera is along
 *    the @p y axis of the scene.
 * @note When a stereoscopic display mode is active, the viewpoint location
 * and the aiming point are adjusted by the amount of the eye separation.
 * Tao Presentation implements an <em>asymmetric frustum parallel axis</em>
 * projection method, which allows the creation of comfortable 3D content
 * that minimize eye strain for the viewer.
 * Cameras (eyes) lie on a straight line, not on an arc, and are spaced evenly
 * by the @ref eye_distance distance. The center of the camera segment is the
 * @ref camera_position. Cameras are globally aimed at the @ref camera_target
 * point, but they are parallel: there is no inward rotation or vergence point.
 *  - the zoom factor. Controls the "focal length" of the camera. Change the
 *    zoom to make objects appear bigger or smaller. See @ref zoom.
 *
 * @~french
 * @addtogroup Camera Caméra
 * @ingroup Graphics
 *
 * Contrôle la position, l'orientation et les caractéristiques de la camera.
 *
 * Les primitives de contrôle de la caméra vous permettent d'interroger et de
 * modifier le point de vue, et donc la manière donc le document est affiché.
 * Les paramètres qui ont une influence sur le point de vue sont :
 *  - la position de la caméra (cf. @ref camera_position)
 *  - le point visé (cf. @ref camera_target). Avec le point où est placée la
 *    caméra, le point visé définit la direction de visée.
 *  - le <em>up vector</em> de la caméra (cf. @ref camera_up_vector). Ce
 *    vecteur définit quelle est la direction vers le haut par rapport à la
 *    caméra (la direction de bas en haut de la vue). Par défaut, le
 *    <em>up vector</em> est aligné le long de l'axe @p y.
 * @note Lorsqu'un mode d'affichage 3D stéréoscopique est activé, la position
 * de la caméra et le point visé sont automatiquement ajustés et décalés de
 * la valeur de l'espace inter-oculaire. Tao Presentation implémente une
 * méthode de projection qui est appelée <em>asymmetric frustum parallel
 * axis</em> (projection à axes parallèles et cône de vision asymétrique).
 * Cette méthode permet la création de contenu 3D agréable à regarder, et qui
 * minimise la fatigue occulaire.
 * Les caméras (qui représentent les yeux du spectateur) sont positionnées sur
 * une ligne droite, non sur un arc de cercle, et sont espacées d'une distance
 * égale à @ref eye_distance. Le milieu du segment constitué par les caméras
 * est @ref camera_position. Les caméras sont globalement dirigées vers le
 * point @ref camera_target, mais elles sont parallèles : il n'y a pas de
 * rotation vers l'intérieur (point de convergence).
 *  - le facteur de zoom. Ce paramètre est relié à la distance focale de la
 * caméra virtuelle. Changer le zoom fait apparaître les objets plus petits
 * ou plus gros. Cf. @ref zoom.
 * @{
 */

/**
 * @~english
 * Returns the current camera position.
 * The default position is (0, 0, 6000).
 * To read one component you may write:
 * @~french
 * Renvoit la position actuelle de la caméra.
 * La position par défaut est (0, 0, 6000).
 * Pour lire seulement une composante vous pouvez écrire :
 * @~
 * @code
cam_x -> camera_position at 1
cam_y -> camera_position at 2
cam_z -> camera_position at 3
 * @endcode
 * @~english
 * @return 3 valeurs réelles, séparées par l'opérateur virgule.
 * @~french
 * @return 3 real values, separated by commas.
 */
infix camera_position();

/**
 * @~english
 * Sets the current camera position.
 * @~french
 * Définit la position actuelle de la caméra.
 */
camera_position(x:real, y:real, z:real);

/**
 * @~english
 * Returns the point the camera is aiming at.
 * The default value is the origin, (0, 0, 0).
 * @return 3 real values, separated by commas.
 * @~french
 * Renvoit le point visé par la caméra.
 * La position par défaut est l'origine, (0, 0, 0).
 * @return 3 valeurs réelles, séparées par l'opérateur virgule.
 */
infix camera_target();

/**
 * @~english
 * Sets the point the camera is aiming at.
 * @~french
 * Définit le point visé par la caméra.
 */
camera_target(x:real, y:real, z:real);

/**
 * @~english
 * Returns the camera "up" direction.
 * The default value is (0, 1, 0), so that the y axis appears vertical.
 * @return 3 real values, separated by commas.
 * @~french
 * Renvoit la direction verticale par rapport à la caméra.
 * La valeur par défaut est (0, 1, 0), de sorte que la verticale est le long
 * de l'axe y.
 * @return 3 valeurs réelles, séparées par l'opérateur virgule.
 */
infix camera_up_vector();

/**
 * @~english
 * Sets the camera "up" direction.
 * @~french
 * Définit la direction verticale par rapport à la caméra.
 */
camera_up_vector(x:real, y:real, z:real);

/**
 * @~english
 * Returns the current zoom factor.
 * The default value is 1.0, or 100%. By default, the "focal length" of the
 * camera is setup so that one pixel at the origin corresponds to one pixel in
 * the output image.
 * @~french
 * Renvoit le facteur de zoom actuel.
 * La valeur par défaut est 1.0, ou 100%. La distance focale de la caméra est
 * telle qu'un pixel à l'origine correspond à un pixel à l'écran.
 */
real zoom();

/**
 * @~english
 * Sets the current zoom factor.
 * Higher values result in a higher magnification.
 * @~french
 * Définit le facteur de zoom.
 * Augmenter la valeur fait apparaître les objets plus gros.
 */
zoom(real z);

/**
 * @~english
 * Sets the distance from the viewer to the near clipping plane (zNear).
 * The zNear value is always positive and non-zero.
 * @~french
 * Définit la distance entre l'observateur et le plan de coupe proche (zNear).
 * La valeur zNear est toujours strictement positive.
 */
z_near(zn:real);

/**
 * @~english
 * Returns the distance from the viewer to the near clipping plane (zNear).
 * The zNear value is always positive and non-zero. The default value
 * is 1500.0.
 * @~french
 * Renvoit la distance entre l'observateur et le plan de coupe proche (zNear).
 * La valeur zNear est toujours strictement positive. La valeur par défaut
 * est 1500.0.
 */
z_near();

/**
 * @~english
 * Sets the distance from the viewer to the far clipping plane (zFar).
 * The zFar value is always positive and non-zero.
 * @~french
 * Définit la distance entre l'observateur et le plan de coupe lointain
 * (zFar).
 * La valeur zFar est toujours strictement positive.
 */
z_far(zf:real);

/**
 * @~english
 * Returns the distance from the viewer to the far clipping plane (zFar).
 * The zFar value is always positive and non-zero. The default value is
 * 1,000,000.0.
 * @~french
 * Renvoit la distance entre l'observateur et le plan de coupe lointain
 * (zFar).
 * La valeur zFar est toujours strictement positive. La valeur par défaut
 * est 1000000.0.
 */
z_far();

/**
 * @~english
 * Convert a screen position to world coordinates.
 * Transform a screen position defined by (@p x, @p y) to world
 * coordinates @p wx, @p wy and @p wz. The depth at the given location
 * is given by the current value of the OpenGL depth buffer.
 * The parameters @p wx, @p wy and @p wz are modified.
 * To simply obtain the depth, use the @ref depth_at function.
 *
 * @return a real number corresponding to the Z position
 * @~french
 * Convertit des coordonnées écran en coordonnées 3D.
 * Transforme une position sur l'écran (@p x, @p y) en coordonnées
 * dans l'espace 3D (@p wx, @p wy, @p wz). La profondeur à l'endroit
 * spécifié est donnée par la valeur du <em>depth buffer</em> OpenGL.
 * Les paramètres  @p wx, @p wy et @p wz sont modifiés.
 * Pour obtenir seulement la profondeur, utilisez la fonction
 * @ref depth_at.
 *
 * @return un nombre réel correspondant à la position en Z
 */
real world_coordinates(x:real, y:real, wx:real, wy:real, wz:real);

/**
 * @~english
 * Get the Z position corresponding to the given coordinates @p x and @p y.
 * The Z position is computed from the OpenGL depth buffer.
 * To obtain the three coordinates, use @ref world_coordinates.
 *
 * @return the Z position at the given coordinates
 * @~french
 * Renvoit la position Z correspondant aux coordonnées écran (@p x, @p y).
 * La position Z est calculée d'après le <em>depth buffer</em> OpenGL.
 * Pour avoir les trois coordonnées 3D, utilisez @ref world_coordinates.
 *
 * @return la profondeur Z aux coordonnées spécifiées
 */
real depth_at(x:real, y:real);

/**
 * @~english
 * Configure the camera as in early versions of Tao Presentations
 * (up to 1.0-beta11).
 * - Camera position is (0, 0, 6000) [now (0, 0, 3000)]
 * - Camera target is (0, 0, 0) [unchanged]
 * - Camera up vector is (0, 1, 0) [unchanged]
 * - zNear is 1000 [now 1500]
 * - zFar is 56000 [now 1e6]
 * - Default eye separation for stereoscopy is 10 [now 100]
 * - Polygon offset settings are:
 * @code
polygon_offset 0, -0.001, 0, -1
 * @endcode
 * [now 0, -0.1, 0, -1]
 *
 * @~french
 * Configure la caméra comme dans les versions anciennes de Tao Presentations
 * (jusqu'à la version 1.0-beta11).
 * - La position de la caméra est (0, 0, 6000) [maintenant : (0, 0, 3000)]
 * - Le point visé est (0, 0, 0) [inchangé]
 * - Le <em>up vector</em> est (0, 1, 0) [inchangé]
 * - zNear est 1000 [maintenant : 1500]
 * - zFar est 56000 [maintenant : 1e6]
 * - La distance entre les yeux pour la stéréoscopie est 10 [maintenant : 100]
 * - Les réglages de <em>polygon offset</em> sont :
 * @code
polygon_offset 0, -0.001, 0, -1
 * @endcode
 * [maintenant : 0, -0.1, 0, -1]
 */
old_camera_settings();

/**
 * @}
 */
