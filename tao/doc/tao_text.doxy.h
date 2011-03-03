/**
 * @addtogroup Text Text and fonts
 * @ingroup TaoBuiltins
 *
 * Displays formatted text.
 *
 * This group has primitives to:
 *   @li Create text boxes,
 *   @li Layout text in a text box and select justification,
 *   @li Control fonts and text attributes (style, weight, underline...).
 *
 * @par About fonts and font parameters
 *
 * Tao Presentations relies on the QFont class from the Qt library to display
 * text. Therefore, the terms that are used here to describe fonts and
 * fonts attributes are the same that the ones used by Qt. You may want to
 * refer to the
 * <a href="http://doc.qt.nokia.com/latest/qfont.html>Qt documentation</a>
 * for more information.
 *
 * Example:
 @verbatim
// text.ddd


 @endverbatim
 *
 * @image html text.png "Displaying text (text.ddd)"
 *
 * @bug [#325]
 * On MacOSX, some fonts cannot be rendered, due to
 * <a href="http://bugreports.qt.nokia.com/browse/QTBUG-11145">QTBUG-11145</a>.
 * The correction of this bug expected in QT 4.8.
 * @bug [#325] On MacOSX, font styles other than "Normal", "Bold", "Italic" and
 * "Bold Italic" cannot be selected. This is caused by
 * <a href="http://bugreports.qt.nokia.com/browse/QTBUG-13518">QTBUG-13518</a>.
 *
 * @{
 */

/**
 * Draws a text box.
 *
 * The text box is centered at @a x, @a y. Th width is @a w and
 * the height is @a h. @a contents is a block of code in which you describe
 * the contents of the box.
 */
tree text_box(real x, real y, real w, real h, code contents);

/**
 * @todo explain
 */
tree text_flow(text name);

/**
 * Inserts text into a text box.
 */
tree text(text t);

/**
 * Sets the current font.
 *
 * The font parameters are given as a comma separated list of arguments, in
 * any order. Arguments are processed in the order they appear; when
 * conflicting arguments are given, the last one takes precedence.
 *
 * For instance you may write the following expression to select
 * font family @a Ubuntu in 24 point size and bold weight:
@verbatim
font "Ubuntu", 24, bold
@endverbatim
 * The following arguments are recognized:
 * <ul>
 *   <li> @b "FontFamily" or @b "FontFamily/Style". Selects the font
 *       family (such as: Times, Arial...), and, optionally, the font style
 *       among those that are available for this font family. When a style is
 *       specified, it must be separated form the family name by a forward
 *       slash. Example styles are: "Light", "Light Italic", "Bold", "Oblique",
 *       "Demi".
 *   <li> @a number. Sets the font size, in points.
 *   <li> @b plain, @b default, @b normal. Resets all font attributes.
 *       See @ref plain.
 *   <li> Font style arguments:
 *   <ul>
 *     <li> @b roman, @b no_italic. Resets the font style. Same as @ref roman.
 *     <li> @b italic. Sets the style of the font to italic.
 *          Same as @ref italic.
 *     <li> @b oblique. Sets the style of the font to oblique.
 *         Same as @ref oblique.
 *   </ul>
 *   <li> Font weight (see also @ref weight):
 *   <ul>
 *     <li> @b light. Same as @ref light.
 *     <li> @b regular, @b no_bold. Same as @ref regular.
 *     <li> @b demibold. Sets the weight of the font an intermediate value
 *         between normal and bold.
 *     <li> @b bold. Same as @ref bold.
 *     <li> @b black. Sets the weight of the font to an intermediate value
 *          between bold and the maximum font weight.
 *   </ul>
 *   <li> Capitalization:
 *   <ul>
 *     <li> @b mixed_case, @b normal_case. Resets capitalization mode to
 *          the normal, mixed case.
 *     <li> @b uppercase. Render text in upper case type.
 *     <li> @b lowercase. Render text in lower case type.
 *     <li> @b small_caps. Render text in small-caps type.
 *     <li> @b capitalized. Render the first character of each word as an
 *       uppercase character.
 *   </ul>
 *   <li> Glyph stretch factor (see also @ref stretch):
 *   <ul>
 *     <li> @b ultra_condensed
 *     <li> @b extra_condensed
 *     <li> @b condensed
 *     <li> @b semi_condensed
 *     <li> @b unstreched, @b no_stretch
 *     <li> @b semi_expanded
 *     <li> @b expanded
 *     <li> @b extra_expanded
 *     <li> @b ultra_expanded
 *   </ul>
 *   <li> @b underline, @b underlined. Sets underline on. See @ref underline.
 *   <li> @b overline. Sets overline on. See @ref overline.
 *   <li> @b strike_out. Sets strikeout on. See @ref strikeout.
 *   <li> @b kerning. Sets font kerning on.
 *   <li> @b no_kerning. Sets font kerning off.
 *   <li> @b size @a s. Sets size to @a s. See @ref font_size.
 *   <li> @b slant @a s. Sets slant to @a s. See @ref slant.
 *   <li> @b weight @a w. Sets weight to @a w. See @ref weight.
 *   <li> @b stretch @a s. Sets character stretch factor to @a s.
 *       See @ref stretch.
 * </ul>
 *
 * @note You cannot provide the family string or the size in variables. The
 * following code, for instance, is invalid and will result in an
 * <em>Unexpected font keyword</em> error:
@verbatim
f := "Times"
font f, 18, bold
@endverbatim
 * Instead, you should use the specific primitives, as follows:
@verbatim
f := "Times"
font f
font_size 18
bold
@endverbatim
 *
 * When an exact match for the requested font cannot be found, the program
 * will select the closest match or possibly the application's default font.
 *
 */
