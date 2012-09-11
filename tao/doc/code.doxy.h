/**
 * @~english
 * @addtogroup code
 * @ingroup TaoBuiltins
 * This group deals with some helper functionnalities.
 * Some of the following functions are postfix ones and are to be used like units :
 * - mm
 * - cm
 * - px
 * - pt
 * - inch
 *
 * If you want to specify a translation (which takes pixels) along X-axis of 2 inches, just use
 * @code translatex 2 inch @endcode
 * @note The size on the screen may not be exactly the one requested, but it is when printing.
 *
 * @~french
 * @addtogroup code Divers
 * @ingroup TaoBuiltins
 * Ce groupe contient des fonctions d'aide.
 * Certaines fonctions sont des postfix, et peuvent être utiisées comme des unités :
 * - mm
 * - cm
 * - px
 * - pt
 * - inch
 *
 * Si vous voulez faire une translation (qui demande des pixels) le long de
 * l'axe X, de 2 cm, écrivez simplement :
 * @code translatex 2 cm @endcode
 * @note La distance à l'écran peut ne pas être exacte (en particulier en cas de
 * projection), mais elle est correcte lors de l'impression sur papier.
 *
 * @~
 * @{
 */
/**
 * @~english
 * Converts from millimeters to pixels.
 * @p x [real] is the value in mm.
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des millimètres en pixels.
 * @p x [réel] est la valeur en millimètres.
 * @returns La valeur en pixels.
 */
x ( mm);

/**
 * @~english
 * Converts from centimeters to pixels.
 * @p x [real] is the value in cm
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des centimètres en pixels.
 * @p x [réel] est la valeur en centimètres.
 * @returns La valeur en pixels.
 */
x ( cm);

/**
 * @~english
 * Converts from pixels to pixels.
 *
 * This function simply returns a copy of @p x.
 * @p x [real] is the value in pixel
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des pixels en pixels.
 *
 * Cette fonction renvoie simplement un copie de @p x.
 * @p x [réel] est la valeur en pixels
 * @returns La valeur en pixels.
 */
x ( px);

/**
 * @~english
 * Convert from points (pt) to pixels.
 *
 * @p x is a value in PostScript points (1/72 inch).
 *
 * @p x [real] is the value in point
 * @returns The value in pixel. 
 *
 * @~french
 * Convertit des points PostScript (pt) en pixels.
 *
 * @p x est une valeur en points PostScript (1/72 de pouce).
 *
 * @p x [réel] est la valeur en points.
 * @returns La valeur en pixels.
 */
x ( pt);

/**
 * @~english
 * Converts from inches to pixels.
 * @p x [real] is the value in inches.
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des pouces en pixels.
 * @p x [réel] est la valeur en pouces.
 * @returns La valeur en pixels.
 */
x ( inch);


/**
 * @~english
 * Marks the tree that it cannot be modified afterward.
 *
 * @param t [tree] the tree that cannot be modify.
 * @returns the tree itself.
 *
 * @~french
 * Empêche la modification d'un arbre.
 *
 * @param t [tree] L'arbre qui ne doit pas être modifiable.
 * @returns L'arbre, constant.
 */
constant (t:tree);

/**
 * @~english
 * Sets the refresh time for the current layout.
 *
 * The current layout (that is, the enclosing @ref locally or @ref shape) will
 * be re-executed after @p interval seconds.
 *
 * @~french
 * Définit un délai de rafraîchissement du layout courant.
 *
 * Le @a layout courant (c'est-à-dire, la commande @ref locally ou @ref shape
 * qui contient le @ref refresh) sera ré-exécuté dans @p interval
 * secondes.
 * @~
 * @see refresh_on, no_refresh_on
 */
refresh (interval:real);


/**
 * @~english
 * Request refresh of the current layout on a specific event.
 *
 * The current layout (that is, the enclosing @ref locally or @ref shape) will
 * be re-executed whenever an event of type @p event is processed. @n
 * Possible values for @p event are:
 * - @c TimerEvent
 * - @c MouseButtonPressEvent
 * - @c MouseButtonReleaseEvent
 * - @c MouseButtonDblClickEvent
 * - @c MouseMoveEvent
 * - @c KeyPressEvent
 * - @c KeyReleaseEvent
 *
 * For instance, the following code counts the number of clicks:
 *
 * @~french
 * Programme un rafraîchissement du layout courant sur un événement donné.
 *
 * Le @a layout courant (c'est-à-dire, la commande @ref locally ou @ref shape
 * qui contient le @ref refresh) sera ré-exécuté lorsqu'un événement de type
 * @p event sera reçu. @n
 * Les valeurs admissibles pour @p event sont:
 * - @c TimerEvent
 * - @c MouseButtonPressEvent
 * - @c MouseButtonReleaseEvent
 * - @c MouseButtonDblClickEvent
 * - @c MouseMoveEvent
 * - @c KeyPressEvent
 * - @c KeyReleaseEvent
 *
 * Par exemple, le code suivant compte le nombre de clics:
 * @~
 * @code
N -> -1
locally
    N := N+1
    text text N
    refresh_on MouseButtonPressEvent
 * @endcode
 * @see refresh, no_refresh_on
 */
