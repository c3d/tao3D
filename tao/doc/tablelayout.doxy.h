/**
 * @~english
 * @addtogroup TableLayout Table layout
 * @ingroup TaoBuiltins
 *
 * Layout text or other items in columns and rows.
 *
 * Example (<a href="examples/table.ddd">table.ddd</a>):
 * @include table.ddd
 * The above code creates a simple two-column table. Each row consists in
 * a text label (in the first column), and a number (in the second column).
 * The last row is for the total.
 *
 * @image html table.png "A simple table layout (table.ddd)"
 *
 * @bug The above code does not work with the current version of Tao.
 *      @c for loops are currently not supported.
 * @todo formula, text_value not documented (my tests failed on interp)
 *
 * @~french
 * @addtogroup TableLayout Tableaux
 * @ingroup TaoBuiltins
 *
 * Place du text ou d'autres éléments an lignes et colonnes.
 *
 * Par exemple (<a href="examples/table.ddd">table.ddd</a>):
 * @include table.ddd
 * Le code ci-dessus crée un tableau à deux colonnes. Chaque ligne est
 * copmosée d'un texte et d'un nombre. La dernière ligne est le total.
 *
 * @image html table.png "Exemple de tableau"
 * @{
 */

/**
 * @~english
 * Creates a table of @p r rows and @p c columns centered at (@p x, @p y).
 *
 * @p x and @p y are the coordinates of the center of the table, in pixels.
 *
 * The @p contents of the table is a code block in which you should use
 * other table primitives to describe the table cells.
 *
 * Cells are inserted in rows, from left to right and top to bottom.
 *
 * @~french
 * Crée un tableau.
 * Le tableau est centré en (@p x, @p y), et comporte @p r lignes et @p p
 * colonnes. Le contenu du tableau est défini par @p contents, un bloc de code
 * dans lequel vous pouvez utiliser les autres primitives de cette page pour
 * décrire les cellules. Les cellules sont insérées ligne après ligne, de
 * haut en bas et de gauche à droite.
 */
table(x:real, y:real, r:integer, c:integer, contents:tree);

/**
 * @~english
 * Creates a table of @p r rows and @p c columns.
 * The table is centered at the origin.
 * @~french
 * Crée un tableau centré à l'origine.
 * @~
 * @see table(x:real, y:real, r:integer, c:integer, contents:tree)
 */
table(r:integer, c:integer, contents:tree);

/**
 * @~english
 * Inserts a fixed-size cell of @p w x @p h pixels.
 * The @p contents code block is executed and is typically used to define
 * the cell's contents.
 * @~french
 * Insère une cellule de taille fixe.
 * La taille de la cellule est @p w x @p h pixels.
 */
table_cell(w:integer, h:integer, contents:tree);


/**
 * @~english
 * Inserts a variable-sized cell.
 * The cell will normally have the smallest possible dimension to fit
 * its contents. However a bigger cell in the same row or same column will
 * cause the current cell to adapt its size.
 *
 * @~french
 * Insère une cellule de taille variable.
 * La cellule a la plus petite dimension possible qui permet a son contenu de
 * s'afficher en entier. Le tableau adapte la taille de ses lignes et de ses
 * colonnes à la plus grande cellule.
 * @~
 * @see table_cell(w:integer, h:integer, contents:tree)
 */
table_cell(contents:tree);


/**
 * @~english
 * Defines margins for all cells in the table.
 * A horizontal space of @p w pixels is added to the left and right sides of
 * each cell. A vertical space of @p h pixels is added to the top and
 * bottom sides.
 *
 * @~french
 * Définit la taille des marges de toutes les cellules du tableau.
 * Un espace horizontal de @p w pixels est ajouté à gauche et à droite de
 * chaque cellule. De même, un espace vertical de @p h pixels est ajouté en
 * haut et en bas de chaque cellule.
 */
table_cell_margins(w:integer, h:integer);


/**
 * @~english
 * Defines margins for all cells in the table.
 * This allow margin to not be centered.
 * @~french
 * Définit la taille des marges de toutes les cellules du tableau.
 * Cette forme permet de créer des marges asymétriques.
 * @~
 * @see table_cell_margins(w:integer, h:integer)
 */
