/**
 * @~english
 * @addtogroup Lighting
 * @ingroup Graphics
 *
 * Controls the lighting of 3D scenes.
 *
 * This page describes all the builtin primitives that are related to lighting.
 * The primitives are mostly direct mappings of the OpenGL calls. Please
 * refer to the OpenGL documentation of
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glLight.xml">glLight</a>
 * and
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glMaterial.xml">glMaterial</a>
 * for details.
 *
 * @par Enabling lighting
 *
 * Lighting calculations do not occur by default. They are enabled as
 * lights are declared and only within the current and child layouts
 * (layouts are created with @ref code.doxy.h::locally "locally" or @ref graph.doxy.h::shape "shape").
 * Therefore, the following code will
 * draw one orange rectangle and one yellow rectangle, as expected:
 @code
locally
    // Red light (diffuse)
    light 0
    light_diffuse 1, 0, 0, 1
    light_position 200, 200, 200
    // Yellow material
    material_ambient 1, 1, 0, 1
    material_diffuse 1, 1, 0, 1
    rectangle -100, 0, 150, 100

// Regular color model, no lighting
color "yellow"
rectangle 100, 0, 150, 100
 @endcode
 *
 * Without the <tt>locally</tt> call (for instance, if you replace
 * <tt>'locally'</tt> with <tt>'if true then'</tt>),
 * the program shows two orange rectangles. This is because
 * lighting is still active when the second rectangle is drawn, contrary
 * to what occurs when the light is enclosed in a @c locally block. The
 * yellow rectangle being lit with a red light, it appears orange.
 *
 * @par Colors and materials
 *
 * When lighting is enabled, changing the current color changes the
 * "front and back, ambient and diffuse" material properties.
 * That is, the following code:
 @code
// Yellow
color 1, 1, 0, 1
 @endcode
 * ...is always equivalent to:
 @code
// Yellow
material_ambient 1, 1, 0, 1
material_diffuse 1, 1, 0, 1
 @endcode
 * ...which is also equivalent to:
 @code
// Yellow
material_back_ambient 1, 1, 0, 1
material_front_ambient 1, 1, 0, 1
material_back_diffuse 1, 1, 0, 1
material_front_diffuse 1, 1, 0, 1
 @endcode
 *
 * @par Example
 *
 * The following code (<a href="examples/lighting.ddd">lighting.ddd</a>) draws
 * a white sphere and creates three colored lights:
 * one red, one green and one blue. Each light rotates around the sphere, so
 * that you can see how lights blend into smooth colors gradients onto the
 * sphere.
 * @include lighting.ddd
 *
 * @image html lighting.png "Lighting demo"
 *
 * @~french
 * @addtogroup Lighting Éclairage
 * @ingroup Graphics
 *
 * Contrôle l'éclairage des scènes 3D.
 *
 * Cette page décrit toutes les primitives relatives à l'éclairage, qui sont
 * essentiellement des adaptations directes des appels OpenGL. Vous pouvez
 * vous référer à la documentation (en anglais) des fonctions OpenGL
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glLight.xml">glLight</a>
 * et
 * <a href="http://www.opengl.org/sdk/docs/man/xhtml/glMaterial.xml">glMaterial</a>
 * pour plus de détails.
 *
 * @par Activer l'éclairage
 *
 * Les calculs d'éclairage ne sont pas activés par défaut. C'est seulement
 * lorsque des lumières sont déclarées dans le document que le @a layout
 * courant (ainsi que ses descendants) sont affectés par la lumière
 * (les primitives qui créent un @a layout sont listées au paragraphe
 * @ref secExecModel).
 * Par conséquent, le code qui suit affiche un rectangle orange et un
 * rectangle jaune :
 @code
locally
    // Lumière rouge (diffuse)
    light 0
    light_diffuse 1, 0, 0, 1
    light_position 200, 200, 200
    // Matière jaune
    material_ambient 1, 1, 0, 1
    material_diffuse 1, 1, 0, 1
    rectangle -100, 0, 150, 100

// Ici, la lumière rouge n'a plus d'effet
color "yellow"
rectangle 100, 0, 150, 100
 @endcode
 *
 * Sans la primitive <tt>locally</tt> (par exemple, si vous remplacez
 * <tt>'locally'</tt> par <tt>'if true then'</tt>),
 * on obtient deux rectangles oranges. En effet, la lumière
 * rouge est encore active lorsque le second rectangle est dessiné, ce qui
 * n'est pas le cas lorsque les primitives de lumière sont dans un bloc
 * @c locally. Le rectangle jaune éclairé par une lumière rouge a
 * logiquement une apparance orange.
 *
 * @par Couleurs et matériaux
 *
 * Lorsqu'au moins une lumière est active, un changement de la couleur
 * courante change la couleur des matériaux <em>front and back, ambient and
 * diffuse</em>. Autrement dit, le code suivant :
 @code
// Jaune
color 1, 1, 0, 1
 @endcode
 * ...est toujours équivalent à :
 @code
// Jaune
material_ambient 1, 1, 0, 1
material_diffuse 1, 1, 0, 1
 @endcode
 * ...ou encore :
 @code
// Jaune
material_back_ambient 1, 1, 0, 1
material_front_ambient 1, 1, 0, 1
material_back_diffuse 1, 1, 0, 1
material_front_diffuse 1, 1, 0, 1
 @endcode
 *
 * @par Example
 *
 * Le code suivant affiche
 * une sphère blanche et crée trois lumières colorées : une rouge, une verte
 * et une bleue. Chaque lumière tourne autour de la sphère, de sorte qu'on
 * peut voir comment les lumières se mélangent progressivement pour créer des
 * dégradés de couleurs sur la sphère.
 * @include lighting.ddd
 *
 * @image html lighting.png "Exemple d'éclairage"
 *
 * @~
 * @{
 */