refresh_on (event:integer);


/**
 * @~english
 * Cancels refresh of the current layout on a specific event.
 *
 * This primitive removes the specified event type from the list of events that
 * will trigger a refresh of the current layout. @n
 * For example, the following code will display the mouse coordinates every
 * two seconds (an not whenever a mouse event is received).
 *
 * @~french
 * Annule le rafraîchissement du layout courant sur un événement donné.
 *
 * Cette primitive supprime le type d'événement spécifié de la liste des
 * événements qui provoqueront un rafraîchissement du @a layout courant.
 * Par exemple, le code suivant affiche la position de la souris toutes les
 * deux secondes environ (et non à chaque fois qu'un événement souris est
 * reçu).
 *
 * @~
 * @code
locally
    refresh 2.0
    text text screen_mouse_x & ", " & text screen_mouse_y
    no_refresh_on MouseMoveEvent
 * @endcode
 * @see refresh_on, refresh
 */
no_refresh_on (event:integer);


#ifndef DOXYGEN
// I keep this undocumented for the time being for several reasons:
//
// 1) I fail to find reasonable use cases to illustrate the primitives
// 2) post_event has an unexpected behavior when used in a layout that
//    depends on time (fast refresh), like:
//      locally { time ; post_event user_event "MyEvent" }
//      locally { circle mouse_x, mouse_y, 10 }
//    This code will make Tao enter a tight loop, sending and processing
//    user event, because a time-dependant layout with unspecified next
//    refresh always gets executed on any event (including user events).
//    See Layout::NeedRefresh(). Then the mouse-dependant layout rarely gets
//    executed.
// 3) There is a one frame delay between 'post_event U' and the actual refresh
//    of the layouts that call 'refresh_on U'.
//
// In fact I have exported these primitives essentially for testing purposes
// (see #2128), and I have considered they implement #1271. But there may be
// some more work needed.

/**
 * @~english
 * Returns an event type (identifier) for the specified name.
 *
 * Allocates an event identifier in the user-defined range and associates it to
 * @p name. A given name always returns the same event identifier. @n
 * The returned value may be used as a paramter to @ref refresh_on,
 * @ref no_refresh_on and @ref post_event.
 *
 * @~french
 * Renvoie un identifiant d'événement pour le nom donné.
 *
 * Cette primitive sélectionne un identifiant dans l'intervalle des événements
 * utilisateur, et l'associe à @p name. Pour une valeur donnée de @p name, c'est
 * toujours le même identifiant d'événement qui est renvoyé. @n
 * La valeur donnée par cette fonction peut être utilisée avec @ref refresh_on,
 * @ref no_refresh_on et @ref post_event.
 *
 * @~
 * @see refresh_on, no_refresh_on, post_event
 */
user_event (name:text);


/**
 * @~english
 * Inserts a new event in the event queue.
 *
 * An event with the specified event type is created and posted into the widget's
 * event queue. For example:
 *
 * @~french
 * Insère un événement dans la queue d'événements.
 *
 * Un événement du type spécifié est créé et inséré dans la queue d'événements
 * du @a widget graphique. Par exemple :
 *
 * @~
 * @code
locally
    X -> 0
    X := 0
    refresh_on user_event "Reset"
    locally
        refresh 1.0
        X := X + 1
        text text X

locally
    refresh_on MouseButtonPressEvent
    post_event user_event "Reset"
 * @endcode
 * @see refresh_on, no_refresh_on, post_event
 */
post_event (type:integer);

#endif

/**
 * @~english
 * Exits the Tao application.
 *
 * @p n is the exit status of the process.
 *
 * @~french
 * Termine l'application Tao Presentations.
 *
 * @p n est le code de retour du processus.
 *
 * @~
 * @bug #771 No application cleanup when exit primitive is called.
 */
exit (n:integer);

/**
 * @~english
 * Returns true if the current document is stored in a repository.
 *
 * Returns true if the current document is stored in a repository,
 * Git functionality is compiled in, and not disabled on the command line.
 * @~french
 * Renvoie vrai si le document appartient à un dépôt Git.
 *
 * Renvoie vrai si le document est enregistré dans un dépôt Git,
 * si la fonctionalité Git est incluse dans l'application et n'a pas été
 * désactivée au démarrage de Tao Presentations.
 */
