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
 * @{
 */
/**
 * @~english
 * Converts from millimeters to pixels.
 * @param x [real] The value in mm
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des millimètres en pixels.
 * @param x [réel] La valeur en millimètres.
 * @returns La valeur en pixels.
 */
x ( mm);

/**
 * @~english
 * Converts from centimeters to pixels.
 * @param x [real] The value in cm
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des millimètres en pixels.
 * @param x [réel] La valeur en centimètres.
 * @returns La valeur en pixels.
 */
x ( cm);

/**
 * @~english
 * Converts from pixels to pixels.
 *
 * This function simply returns a copy of @p x.
 * @param x [real] The value in pixel
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des pixels en pixels.
 *
 * Cette fonction renvoie simplement un copie de @p x.
 * @param x [réel] La valeur en pixels
 * @returns La valeur en pixels.
 */
x ( px);

/**
 * @~english
 * Convert from points (pt) to pixels.
 *
 * @p x is a value in PostScript points (1/72 inch).
 * @param x [real] The value in point
 * @returns The value in pixel. 
 *
 * @~french
 * Convertit des points PostScript (pt) en pixels.
 *
 * @p x est une valeur en points PostScript (1/72 de pouce).
 * @param x [réel] La valeur en points.
 * @returns La valeur en pixels.
 */
x ( pt);

/**
 * @~english
 * Converts from inches to pixels.
 * @param x [real] The value in inches.
 * @returns The value in pixel.
 *
 * @~french
 * Convertit des pouces en pixels.
 * @param x [réel] La valeur en pouces.
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
 * be re-executed every @p interval seconds.
 *
 * @~french
 * Définit un intervalle de rafraîchissement du layout courant.
 *
 * Le @a layout courant (c'est-à-dire, la commande @ref locally ou @ref shape
 * qui contient le @ref refresh) sera ré-exécuté toutes les @p interval
 * secondes.
 */
refresh (interval:real);


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
 * Disables the OpenGL depth test for polygons.
 * @returns The previous state of depth testing
 * @~french
 * Désactive le test de profondeur OpenGL.
 * @returns L'état précédent du test de profondeur.
 */
disable_depth_test ();

/**
 * @~english
 * Enables the OpenGL depth test for polygons.
 * @returns The previous state of depth testing
 * @~french
 * Active le test de profondeur OpenGL.
 * @returns L'état précédent du test de profondeur.
 */
enable_depth_test ();

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

/** @} */
