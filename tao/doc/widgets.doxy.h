/**
 * @addtogroup Widgets GUI widgets
 * @ingroup TaoBuiltins
 *
 * Inserts Graphical User Interface widgets (such as: text edit box,
 * push button, radio button, web browser...).
 *
 * The widgets available through Tao primitives are provided by the Qt
 * library. For instance, @c line_edit creates a QLineEdit object,
 * @c push_button uses QPushButton, and so on.
 *
 * @section regular_widgets Regular widgets
 * Widgets are all created as a texture and then mapped onto a rectangle
 * (or available to be mapped onto another object).
 *
 * @param x [real] x-coordinate of widget center
 * @param y [real] y-coordinate of widget center
 * @param w [real] width of widget
 * @param h [real] height of widget
 * @param name [text] Name that uniquely identify the widget
 *
 * @section dialogbox_widgets Dialog box widgets
 * Widgets using dialog box like font chooser are available in two formats :
 * -# like other widget in a pair of chooser_texture and chooser with x, y, w, h parameters<BR>
 *   or
 * -# in an independent window.
 *
 * There is currently three kind of chooser :
 * -# @ref filechooser "file chooser"
 * -# @ref fontchooser "font chooser"
 * -# @ref colorchooser "color chooser"
 *
 * They all use sub-commands to set there specific properties like filters for file chooser, etc...
 *
 * Then the content of the associated action is parsed and specific names are replaced by their value, e.g.
 * @c file_name is replaced by the name of the selected file.

 * @{
 */
/**
 * Make a texture out of a given push button.
 *
 *  Make a platform dependant texture out of a push button
 *
 * @param label [text] Label of the button
 * @param action [tree] The action to perform when the button is clicked.
 */
push_button_texture ( w:real, h:real, name:text, label:text, action:tree);

/**
 * A push button.
 *
 *  Draw a simple push button in the curent frame. The look and feel of the button is platform dependant. 
 *
 * @param label [text] Label of the button
 * @param action [tree] The action to perform when the button is clicked.
 */
push_button ( x:real, y:real, w:real, h:real, name:text, label:text, action:tree);

/**
 * Make a texture out of a radio button.
 *
 * Draw a radio button in the curent frame. Associate the action to perform when
 * the button state change from non-selected to selected. Radio buttons are mutually exclusive,
 * if you want to have multiple groups of radio buttons, uses the @ref button_group.
 *
 * @param label [text] Label of the button
 * @param marked [text] @c "true" if the button is selected, @c "false" otherwise.
 * This parameter is not represented by a boolean, because we cannot rewrite boolean, 
 * so marked value would not be registered.
 * @param action [tree] The action to perform when the button state change from non-selected to selected.
 */
radio_button_texture ( w:real, h:real, name:text, label:text, marked:text, action:tree);

/**
 * Draw a radio button.
 *
 * @copydetails radio_button_texture
 *
 */
radio_button (x:real, y:real, w:real, h:real, name:text, label:text, marked:text, action:tree);

/**
 * Create a logical group.
 *
 * Create a group for buttons. Buttons included in this group are mutually exclusive.
 * It is useful to create multiple group of radio buttons. This primitive does not have
 * graphical representation. If you want one use @ref group_box.
 *
 * When an action in this group occurs (like a button is clicked), the button's @p action 
 * is called and then the @ref button_group_action code, if one was present in the body. 
 *
 * @param ex [boolean] If true the buttons inside the group are mutually exclusive.
 * @param body [tree] The code that describes buttons. May contains a button_group_action command.
 */
button_group ( ex:boolean, body:tree);

/**
 *  Action for a group.
 *
 *  Set the @p action to be executed by the current @ref button_group. When a button in the group is clicked,
 *  this action is triggered as well as the button's action.
 */
button_group_action ( action:tree );


/**
 * Texture group box.
 *
 *  Make a texture out of a group box.
 *
 * @param label [text] The label of the box
 */
group_box_texture ( w:real, h:real, label:text);

/**
 * Creates a group box.
 *
 * Draw a group box in the current frame, and fill it with the @p body.
 * The x and y coordinate of widgets placed in this box 
 *  are respectively colomn and row number. Total number of rows and columns is dynamic.
 *
 * @param label [text] The label of the box
 * @param body [tree] The tree describing the content of the box
 */
group_box ( x:real, y:real, w:real, h:real, label:text, body:tree);


/**
 * Make a texture out of a checkbox button.
 *
 * Creates a texture for a checkbox button.
 *
 * @param label [text] Label of the button
 * @param marked [text] @c "true" if the button is marked, @c "false" otherwise.
 * This parameter is not represented by a boolean, because we cannot rewrite boolean, 
 * so marked value would not be registered.
 * @param action [tree] The action to perform when the button state change from non-set to set
 */