current_repository ();


/**
 * @~english
 * Checks out a version of the current document.
 *
 * Checkout a branch or a commit from the git repository, if available.
 * @p t must be a valid version identifier: branch name, tag name or commit ID.
 * @see current_repository.
 * @returns True if the checkout succeded, false otherwise.
 *
 * @~french
 * Extrait une version du document.
 *
 * Extrait une branche ou un commit du dépôt Git.
 * @p t doit être un identifiant de version valide : nom de branche, de tag ou
 * identifiant de commit.
 * @see current_repository.
 * @returns Vrai si le @a checkout a réussi, faux sinon.
 */
boolean checkout (t:text);


/**
 * @~english
 * Checks if a Tao feature is available with the current Tao version.
 *
 * @p name is the name of the feature to test. Valid feature names are:
 *   - @b git document versioning with Git
 *
 * @returns True if the feature is compiled in, false otherwise.
 *
 * @~french
 * Vérifie si une fonctionalité Tao est disponible.
 *
 * @p name est le nom de la fonctionalité à tester. Les noms valides sont :
 *   - @b git La gestion de l'historique du document
 *
 * @returns Vrai si la fonctionalité est disponible, faux sinon.
 */
boolean is_available (name:text);

/**
 * @~english
 * Get current OpenGL version.
 *
 * @returns Name of the current OpenGL version.
 *
 * @~french
 * Renvoie la version d'OpenGL.
 *
 * @returns La version d'OpenGL.
 */
text GL_version();

/**
 * @~english
 * Checks if an OpenGL extension is available in Tao.
 *
 * @p name is the name of the OpenGL extension to test.
 *
 * @returns True if the extension is available, false otherwise.
 *
 * @~french
 * Vérifie si une extension OpenGL est disponible.
 *
 * @p name est le nom de l'estension à tester.
 *
 * @returns Vrai si l'extension est disponible, faux sinon.
 */
boolean is_ext_available (name:text);

/**
 * @~english
 * Checks if we are currently rendering to file.
 * @~french
 * Teste si le rendu fichier est actif.
 */
boolean offline_rendering ();

/**
 * @~english
 * Disables the OpenGL depth test.
 * Equivalent to the OpenGL call: <tt>glDisable(GL_DEPTH_TEST)</tt>
 * The state of the depth test is automatically restored when leaving the
 * current layout.
 * @returns The previous state of the depth test.
 * @~french
 * Désactive le test de profondeur OpenGL.
 * Équivalent à l'appel OpenGL : <tt>glDisable(GL_DEPTH_TEST)</tt>
 * L'état précédent du test de profondeur est automatiquement rétabli lors
 * de la sortie du layout.
 * @returns L'état précédent du test de profondeur.
 */
disable_depth_test ();

/**
 * @~english
 * Enables the OpenGL depth test.
 * Equivalent to the OpenGL call: <tt>glEnable(GL_DEPTH_TEST)</tt>
 * The state of the depth test is automatically restored when leaving the
 * current layout. Depth test is enabled by default.
 * @returns The previous state of the depth test.
 * @~french
 * Active le test de profondeur OpenGL.
 * Équivalent à l'appel OpenGL : <tt>glEnable(GL_DEPTH_TEST)</tt>
 * L'état précédent du test de profondeur est automatiquement rétabli lors
 * de la sortie du layout. Le test de profondeur est actif par défaut.
 * @returns L'état précédent du test de profondeur.
 */
enable_depth_test ();

/**
 * @~english
 * Disables writing into the depth buffer.
 * Equivalent to the OpenGL call: <tt>glDepthMask(GL_FALSE)</tt>.
 * The state of the depth mask is automatically restored when leaving the
 * current layout.
 * @returns The previous state of the depth mask.
 * @~french
 * Active l'écriture dans le buffer de profondeur.
 * Équivalent à l'appel OpenGL : <tt>glDepthMask(GL_FALSE)</tt>.
 * L'état précédent du masque de profondeur est automatiquement rétabli lors
 * de la sortie du layout.
 * @returns L'état précédent du masque de profondeur.
 */
disable_depth_mask ();

/**
 * @~english
 * Enables writing into the depth buffer.
 * Equivalent to the OpenGL call: <tt>glDepthMask(GL_TRUE)</tt>.
 * The state of the depth mask is automatically restored when leaving the
 * current layout. Writing into the depth buffer is enabled by default.
 * @returns The previous state of the depth mask.
 * @~french
 * Active l'écriture dans le buffer de profondeur.
 * Équivalent à l'appel OpenGL : <tt>glDepthMask(GL_TRUE)</tt>.
 * L'état précédent du masque de profondeur est automatiquement rétabli lors
 * de la sortie du layout. L'écriture dans le buffer de profondeur est actif
 * par défaut.
 * @returns L'état précédent du masque de profondeur.
 */
