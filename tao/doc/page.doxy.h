/**
 * @addtogroup page Pages
 * @ingroup TaoBuiltins
 * This group of primitives deals with the page attributes.
 * 
 * A document is made of one or more pages. Pages are primarily identified by a name.
 * Navigation between pages is not necessarily linear, it may be built by the document's author. A default linear navigation is automaticaly build.
 *
 * Currently the page is a A4 paper in portrait format.
 *
@code
// Define a page holding an image
new_page filename:text -> 
    page filename, do
        color "white"
        image 0, 0, filename

// Define a single thumbnail of a page. 
// The page_thumbnail is only creating a texture. It must be applied on a shape to show it. This is why we have a rectangle.
// The active rectangle allows the user to click on the thumbnail to go to the real page.
// x, y: position (-0.5 to 0.5, relative to window size)
// s: size (relative to window size)
thumb x:real, y:real, s:real, n:text ->
    color "white"
    page_thumbnail s, 0.02, n
    active_widget
        rectangle window_width * x, window_height * y, window_width * s, window_height * s
        on_click 
            goto_page n

// Create a page with 4 thumbnails
page "Images", do
    thumb -0.25,  0.25, 0.4, "tortue.jpg"
    thumb  0.25,  0.25, 0.4, "papillon2.jpg"
    thumb -0.25, -0.25, 0.4, "coccinelle.jpg"
    thumb  0.25, -0.25, 0.4, "perroquet.jpg"

// Create one page per image
new_page with list_files "*.jpg"

@endcode
 * 
 * @{
 */

/**
 * @fn page ( name, body)
 * @brief Create a page in a document.
 *
 *  Create a new page in the document with the given name, or add to the content of a previously created page. The body contents will be evaluated only when the page is being displayed. The order of pages in the document is the order of page creation.
 *
 * @param name [text] Name of the page. Any valid string can be used.
 * @param body [tree] Content of the page.
 * @returns The name of the previous page
 */
page ( name, body);

/**
 * @fn goto_page ( p)
 * @brief Directly go to the given page.
 *
 *  Directly go to the given page. Page is identified by its name given as the 'p' parameter.
 *
 * @param p [text] The name of the page to go to.
 * @returns The name of the previously shown page.
 */
goto_page ( p);

/**
 * @fn page_print_time ( t)
 * @brief Set the freeze time used when printing.
 *
 *  As pages can change with the time, the user can set the time used when printing this page.
 *
 * @param t [real] Time used to print the page.
 * @returns Previous time used for printing
 */
page_print_time ( t);


/**
 * @fn page_link ( key, name)
 * @brief Link other pages to the current page.
 *
 *  Add a key binding to this page to go to the given page.
 *
 * @param key [text] Key used to change pages
 * @param name [text] The name of the page to go to
 * @returns The name of the page previously linked to this key.
 */
page_link ( key, name);

/**
 * @fn page_name ( n)
 * @brief The name of the nth page.
 *
 *  Return the name of a page given its index. If the page does not exist an empty string is returned.
 *
 * @param n [integer] Index of page starting at 1. @c n is treated as an unsigned integer.
 * @returns The name of the nth page
 */
page_name ( n);

/**
 * @fn page_thumbnail ( s, i, p)
 * @brief Generate a page thumbnail as a texture.
 *
 *  The mini page is scaled and refreshed as specified in parameter.
 *
 * @param s [real] Scaling of the thumbnail
 * @param i [real] Refresh interval of the thumbnail
 * @param p [text] Page name
 */
page_thumbnail ( s, i, p);

/**
 * @fn page_count ()
 * @brief The number of page.
 *
 *  Return the number of pages in the current document.
 * @returns Return the number of pages in the current document.
 */
page_count ();

/**
 * @fn page_height ()
 * @brief Return the height of pages
 *
 *  Return the height of pages as a real.
 * @returns Return the height of pages
 */
page_height ();

/**
 * @fn page_label ()
 * @brief The current page name.
 *
 *  Return the current page name
 * @returns Return the label of the current page
 */
page_label ();

/**
 * @fn page_number ()
 * @brief The current page number
 *
 *  Return the number of the current page
 * @returns Return the number of the current page
 */
page_number ();

/**
 * @fn page_width ()
 * @brief Return the width of pages
 *
 *  Return the width of pages as a real.
 * @returns Return the width of pages
 */
page_width ();

/** @} */