/**
 * @~english
 * Switch a light on or off and make it the current light.
 * @p id is an integer between 0 and the maximum number of lights minus 1.
 * The number of lights depends on the OpenGL implementation, but at least
 * eight lights are supported.
 * When the primitive returns, the light is the current light for the
 * primitives that control light properties.
 * @~french
 * Allume une lumière et en fait la lumière courante.
 * @p id est un entier entre 0 et le nombre maximum de lumières moins 1.
 * Le nombre de lumières dépend de l'implémentation OpenGL, mais au moins huit
 * lumières sont disponibles. Après exécution de cette primitive, la lumière
 * est considérée comme la lumière courante pour les primitives qui contrôlent
 * les propriétés d'éclairage.
 */
light(id:integer, enable:boolean);


/**
 * @~english
 * Switch a light on and make it the current light.
 * Same as <tt>light(id, true)</tt>.
 * @~french
 * Allume une lumière et en fait la lumière courante.
 * Équivalent à : <tt>light(id, true)</tt>
 */
light(id:integer);

/**
 * @~english
 * Enables or disables per pixel lighting.
 * When per-pixel lighting (PPL) is enabled, the lighting computations are done
 * for each visible pixel of illuminated objects. When disabled, they are done
 * for each vertex of such objects (and then vertex values are linearly
 * interpolated to give per-pixel values). PPL is implemented by a GLSL
 * fragment shader.
 * Consequently:
 * - PPL usually gives better visual result, especially for coarsely divided
 * geometries,
 * - PPL is usually faster than per-vertex lighting when rendering complex
 * models. It may be slower on older graphic cards (which have poor GLSL
 * performances, even on simple geometries).
 *
 * By default, per-pixel lighting is disabled.
 *
 * @note Tao Presentations has a global setting for per-pixel lighting, in the
 * preference dialog. This global setting is used when @c per_pixel_lighting
 * is not used.
 *
 * @~french
 * Active ou désactive l'éclairage par pixel.
 * Lorsque l'éclairage par pixel (<em>per-pixel lighting</em> ou PPL) est
 * activé, les calculs d'illumination sont faits pour chaque pixel visible
 * des objets illuminés. Lorsqu'il est désactivé, les calculs sont faits
 * pour chaque sommet de ces objets (et ensuite les valeurs obtenues aux
 * sommets sont interpolés linéairement pour obtenir des valeurs sur chaque
 * pixel). Le PPL est implémenté par un <em>fragment shader</em> GLSL.
 * Par conséquent :
 * - Le PPL donne en général de meilleurs résultats visuels, en particulier
 * sur des objets 3D faiblement subdivisés,
 * - Le PPL est en général plus rapide lorsqu'on affiche des objets 3D
 * complexes. Il peut être plus lent sur des cartes graphiques anciennes
 * (qui ont des implémentations GLSL peu performantes).
 *
 * Par défaut, l'éclairage per-pixel est désactivé.
 *
 * @note Tao Presentations dispose d'un réglage global pour l'éclairage
 * par pixel, dans la fenêtre de préférences de l'application. Ce réglage
 * global est utilisé en l'absence d'appel à @c per_pixel_lighting.
 */
