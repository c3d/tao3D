/**
 * @~english
 * @addtogroup TaoChooser Command chooser
 * @ingroup TaoBuiltins
 *
 * Create and display a command menu with interactive search capabilities.
 *
 * The command chooser primitives are designed to enable the creation of
 * interactive menus within a presentation. The display of the menu may be
 * triggered by a key press or mouse click, for instance. When the chooser
 * is active, the user can:
 *   @li use the up and down arrow keys to scroll through the menu options,
 *   @li press return to select an entry and execute the associated action,
 *   @li press escape to leave the menu
 *   @li use the keyboard to perform an interactive search among the chooser
 *       options. Following each key press, the list of entries is reduced
 *       to show only the ones that match the text the user has entered so
 *       far. For instance, type "e": the chooser will show only the options
 *       that contain an "e". Press "x": only the entry that contain "ex"
 *       will be displayed.
 *
 * @note You cannot validate an entry with the mouse. You need to press the
 * return key instead.
 *
 * The same primitives are used by Tao to display the main application
 * menu (when you press the escape key).
 *
 * Let's illustrate this with an example,
 * (<a href="examples/chooser.ddd">chooser.ddd</a>).
 * @include chooser.ddd
 *
 * The first picture shows the command chooser as the user has just pressed
 * the "a" key to show the manu. The second one show the updated command list
 * after the user entered a "g".
 * @image html chooser.png "Using the command chooser"
 *
 * @note The message sent by writeln are visible on the application's
 * standard ouput. See @ref secStdoutStderr "Standard Ouput, Standard Error"
 * for details.
 * @note Currently, the XL forms controlling the appearance of the chooser
 * are defined in <tt>tao.xl</tt> and cannot be overriden in a Tao document.
 *
 * @~french
 * @addtogroup TaoChooser Menu interactif
 * @ingroup TaoBuiltins
 *
 * Crée et affiche un menu de commandes avec des capacités de filtrage
 * interactif.
 *
 * Les primitives du menu interactif (appelé <em>command chooser</em> en
 * anglais) permettent la création de menus au sein d'une présentation.
 * L'affichage du menu peut être déclenché par l'appui sur une touche ou
 * ou par un clic de souris, par exemple. Quand le menu est affiché,
 * l'utilisateur peut :
 *   @li utiliser les flèches haut et bas pour se déplacer parmi les choix
 *       proposés par le menu
 *   @li appuyer sur la touche Entrée pour valider un choix et exécuter la
 *       commande associée
 *   @li appuyer sur la touche Échap pour sortir du menu
 *   @li utiliser le clavier pour filtrer les options proposées.
 *       Après chaque appui sur une touche, la liste des options proposées
 *       est réduite pour ne montrer que les lignes qui contiennent le texte
 *       tapé jusqu'ici. Par exemple, tapez «e» : le menu ne montre que les
 *       choix qui contiennent la lettre «e». Puis tapez «x» : seules les
 *       choix qui contiennent «ex» sont affichés.
 *
 * @note Vous ne pouvez pas valider une option en cliquant avec la souris.
 * Seule la touche Entrée permet de valider votre choix.
 *
 * Les primitives décrites dans cette page sont les mêmes utilisées par
 * Tao Presentations pour afficher le menu principal de l'application, qui
 * apparaît lorsque vous appuyez sur la touche Échap.
 *
 * Illustrons celà par un exemple,
 * (<a href="examples/chooser.ddd">chooser.ddd</a>).
 * @include chooser.ddd
 *
 * La première image montre l'état du menu lorsque l'utilisateur a appuyé
 * sur la touche «a» pour afficher le menu. La seconde image montre le menu
 * après que l'utilisateur a appuyé sur «g».
 * @image html chooser.png "L'utilisation du menu interactif"
 *
 * @note Les message affichés par writeln sont visibles sur stdout.
 * Cf. @ref secStdoutStderr pour plus de details.
 * @note Actuellement, les formes XL qui définissent l'apparence du menu
 * sont définies dans <tt>tao.xl</tt> et ne peuvent pas être redéfinies dans
 * un document Tao.
 *
 * @~
 * @{
 */


/**
 * @~english
 * Creates and shows a chooser with the given caption.
 * A chooser shows a selection among the possible commands.
 * @~french
 * Crée et affiche un menu interactif avec le titre spécifié.
 * Le menu permet de choisir une commande parmi une liste.
 */
tree chooser(caption:text);


/**
 * @~english
 * Adds a command into the current chooser.
 * Creates a chooser item, associates the @p action code block to the item,
 * and appends a new line to the current chooser list. @p action is
 * executed when the user presses the return key on the highlighted command.
 * @~french
 * Ajoute une commande au menu actuellement affiché.
 * Crée une commande dans le menu, y associe le bloc de code @p action,
 * et ajoute une ligne supplémentaire à la liste des commandes du menu.
 * @p action est exécutée quand l'utilisateur appuie sur la touche Entrée.
 */
tree chooser_choice(label:text, action:tree);


/**
 * @~english
 * Adds commands into the current chooser, by name.
 * All the commands in the current symbol table that have the given prefix are
 * added to the chooser.
 * The current symbol table is scanned, and each time a parameterless function
 * starting with @p prefix is found, a new command is added to the current
 * chooser. The label of the command is the concatenation of:
 *    @li @p label_prefix, and
 *    @li the end of the symbol name, with all underscores changed into spaces.
 *
 * When a command is chosen, the associated action is executed.
 * @~french
 * Ajoute une commande au menu actuellement affiché, par symbole.
 * Tous les symboles, dans la table de symboles courante, qui ont un préfixe en
 * commun avec le texte spécifié, sont ajoutés au menu actuellement affiché.
 * La table des symboles est parcourue, et à chaque fois qu'une fonction sans
 * paramètre dont le nom commence par @p prefix est rencontrée, une nouvelle
 * commande est ajoutée au menu en cours. Le texte affiché dans le menu est la
 * concaténation de :
 *    @li @p label_prefix, et
 *    @li la fin du nom du symbole, dans lequel tous les soulignés (_) sont
 *        remplacés par un espace.
 *
 * Quand une commande est sélectionnée, l'action associée est exécutée.
 */
tree chooser_commands(prefix:text, label_prefix:text);

/**
 * @~english
 * Adds one command for each page into the current chooser.
 * This primitive is useful when you want to give the user the opportunity to
 * select a page from the current document, and execute an action related to
 * this page. The page table is scanned, and for each page a new command is added
 * into the current chooser. The label of the command is the concatenation of:
 *    @li @p label_prefix,
 *    @li the page number,
 *    @li a space character, and
 *    @li the page name.
 *
 * When a command is chosen, the symbol action is executed and is passed the
 * page name (without the page number).
 * @~french
 * Ajoute une commande au menu en cours pour chaque page du document.
 * Cette primitive est utile lorsque vous souhaitez donner à l'utilisateur
 * l'opportunité de sélectionner une page du document en cours, et d'exécuter
 * une action relativement à cette page.
 * La table de pages est parcourue, et pour chaque page une nouvelle commande
 * est ajoutée au menu en cours. Le texte affiché dans le menu est la
 * concaténation de :
 *    @li @p label_prefix,
 *    @li le numéro de page,
 *    @li un espace, et
 *    @li le nom de la page.
 *
 * Quand une commande est sélectionnées, @p action est exécutée et reçoit
 * le nom de la page (sans le numéro de page).
 */
tree chooser_pages(action:name, label_prefix:text);

/**
 * @}
 */
