/**
 * @addtogroup Text Text layout and fonts
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
 * fonts attributes are the same as those used by Qt. You may want to
 * refer to the
 * <a href="http://doc.qt.nokia.com/latest/qfont.html">Qt documentation</a>
 * for more information.
 *
 * Example (<a href="examples/fonts.ddd">fonts.ddd</a>):
 * @include fonts.ddd
 *
 * Here is a screen capture:
 * @image html fonts.png "Changing fonts and font attributes (fonts.ddd)"
 *
 * @par Text layout
 *
 * You can control the horizontal and vertical distribution of text with two
 * powerful primitives: @ref align and @ref vertical_align.
 *
 * Example (<a href="examples/justification.ddd">justification.ddd</a>):
 * @include justification.ddd
 *
 * @image html justification.png "Various text justifications (justification.ddd)"
 *
 * @bug [#325] On MacOSX, some fonts cannot be rendered, due to
 * <a href="http://bugreports.qt.nokia.com/browse/QTBUG-11145">QTBUG-11145</a>.
 *      The correction of this bug expected in QT 4.8.
 * @bug [#325] On MacOSX, font styles other than "Normal", "Bold", "Italic" and
 *     "Bold Italic" cannot be selected. This is caused by
 * <a href="http://bugreports.qt.nokia.com/browse/QTBUG-13518">QTBUG-13518</a>.
 *
 * @todo font_scaling not documented - I don't understand its purpose
 * @todo The following primitives are not documented yet: paragraph_space,
 *       character_space (should it be called word_space ?), line_break,
 *       sentence_break, paragraph_break, column_break, page_break.
 * @todo Document enable_glyph_cache?
 * @{
 */

/**
 * Draws a text box.
 *
 * The text box is centered at @p x, @p y. Th width is @p w and
 * the height is @p h. @p contents is a block of code in which you describe
 * the content of the box.
 */
text_box(x:real, y:real, w:real, h:real, contents:tree);

/**
 * Sets the name of the current text flow.
 *
 * Use this function to name a text flow, either before creating it (with
 * @ref text_box) or before accessing it (with @ref text_overflow).
 */
text_flow(name:text);

/**
 * Creates a text box automatically filled by text from a text flow.
 * This function is useful to display text in several separate text boxes.
 * The text to render has to be declared inside a single text_box, after
 * the flow name has been set. Then, you call @ref text_overflow to create
 * additional text boxes that will be automatically filled with the text
 * that could not make it into the previous boxes.
 *
 * Here is the typical use:
@code
// Name the text flow we are going to create
text_flow "First"
text_box -100, 0, 200, 400,
    // Use the text box normally
    text "Some long text"
// Then, later:
text_flow "First"               // Select text flow
text_overflow 100, 0, 200, 400  // Continuation of 1st text box
@endcode
 *
 * @bug [#794] Empty text_overflow even though there is enough text to display
 */
text_overflow(x:real, y:real, w:real, h:real);

/**
 * Inserts text into a text box.
 */
text(t:text);

/**
 * Sets the current font.
 *
 * The font parameters are given as a comma separated list of arguments, in
 * any order. Arguments are processed in the order they appear; when
 * conflicting arguments are given, the last one takes precedence.
 *
 * For instance you may write the following expression to select
 * font family @p Ubuntu in 24 point size and bold weight:
@code
font "Ubuntu", 24, bold
@endcode
 * The following arguments are recognized:
 * <ul>
 *   <li> @b "FontFamily" or @b "FontFamily/Style". Selects the font
 *       family (such as: Times, Arial...), and, optionally, the font style
 *       among those that are available for this font family. When a style is
 *       specified, it must be separated form the family name by a forward
 *       slash. Example styles are: "Light", "Light Italic", "Bold", "Oblique",
 *       "Demi".
 *   <li> @p number. Sets the font size, in points.
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
 *   <li> @b size @p s. Sets size to @p s. See @ref font_size.
 *   <li> @b slant @p s. Sets slant to @p s. See @ref slant.
 *   <li> @b weight @p w. Sets weight to @p w. See @ref weight.
 *   <li> @b stretch @p s. Sets character stretch factor to @p s.
 *       See @ref stretch.
 * </ul>
 *
 * @note You cannot provide the family string or the size in variables. The
 * following code, for instance, is invalid and will result in an
 * <em>Unexpected font keyword</em> error:
@code
f := "Times"
font f, 18, bold
@endcode
 * Instead, you should use the specific primitives, as follows:
@code
f := "Times"
font f
font_size 18
bold
@endcode
 *
 * When an exact match for the requested font cannot be found, the program
 * will select the closest match or possibly the application's default font.
 *
 */
font(desc:tree);