per_pixel_lighting(enable:boolean);

/**
 * @~english
 * Return a bitmask of current enabled lights.
 * The default value is 0 if there is no current light.
 * @~french
 * Renvoie un masque de bits indiquant les lumières actives.
 * La veleur est 0 lorsqu'aucune lumière n'est active.
 */
integer lights_mask();

/**
 * @~english
 * Sets the color and intensity of the ambient emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_AMBIENT, {r, g, b, a})</tt>
 *
 * The default color is black for all lights.
 *
 * @~french
 * Définit la couleur et l'intensité de la composante ambiante de la lumière
 * courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightfv(id, GL_AMBIENT, {r, g, b, a})</tt>
 *
 * La couleur par défaut est noir pour toutes les lumières.
 */
light_ambient(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the color and intensity of the diffuse emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_DIFFUSE, {r, g, b, a})</tt>
 *
 * The default color is white for light 0 and black for the others.
 *
 * @~french
 * Définit la couleur et l'intensité de la composante diffuse de la lumière
 * courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightfv(id, GL_DIFFUSE, {r, g, b, a})</tt>
 *
 * La couleur par défaut est le blanc pour la lumière numéro 0 et le noir pour
 * toutes les autres.
 */
light_diffuse(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the color and intensity of the specular emission of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_SPECULAR, {r, g, b, a})</tt>
 *
 * The default color is white for light 0 and black for the others.
 *
 * @~french
 * Définit la couleur et l'intensité de la composante spéculaire de la lumière
 * courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightfv(id, GL_SPECULAR, {r, g, b, a})</tt>
 *
 * La couleur par défaut est le blanc pour la lumière numéro 0 et le noir pour
 * toutes les autres.
 */
light_specular(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the position of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, 0})</tt>
 *
 * @~french
 * Définit la position de la lumière courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, 0})</tt>
 */
light_position(x:real, y:real, z:real);


/**
 * @~english
 * Sets the position of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, w})</tt>
 *
 * If @p w is 0.0, the light is treated as a directional source.
 * Diffuse and specular lighting calculations take the light's direction,
 * but not its actual position, into account, and attenuation is disabled.
 * Otherwise, diffuse and specular lighting calculations are based on the
 * actual location of the light, and attenuation is enabled.
 *
 * <tt>w = 0.0</tt> is typically used for a light that is infinitely far
 * away (like the sun), and <tt>w = 1.0</tt> for a light in a fixed position
 * (like a lamp).
 *
 * @~french
 * Définit la position de la lumière courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightfv(id, GL_POSITION, {x, y, z, w})</tt>
 *
 * Si @p w est égal à 0.0 (la valeur par défaut), la lumière est
 * considérée comme une source
 * directionnelle. Les calculs d'illumination diffuse et spéculaire utilisent
 * la direction de la lumière, mais pas sa position réelle, et l'atténuation est
 * désactivée. Sinon, les calculs prennent en compte la position réelle de la
 * lumière et l'atténuation est activée.
 *
 * On utilise typiquement <tt>w = 0.0</tt> pour une lumière "à l'infini" (par
 * exemple, le soleil) et <tt>w = 1.0</tt> pour une lumière proche (par exemple,
 * une lampe).
 */
light_position(x:real, y:real, z:real, w:real);


/**
 * @~english
 * Sets the spot direction of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightfv(id, GL_SPOT_DIRECTION, {x, y, z})</tt>
 *
 * @~french
 * Définit la direction du spot de la lumière courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightfv(id, GL_SPOT_DIRECTION, {x, y, z})</tt>
 */
light_spot_direction(x:real, y:real, z:real);