tree font(code desc);

/**
 * Sets the current font.
 *
 * @a name is the font family (for instance, "Times") and optionally the font
 * style after a slash ("Times/Bold", "Futura/Medium").
 */
tree font(text name);

/**
 * Sets the current font size.
 *
 * @a size is in points (pt).
 */
tree font_size(integer size);

/**
 * Resets all the font attributes of the current font.
 *
 * Same as calling @ref roman, @ref regular, @ref underline 0,
 * @ref stretch 1.0, @ref overline 0, and @ref strikeout 0.
 */
tree plain();

/**
 * Sets the style of the current font.
 *
 * Valid values are 0 for roman, 1 for italic and 2 for oblique.
 *
 * @todo Check valid values are indeed 0, 1, and 2.
 * @bug In reality, takes a real
 * @todo Remove? (and also slant parameter in @ref font)
 */
tree slant(integer s);


/**
 * Sets the style of the current font to normal.
 *
 * Cancels italic or oblique style.
 */
tree roman();

/**
 * Sets the style of the current font to italic.
 */
tree italic();

/**
 * Sets the style of the current font to oblique.
 *
 * Oblique glyphs have an italic appearance, but are typically based
 * on the unstyled glyphs, which are slanted.
 */
tree oblique();

/**
 * Sets the weight of the current font.
 *
 * @a w should be an integer value between 0 and 99. 0 is ultralight, 99 is
 * extremely black. The following predefined values are used by
 * the @ref font function:
 *   @li @b light: 25
 *   @li @b regular, no_bold: 50
 *   @li @b demi_bold: 63
 *   @li @b bold: 75
 *   @li @b black: 87
 *
 * @bug type should be integer
 */
tree weight(real w);


/**
 * Sets the weight of the current font to normal.
 *
 * @see weight
 */
tree regular();

/**
 * Sets the weight of the current font to light.
 *
 * @see weight
 */
tree light();

/**
 * Sets the weight of the current font to bold.
 *
 * @see weight
 */
tree bold();

/**
 * Enables or disables underline.
 *
 * Set @a w to 0 to disable the attribute, or to any positive value to enable
 * it.
 * @todo take boolean instead of real?
 */
tree underline(real w);

/**
 * Enables or disables overline.
 *
 * Set @a w to 0 to disable the attribute, or to any positive value to enable
 * it.
 * @todo take boolean instead of real?
 */
tree overline(real w);

/**
 * Enables or disables strikeout.
 *
 * Set @a w to 0 to disable the attribute, or to any positive value to enable
 * it.
 * @todo take boolean instead of real?
 */
tree strikeout(real w);

/**
 * Sets the stretch factor for the current font.
 *
 * For example, setting @a s to 1.5 results in all glyphs in the font
 * being 1.5 times wider. The following predefined values are used by
 * the @ref font function:
 *   @li @b ultra_condensed: 0.5
 *   @li @b extra_condensed: 0.62
 *   @li @b condensed: 0.75
 *   @li @b semi_condensed: 0.87
 *   @li @b unstretched, @b no_stretch: 1.0
 *   @li @b semi_expanded: 1.12
 *   @li @b expanded: 1.25
 *   @li @b extra_expanded: 1.5
 *   @li @b ultra_expanded: 2.0
 */
tree stretch(real s);


/**
 * @}
 */