/**
 * Sets the current font.
 *
 * @p name is the font family (for instance, "Times") and optionally the font
 * style after a slash ("Times/Bold", "Futura/Medium").
 */
font(name:text);

/**
 * Sets the current font size.
 *
 * @p size is in points (pt).
 */
font_size(size:real);

/**
 * Resets all the font attributes of the current font.
 *
 * Same as calling @ref roman, @ref regular, @ref underline 0,
 * @ref stretch 1.0, @ref overline 0, and @ref strikeout 0.
 */
plain();

/**
 * Sets the style of the current font.
 *
 * Valid values are 0 for roman, 1 for italic and 2 for oblique.
 *
 * - normal	0	Normal glyphs used in unstyled text.
 * - italic	1	Italic glyphs that are specifically designed for the purpose of representing italicized text.
 * - oblique	2	Glyphs with an italic appearance that are typically based on the unstyled glyphs, but are not fine-tuned for the purpose of representing italicized text.
 */
slant(s:real);


/**
 * Sets the style of the current font to normal.
 *
 * Cancels italic or oblique style.
 */
roman();

/**
 * Sets the style of the current font to italic.
 */
italic();

/**
 * Sets the style of the current font to oblique.
 *
 * Oblique glyphs have an italic appearance, but are typically based
 * on the unstyled glyphs, which are slanted.
 */
oblique();

/**
 * Sets the weight of the current font.
 *
 * @p w should be an integer value between 0 and 99. 0 is ultralight, 99 is
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
weight(w:real);


/**
 * Sets the weight of the current font to normal.
 *
 * @see weight
 */
regular();

/**
 * Sets the weight of the current font to light.
 *
 * @see weight
 */
light();

/**
 * Sets the weight of the current font to bold.
 *
 * @see weight
 */
bold();

/**
 * Enables or disables underline.
 *
 * Set @p w to 0 to disable the attribute, or to any positive value to enable
 * it.
 */
underline(w:real);

/**
 * Enables or disables overline.
 *
 * Set @p w to 0 to disable the attribute, or to any positive value to enable
 * it.
 */
overline(w:real);

/**
 * Enables or disables strikeout.
 *
 * Set @p w to 0 to disable the attribute, or to any positive value to enable
 * it.
 */
strikeout(w:real);

/**
 * Sets the stretch factor for the current font.
 *
 * For example, setting @p s to 1.5 results in all glyphs in the font
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
stretch(s:real);

/**
 * Sets parameters for horizontal text layout.
 *
 * This primitive controls how words and glyphs are positioned horizontally
 * to form a line of text in the current text box.
 * The four parameters @p center, @p justify, @p spread
 * and @p full_justify may take any value between 0 and 1 (inclusive). Some
 * combinations have a usual meaning; see the functions listed below
 * for details.
 *
 * Default alignment is equivalent to <tt> align 0, 0, 0, 0</tt>
 * @see align_left, align_right, align_center, align_justify,
 * align_right_justify, align_full_justify, align_spread, align_full_spread.
 */
align(center:real, justify:real, spread:real, full_justify:real);

/**
 * Sets parameters for horizontal text layout.
 *
 * This is equivalent to @code align center, justify, spread, 0 @endcode
 * @see align(center:real, justify:real, spread:real, full_justify:real);
 */
align(center:real, justify:real, spread:real);

/**
 * Sets parameters for horizontal text layout.
 *
 * This is equivalent to @code align center, justify, 0, 0 @endcode
 * @see align(center:real, justify:real, spread:real, full_justify:real);
 */
align(center:real, justify:real);

/**
 * Sets parameters for horizontal text layout.
 *
 * This is equivalent to @code align center, 0, 0, 0 @endcode
 * @see align(center:real, justify:real, spread:real, full_justify:real);
 */
align(center:real);

/**
 * Align text to the left of the text box.
 * Synonym for @ref align 0, 0, 0, 0.
 */
align_left();

/**
 * Align text to the right of the text box.
 * Synonym for @ref align 1, 0, 0, 0.
 */
align_right();

/**
 * Center text horizontally in the text box.
 * Synonym for @ref align 0.5, 0, 0, 0.
 */
align_center();

/**
 * Justify text horizontally in the text box.
 * Text is aligned both to the left and to the right of the text box,
 * except for the last line of each paragraph, which is aligned to the left.
 * Justification is obtained by inserting extra space between words, and
 * without changing the space between characters.
 * Synonym for @ref align 0, 1, 0, 0.
 */
align_justify();

/**
 * Justify text horizontally in the text box.
 * Synonym for @ref align_justify.
 */
align_left_justify();

