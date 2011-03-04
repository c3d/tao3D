/**
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
 *       that contain an "e". Press "x": only the entrie that contain "ex"
 *       will be displayed.
 *
 * The same primitives are used by Tao to display the main application
 * menu (when you press the escape key).
 *
 * Let's illustrate this with an example.
 @verbatim
// chooser.ddd

animal_chooser ->
    chooser "Select an animal"
    chooser_choice "A dog", { writeln "Bark! Bark!" }
    chooser_choice "A cat", { writeln "Meowww..." }
    chooser_commands "wild_animal_", "(Wild) "

wild_animal_A_lion ->
    writeln "Roarrr!"

wild_animal_A_tiger ->
    writeln "Growl!"

key "a" -> animal_chooser
 @endverbatim
 *
 * The first picture shows the command chooser as the user has just pressed
 * the "a" key to show the manu. The second one show the updated command list
 * after the user entered a "g".
 * @image html chooser.png "Using the command chooser"
 *
 * @note Currently, the XL forms controlling the appearance of the chooser
 * are defined in <tt>tao.xl</tt> and cannot be overriden in a Tao document.
 *
 * @{
 */


/**
 * Creates and shows a chooser with the given caption.
 * A chooser shows a selection among the possible commands.
 */
tree chooser(text caption);


/**
 * Adds a command into the current chooser.
 * Creates a chooser item, associates the @a action code block to the item,
 * and appends a new line to the current chooser list. @a action is
 * executed when the user presses the return key on a highlighted command.
 */
tree chooser_item(text label, code action);


/**
 * Adds commands into the current chooser, by name.
 * All the commands in the current symbol table that have the given prefix are
 * added to the chooser.
 * The current symbol table is scanned, and each time a parameterless function
 * starting with @a prefix is found, a new command is added to the current
 * chooser. The label of the command is the concatenation of:
 *    @li @a label_prefix, and
 *    @li the end of the symbol name, with all underscores changed into spaces.
 *
 * When a command is chosen, the associated action is executed.
 */
tree chooser_commands(text prefix, text label_prefix);

/**
 * Adds one command for each page into the current chooser.
 * This primitive is useful when you want to give the user the opportunity to
 * select a page from the current document, and execute an action related to
 * this page. The page table is scanned, and for each page a new command is added
 * into the current chooser. The label of the command is the concatenation of:
 *    @li @a label_prefix,
 *    @li the page number,
 *    @li a space character, and
 *    @li the page name.
 *
 * When a command is chosen, the symbol action is executed and is passed the
 * page name (without the page number).
 */
tree chooser_pages(name action, text label_prefix);

/**
 * @}
 */