/**
 * @~english
 * Sets the spot exponent of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightf(id, GL_SPOT_EXPONENT, x)</tt>
 *
 * @~french
 * Définit l'exposant de spot de la lumière courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightf(id, GL_SPOT_EXPONENT, x)</tt>
 */
light_spot_exponent(x:real);

/**
 * @~english
 * Sets the spot cutoff of the current light.
 * Equivalent to the OpenGL call:
 * <tt>glLightf(id, GL_SPOT_CUTOFF, x)</tt>
 *
 * @~french
 * Définit l'angle limite du spot de la lumière courante.
 * Équivalent à l'appel OpenGL :
 * <tt>glLightf(id, GL_SPOT_CUTOFF, x)</tt>
 */
light_spot_cutoff(x:real);

/**
 * @~english
 * Sets the attenuation factors of the current light.
 * Equivalent to the OpenGL calls:
 *
 * @~french
 * Définit les facteurs d'atténuation de la lumière courante.
 * Équivalent aux appels OpenGL :
 * @~
 * @code
 * glLightf(id, GL_CONSTANT_ATTENUATION, c);
 * glLightf(id, GL_LINEAR_ATTENUATION, l);
 * glLightf(id, GL_QUADRATIC_ATTENUATION, q);
 * @endcode
 */
light_attenuation(c:real, l:real, q:real);


/**
 * @~english
 * Sets the ambient parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur ambiante du matériau (deux faces).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 */
material_ambient(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the diffuse parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur diffuse du matériau (deux faces).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 */
material_diffuse(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the specular parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur spéculaire du matériau (deux faces).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 */
material_specular(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the emission parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur d'émission du matériau (deux faces).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, {r, g, b, a})</tt>
 */
material_emission(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the shininess parameter of the current material (two faces).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, x)</tt>
 *
 * @~french
 * Définit la brillance du matériau (deux faces).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, x)</tt>
 */
material_shininess(x:real);


/**
 * @~english
 * Sets the ambient parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_AMBIENT, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur ambiante du matériau (face avant).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT, GL_AMBIENT, {r, g, b, a})</tt>
 */
material_front_ambient(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the diffuse parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_DIFFUSE, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur diffuse du matériau (face avant).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT, GL_DIFFUSE, {r, g, b, a})</tt>
 */
material_front_diffuse(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the specular parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_SPECULAR, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur spéculaire du matériau (face avant).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT, GL_SPECULAR, {r, g, b, a})</tt>
 */
material_front_specular(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the emission parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_FRONT, GL_EMISSION, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur d'émission du matériau (face avant).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_FRONT, GL_EMISSION, {r, g, b, a})</tt>
 */
material_front_emission(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the shininess parameter of the current material (front face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_FRONT, GL_SHININESS, x)</tt>
 *
 * @~french
 * Définit la brillance du matériau (face avant).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialf(GL_FRONT, GL_SHININESS, x)</tt>
 */
material_front_shininess(x:real);


/**
 * @~english
 * Sets the ambient parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur ambiante du matériau (face arrière).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_BACK, GL_AMBIENT, {r, g, b, a})</tt>
 */
material_back_ambient(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the diffuse parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur diffuse du matériau (face arrière).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_BACK, GL_DIFFUSE, {r, g, b, a})</tt>
 */
material_back_diffuse(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the specular parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur spéculaire du matériau (face arrière).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_BACK, GL_SPECULAR, {r, g, b, a})</tt>
 */
material_back_specular(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the emission parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialfv(GL_BACK, GL_EMISSION, {r, g, b, a})</tt>
 *
 * @~french
 * Définit la composante couleur d'émission du matériau (face arrière).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialfv(GL_BACK, GL_EMISSION, {r, g, b, a})</tt>
 */
material_back_emission(r:real, g:real, b:real, a:real);


/**
 * @~english
 * Sets the shininess parameter of the current material (back face).
 * Equivalent to the OpenGL call:
 * <tt>glMaterialf(GL_BACK, GL_SHININESS, x)</tt>
 *
 * @~french
 * Définit la brillance du matériau (face arrière).
 * Équivalent à l'appel OpenGL :
 * <tt>glMaterialf(GL_BACK, GL_SHININESS, x)</tt>
 */
material_back_shininess(x:real);


/**
 * @}
 */