check_box_button_texture (w:real, h:real, name:text, label:text, marked:text, action:tree);

/**
 * Creates a check button.
 *
 *  Draw a check button in the curent frame from the checkbox_button_texture.
 *@copydetails check_box_button_texture
 */
check_box_button (x:real, y:real, w:real, h:real, name:text, label:text, marked:text, action:tree);

/**
 * Make a texture out of a line editor.
 *
 * A line edit allows the user to enter and edit a single @p line of plain text with a useful
 * collection of editing functions, including undo and redo, cut and paste, and drag and drop.
 * @param line is the text present in the line editor. It is updated if edited
 * from the interface and shown if modify in the code.
 *
 */
line_edit_texture ( w:real, h:real, line:text);

/**
 * Creates a line editor.
 *
 * @copydetails line_edit_texture
 */
line_edit (x:real, y:real, w:real, h:real, line:text);

/**
 * @}
 */

/**********************************************************************************************/


/**
 * @addtogroup filechooser File chooser widget
 * @ingroup Widgets
 *
 * File chooser widget documentation.
 *
 * The file chooser is based on <a href="http://doc.qt.nokia.com/4.7/qfiledialog.html">QFileDialog</a> box.
 *
 * The file_chooser can be parameterized with 4 attributes which are
 * - @c label,
 * - @c directory,
 * - @c filter,
 * - @c action.
 *
 * Those attributes are defined in the function with file_chooser_ prefix.
 * In the file_chooser body, either short names and long names of those attributes are available.
 *
 * @anchor filevalues
 * Once the file is selected, following Names are replaced by their corresponding values from the
 * selected file. The values are extracted from a
 * <a href="http://doc.qt.nokia.com/4.7/qfileinfo.html">QFileInfo</a>.
 * <table>
 * <tr><th> XL Name           </th><th> Replacement value </th></tr>
 * <tr><td> @c file_name      </td><td> QFileInfo.fileName </td> </tr>
 * <tr><td> @c file_directory </td><td> QFileInfo.cannonicalPath </td> </tr>
 * <tr><td> @c file_path      </td><td> QFileInfo.cannonicalFilePath </td> </tr>
 * <tr><td> @c rel_file_path  </td><td> a computed relative path with no subdirectory
 * reference (no <tt>..</tt> ). Returned value is one of the following ones in order of precedence. 
 *    -# relative to current document folder
 *    -# relative to user's home directory (with <tt>~</tt> prepended)
 *    -# absolute : QFileInfo.cannonicalFilePath </td> </tr>
 * </table>
 *
 * Example of code used to show an image chooser (from a file).
 *@code
image_chooser -> 
    file_chooser
        filter "All image formats (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.svg *.tif *.tiff *.xbm *.xpm);;" &
           "BMP - Windows Bitmap (*.bmp);;" &
           "GIF - Graphic Interchange Format (*.gif);;" &
           "JPEG - Joint Photographic Experts Group (*.jpg *.jpeg);;" &
           "PNG - Portable Network Graphics (*.png);;" &
           "SVG - Scalable Vector Graphics (*.svg);;" &
           "TIFF - Tagged Image File Format (*.tif *.tiff);;" &
           "PBM - Portable Bitmap (*.pbm);;" &
           "PGM - Portable Graymap (*.pgm);;" &
           "PPM - Portable Pixmap (*.ppm);;" &
           "XBM - X11 Bitmap (*.xbm);;" &
           "XPM - X11 Pixmap (*.xpm)"
        action
            insert "Insert image",
                shape
                    color 100%,100%,100%,100%
                    image 0, 0, 100%, 100%, rel_file_path
@endcode
 * @{
 */
/**
 *  File chooser directory.
 *
 *  Set the directory to open first to @p d.
 *
 */
file_chooser_directory ( d:text );

/**
 * Set different labels on a file dialog.
 *
 * Available labels are :
 * - @c LookIn
 * - @c FileName
 * - @c FileType 
 * - @c Accept
 * - @c Reject
 *
 * @param label_name [text] The name of the label to set
 * @param label_value [text] The value to display
 */
file_chooser_label ( label_name:text, label_value:text);

/**
 * Set the file filters.
 *
 *  Set the file filters (file pattern, e.g. *.img).
 * A sequence of filters can be provided separated by <tt>;;</tt>.
 * Filters are made of either the file pattern, or of a textual description 
 * followed by the file pattern enclosed in parenthesis <tt>()</tt>.
 *
 *Example :
@code
filter "C files (*.c);;Header files (*.h)"
@endcode
 *
 */