table_cell_margins(x:real, y:real, w:integer, h:integer);


/**
 * @~english
 * Defines common fill code for cells.
 *
 * The @p contents code block is executed once for each cell in the table,
 * before any table_cell specific code is executed. You may use this
 * form to define the cell background color. The following code, for
 * instance, will create a light blue background:
 *
 * @~french
 * Définit du code commun pour toutes les cellules.
 * Le code @p contents est exécuté une fois pour chaque cellule du tableau,
 * avant que le contenu de la cellule (cf. @ref table_cell) ne soit affiché.
 * Vous pouvez utiliser cette fonction par exemple pour définir une couleur
 * de remplissage. Le code qui suit, par exemple, crée un fond bleu clair :
 * @~
 @code
table nr, nc,
    table_cell_fill
        line_width 0
        color "lightblue"
        rectangle table_cell_x, table_cell_y, table_cell_w, table_cell_h
 @endcode
 */
table_cell_fill(contents:tree);


/**
 * @~english
 * Defines common border code for cells.
 *
 * The @p contents code block is executed once for each cell in the table,
 * before any table_cell specific code is executed and after code declared
 * by @ref table_cell_fill.
 * @todo Isn't this function redundant with table_cell_fill?
 *
 * @~french
 * Définit du code commun pour toutes les cellules.
 * Le code @p contents est exécuté une fois pour chaque cellule du tableau,
 * avant que le contenu de la cellule (cf. @ref table_cell) ne soit affiché,
 * mais après le code déclaré par @ref table_cell_fill.
 */
table_cell_border(contents:tree);


/**
 * @~english
 * The x coordinate of the center of the current cell, in pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 *
 * @~french
 * Renvoie l'abscisse du centre de la cellule courante, en pixels.
 * Valide seulement dans le contexte d'un appel à @ref table_cell_fill,
 * @ref table_cell_border ou @ref table_cell.
 */
real table_cell_x();


/**
 * @~english
 * The x coordinate of the center of the current cell, in pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 *
 * @~french
 * Renvoie l'ordonnée du centre de la cellule courante, en pixels.
 * Valide seulement dans le contexte d'un appel à @ref table_cell_fill,
 * @ref table_cell_border ou @ref table_cell.
 */
real table_cell_y();


/**
 * @~english
 * The width of the current cell, in pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 *
 * @~french
 * Renvoie la largeur de la cellule courante, en pixels.
 * Valide seulement dans le contexte d'un appel à @ref table_cell_fill,
 * @ref table_cell_border ou @ref table_cell.
 */
real table_cell_w();


/**
 * @~english
 * The height of the current cell, in pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 *
 * @~french
 * Renvoie la hauteur de la cellule courante, en pixels.
 * Valide seulement dans le contexte d'un appel à @ref table_cell_fill,
 * @ref table_cell_border ou @ref table_cell.
 */
real table_cell_h();


/**
 * @~english
 * The row index of the current cell.
 * First row has index 0.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 *
 * @~french
 * Renvoie le numéro de la ligne à laquelle appartient la cellule courante.
 * La première ligne porte le numéro 0.
 * Valide seulement dans le contexte d'un appel à @ref table_cell_fill,
 * @ref table_cell_border ou @ref table_cell.
 */
integer table_cell_row();


/**
 * @~english
 * The column index of the current cell.
 * First column has index 0.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 *
 * @~french
 * Renvoie le numéro de la colonne à laquelle appartient la cellule courante.
 * La première colonne porte le numéro 0.
 * Valide seulement dans le contexte d'un appel à @ref table_cell_fill,
 * @ref table_cell_border ou @ref table_cell.
 */
integer table_cell_column();


/**
 * @~english
 * The number of rows in the current table.
 * Valid only in the context of the code block of @ref table command.
 *
 * @~french
 * Le nombre de lignes dans le tableau.
 * Valide seulement dans le contexte d'un appel à @ref table.
 */
integer table_rows();


/**
 * @~english
 * The number of columns in the current table.
 * Valid only in the context of the code block of @ref table command.
 *
 * @~french
 * Le nombre de colonnes dans le tableau.
 * Valide seulement dans le contexte d'un appel à @ref table.
 */
integer table_columns();


/**
 * @}
 */
