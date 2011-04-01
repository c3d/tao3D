/**
 * @addtogroup Page Pages
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
new_page Filename:text -> 
    page Filename, do
        color "white"
        image 0, 0, filename

// Define a single thumbnail of a page. 
// The page_thumbnail is only creating a texture. It must be applied on a shape to show it. This is why we have a rectangle.
// The active rectangle allows the user to click on the thumbnail to go to the real page.
// x, y: position (-0.5 to 0.5, relative to window size)
// s: size (relative to window size)
thumb X:real, Y:real, S:real, N:text ->
    color "white"
    page_thumbnail S, 0.02, N
    active_widget
        rectangle window_width * X, window_height * Y, window_width * S, window_height * S
        on_click 
            goto_page N

// Create a page with 4 thumbnails
page "Images", do
    thumb -0.25,  0.25, 0.4, "tortue.jpg"
    thumb  0.25,  0.25, 0.4, "papillon2.jpg"
    thumb -0.25, -0.25, 0.4, "coccinelle.jpg"
    thumb  0.25, -0.25, 0.4, "perroquet.jpg"

// Create one page per image
new_page with files "*.jpg"

@endcode
 * 
 * @{
 */

/**
 * Creates a new page or make page current.
 *
 * Creates a new page in the document with the given @p name, or add to the
 * content of a previously created page. The body contents will be evaluated
 * only when the page is being displayed. The order of pages in the document
 * is the order of page creation.
 *
 * @returns The name of the previous page.
 */
page (name:text, body:tree);

/**
 * Goes to the given page.
 *
 * The page is identified by its name.
 *
 * @returns The name of the previously shown page.
 */
goto_page (p:text);

/**
 * Sets the page time to use when printing.
 *
 * As pages can change with the time, the user can set the time used when
 * printing this page.
 *
 * @returns Previous time used for printing
 */
page_print_time (t:real);


/**
 * Links another page to the current page.
 *
 * Add a key binding to go from the current page to the specified page.
 * The page change is triggered when @p key is pressed.
 *
 * @returns The name of the page previously linked to this key.
 */
page_link (key:text, name:text);

/**
 *  Returns the name of the nth page.
 *
 * @p n is the page index. First page has index 1. If the page does not exist
 * an empty string is returned.
 */
page_name (n:integer);

/**
 * Generates a page thumbnail as a texture.
 *
 * @p p is the name of the page to render. The mini page is scaled by a
 * factor @p s. It is refreshed every @p i seconds.
 */
page_thumbnail (s:real, i:real, p:text);

/**
 * Returns the number of pages in the current document.
 */
page_count ();

/**
 * Returns the height of the current page.
 *
 * The page height is in pixels.
 * @todo check unit (pixels)
 */
page_height ();

/**
 * Returns the width of the current page.
 *
 * The page width is in pixels.
 * @todo check unit (pixels)
 */
page_width ();

/**
 * Returns the name of the current page.
 */
page_label ();

/**
 * Returns the number of the current page.
 */
page_number ();

/** @} */