enable_depth_mask ();


/**
 * @~english
 * Specify the value used for depth buffer comparisons.
 *
 * Set the function used to compare each incoming pixel depth value
 * with the depth value present in the depth buffer.
 * @note The comparison is performed only if depth testing is enabled
 *
 * @param func specifies the conditions under which the pixel will be drawn. Accepted values are :
 * - Never
 * - Less
 * - Equal
 * - Lequal (lesser or equal)
 * - Greater
 * - Notequal (not equal)
 * - Gequal (greater or equal)
 * - Always
 *
 * @see For further informations, see http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml
 * @~french
 * Specifie la valeur utilisée pour les comparaisons du masque de profondeur.
 *
 * Change la fonction utilisée pour comparer la profondeur de chacun des pixels entrants avec celle
 * présente dans le masque de profondeur.
 * @note La comparaison est effectuée uniquement si le test de profondeur est activé.
 *
 * @param func Définit les conditions sous lesquelles les pixels seront dessinés. Les valeurs acceptées sont :
 * - Never (jamais)
 * - Less  (inférieur)
 * - Equal (égal)
 * - Lequal (inférieur ou égal)
 * - Greater (supérieur)
 * - Notequal (non-égal)
 * - Gequal (supérieur ou égal)
 * - Always (toujours)
 *
 * @see Pour plus d'information, voir http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml

 */
depth_function (func:text);

/**
 * @~english
 * Sends a keypress event.
 * @~french
 * Envoie un événement clavier.
 */
key(keyname:text);

/**
 * @~english
 * Return the current keyboard modifiers.
 * The keyboard modifiers are a bitwise or of the following values
 *  - 0x02000000	One of the Shift keys
 *  - 0x04000000	One of the Ctrl / Control keys
 *  - 0x08000000	One of the Alt keys
 *  - 0x10000000	One of the Meta keys
 *  - 0x20000000	A keypad button
 *  - 0x40000000	Mode_switch key (X11 only)
 *  
 * @note On Mac OS X, the Control modifier corresponds to
 * the Command keys on the Macintosh keyboard, and the
 * Meta modifier corresponds to the Control keys.
 * The Keypad modifier will also be set when an arrow key
 * is pressed as the arrow keys are considered part of
 * the keypad.
 *
 * @note On Windows keyboards, the Meta modifier is mapped
 *  to the optional Windows key.
 *
 * @~french
 * Renvoie l'état des touches de modification du clavier.
 * La valeur de retour de cette fonction est une combinaison
 * des valeurs suivantes :
 *  - 0x02000000 Shift (Maj)
 *  - 0x04000000 Ctrl
 *  - 0x08000000 Alt
 *  - 0x10000000 Meta
 *  - 0x20000000 Pavé numérique
 *
 * @note Sous MacOSX, Ctrl correspond à la touche Commande du clavier Mac,
 * et Meta correspond à la touche Ctrl.
 * Le modificateur de pavé numérique est présent également quand une touche de
 * direction est activée, car les flèches sont considérées comme faisant
 * partie du pavé numérique.
 *
 * @note Sous Windows, Meta correspond à la touche Windows.
 */
integer keyboard_modifiers();

/**
 * @~english
 * Get control modifier
 * @~french
 * Teste le modificateur Control
 * @~
@code
control_modifier  -> (keyboard_modifiers and 16#04000000) <> 0
@endcode
 */
boolean control_modifier();

/**
 * @~english
 * Get shift modifier
 * @~french
 * Teste le modificateur Shift
 * @~
@code
shift_modifier    -> (keyboard_modifiers and 16#02000000) <> 0
@endcode
 */
boolean shift_modifier();

/**
 * @~english
 * Get Alt modifier
 * @~french
 * Teste le modificateur Alt
 * @~
@code
alt_modifier      -> (keyboard_modifiers and 16#08000000) <> 0
@endcode
 */
boolean alt_modifier();

/**
 * @~english
 * Get Meta modifier
 * @~french
 * Teste le modificateur Meta
 * @~
@code
meta_modifier     -> (keyboard_modifiers and 16#10000000) <> 0
@endcode
 */
boolean meta_modifier();

/**
 * @~english
 * Do not execute a block of code.
 * @~french
 * Empêche l'exécution d'un bloc de code.
 */
disabled (B: block);

/** @} */