/**
 * Justify text horizontally in the text box.
 * Text is aligned both to the left and to the right of the text box,
 * except for the last line of each paragraph, which is aligned to the right.
 * Justification is obtained by inserting extra space between words, and
 * without changing the space between characters.
 * Synonym for @ref align 1, 1, 0, 0.
 */
align_right_justify();

/**
 * Justify text horizontally in the text box.
 * Text is aligned both to the left and to the right of the text box,
 * including the last line of each paragraph, even if this line is too short
 * to fit nicely in the horizontal space. In the extreme case where the last
 * line contains a single word, this word is aligned to the left.
 * Justification is obtained by inserting extra space between words, and
 * without changing the space between characters.
 * Synonym for @ref align 0, 1, 0, 1.
 */
align_full_justify();

/**
 * Justify characters horizontally in the text box.
 * Similar to @ref align_justify, except that extra space in each line is
 * distributed evenly between characters, not just between words.
 * The last line of each paragraph is rendered aligned the left.
 * Synonym for @ref align 0, 1, 1, 0.
 */
align_spread();

/**
 * Justify characters horizontally in the text box.
 * Like @ref align_spread, but all lines are fully justified (including the
 * last line of each paragraph).
 * Synonym for @ref align 0, 1, 1, 1.
 */
align_full_spread();

/**
 * Sets parameters for vertical text layout.
 *
 * This primitive controls how lines of text are positioned vertically in the
 * current text box. The four parameters @p center, @p justify, @p spread
 * and @p full_justify may take any value between 0 and 1 (inclusive). Some
 * combinations have a usual meaning; see the functions listed below
 * for details.
 *
 * This function is to be called once per text box, before any text is written.
 * Otherwise, unexpected results may occur such as overlapping text or
 * otherwise incorrect layout.
 *
 * The default vertical text layout is vertical_align_top.
 *
 * @see vertical_align_top, vertical_align_bottom, vertical_align_center.
 */
vertical_align(center:real, justify:real, spread:real, full_justify:real);

/**
 * Sets parameters for vertical text layout.
 *
 * This is equivalent to @code vertical_align center, justify, spread, 0 @endcode
 * @see vertical_align(center:real, justify:real, spread:real, full_justify:real);
 */
vertical_align(center:real, justify:real, spread:real);
 
/**
 * Sets parameters for vertical text layout.
 *
 * This is equivalent to @code vertical_align center, justify, 0, 0 @endcode
 * @see vertical_align(center:real, justify:real, spread:real, full_justify:real);
 */
vertical_align(center:real, justify:real);
 
/**
 * Sets parameters for vertical text layout.
 *
 * This is equivalent to @code vertical_align center, 0, 0, 0 @endcode
 * @see vertical_align(center:real, justify:real, spread:real, full_justify:real);
 */
vertical_align(center:real);
 

/**
 * Flush all lines of text to the top of the text box.
 * If there are not enough lines of text to fill the box vertically, empty
 * space is at the bottom of the box.
 * Synonym for @ref vertical_align 0, 0, 0, 0.
 */
vertical_align_top();

/**
 * Flush all lines of text to the bottom of the text box.
 * If there are not enough lines of text to fill the box vertically, empty
 * space is at the top of the box.
 * Synonym for @ref vertical_align 1, 0, 0, 0.
 */
vertical_align_bottom();


/**
 * Center lines of text vertically within the text box.
 * If there are not enough lines of text to fill the ox vertically, empty
 * space is equally distributed between top and bottom of the box.
 * Synonym for @ref vertical_align 0.5, 0, 0, 0.
 */
vertical_align_center();

/**
 * Justify lines of text vertically within the text box.
 *
 * Synonym for @ref vertical_align 0, 1, 0, 1.
 */
vertical_align_justify();

/**
 * Spread lines of text vertically within the text box.
 * 
 * Space is equally distributed between lines and top and bottom of the box.
 * Synonym for @ref vertical_align 0, 1, 1, 1.
 */
vertical_align_spread();

/**
 * Sets the word spacing factor.
 *
 * Give a @p factor value larger than 1.0 to increase the space between words,
 * or a value smaller than 1.0 to reduce it.
 */
word_spacing(factor:real);

/**
 * Sets the line spacing factor.
 *
 * Give a @p factor value larger that 1.0 to increase the space between lines,
 * or a value smaller than 1.0 to reduce it.
 */
line_spacing(factor:real);

/**
 * Sets the left and right margins.
 *
 * The margin sizes are in pixels.
 * @bug [#793] The margins are ignored for first line of the first paragraph.
 */
margins(left:real, right:real);

/**
 * Sets the top and bottom margins.
 *
 * The margin sizes are in pixels.
 */
vertical_margins(top:real, bottom:real);

/**
 * @}
 */
