/**
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
 * @{
 */

/**
 * Creates a table of @p r rows and @p c columns centered at (@p x, @p y).
 *
 * @p x and @p y are the coordinates of the center of the table, in pixels.
 *
 * The @p contents of the table is a code block in which you should use
 * other table primitives to describe the table cells.
 *
 * Cells are inserted in rows, from left to right and top to bottom.
 *
 * @return [tree] the contents evaluation result.
 */
tree table(x:real, y:real, r:integer, c:integer, contents:tree);

/**
 * Creates a table of @p r rows and @p c columns.
 *
 * The table is centered at the origin.
 * @see table(x:real, y:real, r:integer, c:integer, contents:tree)
 * @return [tree] the contents evaluation result.
 */
tree table(r:integer, c:integer, contents:tree);

/**
 * Inserts a fixed-size cell of @p w x @p h pixels.
 *
 * The @p contents code block is executed and is typically used to define
 * the cell's contents.
 * @return [tree] the contents evaluation result.
 */
tree table_cell(w:integer, h:integer, contents:tree);


/**
 * Inserts a variable-sized cell.
 *
 * The cell will normally have the smallest possible dimension to fit
 * its contents. However a bigger cell in the same row or same column will
 * cause the current cell to adapt its size.
 * @see table_cell(w:integer, h:integer, contents:tree)
 * @return [tree] the contents evaluation result.
 */
tree table_cell(contents:tree);


/**
 * Defines margins for all cells in the table.
 *
 * A horizontal space of @p w pixels is added to the left and right sides of
 * each cell. A vertical space of @p h pixels is added to the top and
 * bottom sides.
 */
table_cell_margins(w:integer, h:integer);


/**
 * Defines margins for all cells in the table.
 *
 * This allow margin to not be centered.
 */
table_cell_margins(x:real, y:real, w:integer, h:integer);


/**
 * Defines common fill code for cells.
 *
 * The @p contents code block is executed once for each cell in the table,
 * before any table_cell specific code is executed. You may use this
 * form to define the cell background color. The following code, for
 * instance, will create a light blue background:
 @code
table nr, nc,
    table_cell_fill
        line_width 0
        color "lightblue"
        rectangle table_cell_x, table_cell_y, table_cell_w, table_cell_h
 @endcode
 *
 */
table_cell_fill(contents:tree);


/**
 * Defines common border code for cells.
 *
 * The @p contents code block is executed once for each cell in the table,
 * before any table_cell specific code is executed and after code declared
 * by @ref table_cell_fill.
 * @todo Isn't this function redundant with table_cell_fill?
 */
table_cell_border(contents:tree);


/**
 * The x coordinate of the center of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 * @return [real] The x-coordinate in pixels.
 */
real table_cell_x();


/**
 * The y coordinate of the center of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 * @return [real] The y-coordinate in pixels.
 */
real table_cell_y();


/**
 * The width of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 * @return [real] The width in pixels.
 */
real table_cell_w();


/**
 * The height of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 * @return [real] The height in pixels.
 */
real table_cell_h();


/**
 * The row index of the current cell. First row has index 0.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 * @return [integer] The row index.
 */
integer table_cell_row();


/**
 * The column index of the current cell. First column has index 0.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 * @return [integer] The column index.
 */
integer table_cell_column();


/**
 * The number of rows in the current table.
 * Valid only in the context of the code block of @c table command.
 * @return [integer] The number of row.
 */
integer table_rows();


/**
 * The number of columns in the current table.
 * Valid only in the context of the code block of @c table command.
 * @return [integer] The number of column.
 */
integer table_columns();


/**
 * @}
 */