file_chooser_filter ( filter:text);

/**
 * File chooser action
 *
 *  Set the action that will be execute when OK is pressed. Only available in a file chooser sub tree.
 * This action may contain one or more of the predefined @ref filevalues "XL Names" that will be replaced
 * by their real value.
 *
 * @param body [tree] The action to modify and execute when file is chosen.
 */
file_chooser_action ( body:tree);

/**
 * Pops a file chooser dialog box.
 * Pop a window with the platform dependant file chooser inside.
 * @copydetails file_chooser(x:real,y:real,w:real,h:real,body:tree)
 */
file_chooser (body:tree);

/**
 * Creates a file chooser dialog box.
 *
 * @param body [tree] Description of the file dialog box.
 */
file_chooser (x:real, y:real, w:real, h:real, body:tree);

/**
 * Make a texture out of a given file chooser dialog box.
 *
 * @copydetails file_chooser(x:real,y:real,w:real,h:real,body:tree)
 */
file_chooser_texture (w:real, h:real, body:tree);


/**
 * @}
 */



/**
 * @addtogroup fontchooser Font chooser widget
 * @ingroup Widgets
 *
 * Font chooser widget documentation.
 *
 * The font chooser is based on <a href="http://doc.qt.nokia.com/4.7/qfontdialog.html">QFontDialog</a> box.
 *
 * The font_chooser cannot be parameterized.
 * @anchor fontvalues
 * Once the font is selected, following Names are replaced by their corresponding values from the
 * selected font. The values are extracted from a
 * <a href="http://doc.qt.nokia.com/4.7/qfont.html">QFont</a>.
 * <table>
 * <tr><th> XL Name           </th><th> Replacement value    </th> </tr>
 * <tr><td> @c font_family    </td><td> QFont.family         </td> </tr>
 * <tr><td> @c font_size      </td><td> QFont.pointSize      </td> </tr>
 * <tr><td> @c font_weight    </td><td> QFont.weight         </td> </tr>
 * <tr><td> @c font_slant     </td><td> QFont.style() * 100  </td> </tr>
 * <tr><td> @c font_stretch   </td><td> QFont.stretch        </td> </tr>
 * <tr><td> @c font_is_italic </td><td> QFont.italic == true </td> </tr>
 * <tr><td> @c font_is_bold   </td><td> QFont.bold   == true </td> </tr>
 * </table>
 *
 * Example of code that define a font chooser.
 *@code
text_font_chooser ->
    font_chooser
        set_attribute "font", {
            font font_family, 
            font_size, 
            slant font_slant%,
            weight font_weight%, 
            stretch font_stretch% }

@endcode
 * @{
 */
/**
 * Shows a font chooser.
 * @param action The action to be parsed, updated and then executed when the font is selected.
 */
font_chooser_texture ( w:real, h:real, action:tree );

/**
 * @copydoc font_chooser_texture
 */
font_chooser ( action:tree );

/**
 * @copydoc font_chooser_texture
 */
font_chooser (x:real, y:real, w:real, h:real, action:tree );

/**
 * @}
 */


/**
 * @addtogroup colorchooser Color chooser widget
 * @ingroup Widgets
 *
 * Color chooser widget documentation.
 *
 * The color chooser is based on <a href="http://doc.qt.nokia.com/4.7/qcolordialog.html">QColorDialog</a> box.
 *
 * The color_chooser cannot be parameterized.
 * @anchor colorvalues
 * Once the color is selected, following Names are replaced by their corresponding values from the
 * selected color. The values are extracted from a
 * <a href="http://doc.qt.nokia.com/4.7/qColor.html">QColor</a>.
 * <table>
 * <tr><th> XL Name  </th><th> Replacement value </th> </tr>
 * <tr><td> @c red   </td><td> QColor.redF       </td> </tr>
 * <tr><td> @c green </td><td> QColor.greenF     </td> </tr>
 * <tr><td> @c blue  </td><td> QColor.blueF      </td> </tr>
 * <tr><td> @c alpha </td><td> QColor.alphaF     </td> </tr>
 * </table>
 *
 * Example of code that define a font chooser.
 *@code
shape_color_chooser ->
    color_chooser "color",
        set_attribute "color", {color red, green, blue, alpha}

@endcode
 * @{
 */
/**
 * Shows a color chooser.
 * @param action The action to be parsed, updated and then executed when the color is selected.
 */
color_chooser_texture ( w:real, h:real, action:tree );

/**
 * @copydoc color_chooser_texture
 */
color_chooser ( action:tree );

/**
 * @copydoc color_chooser_texture
 */
color_chooser (x:real, y:real, w:real, h:real, action:tree );

/**
 * @}
 */

