/**
 * @addtogroup TableLayout Table layout
 * @ingroup TaoBuiltins
 *
 * Layout text or other items in columns and rows.
 *
 * Example:
 @code
// table.ddd

cols := 2
rows := 3

my_data 1 -> "Foo", 12.0
my_data 2 -> "Bar", 5.2
my_data 3 -> "Baz", 100.0

first x, y -> x
second x, y -> y

my_sum from, to ->
    s := 0
    for i in from..to loop
        s += second my_data i
    s

font_size 18
table 0, 0, rows + 1, cols,
    table_cell_margins 10, 10
    table_cell_fill
        line_width 1
        line_color "black"
        if table_cell_row <> table_rows - 1 then
            color "transparent"
        else
            color 0, 0, 1, 20%
        rectangle table_cell_x, table_cell_y, table_cell_w, table_cell_h
    for i in 1..rows loop
        table_cell
            align_left
            text first my_data i
        table_cell
            align_right
            text text second my_data i
    table_cell
        text "Total"
    table_cell
        text text my_sum (1, rows)
 @endcode
 *
 * The above code creates a simple two-column table. Each row consists in
 * a text label (in the first column), and a number (in the second column).
 * The last row is for the total.
 *
 * @image html table.png "A simple table layout (table.ddd)"
 *
 * @todo formula, text_value not documented (my tests failed on interp)
 * @{
 */

/**
 * Creates a table of @a r rows and @a c columns centered at (@a x, @a y).
 *
 * @a x and @a y are the coordinates of the center of the table, in pixels.
 *
 * The @a contents of the table is a code block in which you should use
 * other table primitives to describe the table cells.
 *
 * Cells are inserted in rows, from left to right and top to bottom.
 */
tree table(real x, real y, integer r, integer c, code contents);

/**
 * Creates a table of @a r rows and @a c columns.
 *
 * The table is centered at the origin.
 * @see table(real x, real y, integer r, integer c, code contents)
 */
tree table(integer r, integer c, code contents);

/**
 * Inserts a fixed-size cell of @a w x @a h pixels.
 *
 * The @a contents code block is executed and is typically used to define
 * the cell's contents.
 */
tree table_cell(integer w, integer h, code contents);


/**
 * Inserts a variable-sized cell.
 *
 * The cell will normally have the smallest possible dimension to fit
 * its contents. However a bigger cell in the same row or same column will
 * cause the current cell to adapt its size.
 * @see table_cell(integer w, integer h, code contents)
 */
tree table_cell(code contents);


/**
 * Defines margins for all cells in the table.
 *
 * A vertical space of @a w pixels is added to the left and right sides of
 * each cell. A horizontal space of @a h pixels is added to the top and
 * bottom sides.
 */
tree table_cell_margins(integer w, integer h);


/**
 * Defines margins for all cells in the table.
 *
 * @todo What is the purpose of @a x and @a y parameters? What is the
 * difference with the other table_cell_margins primitive ?.
 */
tree table_cell_margins(real x, real y, integer w, integer h);


/**
 * Defines common fill code for cells.
 *
 * The @a contents code block is executed once for each cell in the table,
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
tree table_cell_fill(code contents);


/**
 * Defines common border code for cells.
 *
 * The @a contents code block is executed once for each cell in the table,
 * before any table_cell specific code is executed and after code declared
 * by @ref table_cell_fill.
 * @todo Isn't this function redundant with table_cell_fill?
 */
tree table_cell_border(code contents);


/**
 * The x coordinate of the center of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 */
real table_cell_x();


/**
 * The y coordinate of the center of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 */
real table_cell_y();


/**
 * The width of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 */
real table_cell_w();


/**
 * The height of the current cell. In pixels.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 */
real table_cell_h();


/**
 * The row index of the current cell. First row has index 0.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 */
integer table_cell_row();


/**
 * The column index of the current cell. First column has index 0.
 * Valid only in the context of the code block of @ref table_cell_fill,
 * @ref table_cell_border or @ref table_cell.
 */
integer table_cell_column();


/**
 * The number of rows in the current table.
 * Valid only in the context of the code block of @c table command.
 */
integer table_rows();


/**
 * The number of columns in the current table.
 * Valid only in the context of the code block of @c table command.
 */
integer table_columns();


/**
 * @}
 */
