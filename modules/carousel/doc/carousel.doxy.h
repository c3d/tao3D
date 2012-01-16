/**
 * @taomoduledescription{Carousel, Carousel}
 *
 * 3D Carousel
 *
 * Carousel allows to add a 3D and dynamic carousel in your presentations.
 *
 * For instance, the following code create a carousel for your images.
 *
 * @include carousel.ddd
 *
 * Here is a screen capture of this document:
 * @image html "carousel.png" "Output from carousel.ddd"
 *
 * @endtaomoduledescription{Carousel}
 * @{
 */

/**
 * Create an empty 3D carousel.
 *
 * Define an empty carousel with a size of (@p w, @p d)
 * and a number of slots of @p n.
 *
 * @param n Number of slots.
 * @param w Width of the carousel.
 * @param d Depth of the carousel.
 *
 */
carousel(n:integer, w:integer, d:integer);

/**
 * Add an item to the carousel.
 *
 * Set an item to the slot @n of the carousel.
@code
// Set an item to the slot 1
carousel_item 1 -> image 0, 0, 1, 1, "flowers.png"
@endcode
 */
carousel_item(n:integer);

/**
 * Add mouse control.
 *
 * Allows to control the carousel with the mouse.
@code
carousel 2, 400, 800
carousel_hand
@endcode
 */
carousel_hand(n:integer);

/**
 * Add automatic rotation.
 *
 * Allows to add an automatic rotation on the carousel at the speed @p s.
@code
carousel_auto_rotate page_time * 0.4
@endcode
 */
carousel_auto_rotate(s:real);


/**
 * @}
 */
