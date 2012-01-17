/**
 * @~english
 * @taomoduledescription{Carousel, 3D Carousel}
 *
 * 3D Carousel.\n
 *
 * This module allows to add a 3D and dynamic carousel in your presentations.
 *
 * For instance, the following code create a 3D carousel with some geometric shapes.
 *
 * @include carousel.ddd
 *
 * Here is a screen capture of this document:
 * @image html "carousel.png" "Output from carousel.ddd"
 *
 * @endtaomoduledescription{Carousel}
 *
 * @~french
 * @taomoduledescription{Carousel, Carrousel 3D}
 *
 * Carrousel 3D.\n
 *
 * Ce module permet d'ajouter un carousel dynamique en 3D dans vos présentations.
 *
 * Par exemple, le code suivant permet de réaliser un carrousel 3D avec des formes géométriques.
 *
 * @include carousel.ddd
 *
 * Ci-dessous, une capture d'écran issue du document précédent:
 * @image html "carousel.png" "Output from carousel.ddd"
 *
 * @endtaomoduledescription{Carousel}
 * @~
 * @{
 */

/**
 * @~english
 * Create an empty 3D carousel.\n
 *
 * Define an empty carousel with a size of (@p w, @p d)
 * and a number of slots of @p n.
 *
 * @param n Number of slots.
 * @param w Width of the carousel.
 * @param d Depth of the carousel.
 *
 * @~french
 * Créer un carrousel 3D vide.
 *
 * Définit un carrousel vide de taille (@p w, @p d)
 * avec un nombre total de positions @p n.
 *
 * @param n Nombre total de positions.
 * @param w Largeur du carrousel.
 * @param d Profondeur du carrousel.
 */
carousel(n:integer, w:integer, d:integer);

/**
 * @~english
 * Add an item to the carousel.
 *
 * Set an item to the slot @p n of the carousel.
@code
// Set an item to the slot 1
carousel_item 1 -> image 0, 0, 1, 1, "flowers.png"
@endcode
 *
 * @~french
 * Ajoute un élément au carrousel.
 *
 * Intégre un élément à la position @p n du carrousel.
@code
// Ajoute un élément à la position 1 du carousel
carousel_item 1 -> image 0, 0, 1, 1, "flowers.png"
@endcode
 */
carousel_item(n:integer);

/**
 * @~english
 * Add mouse control.
 *
 * Allows to control the carousel with the mouse.
@code
carousel 2, 400, 800
carousel_hand
@endcode
 *
 * @~french
 * Ajoute la souris comme contrôle.
 *
 * Permer d'utiliser la souris pour contrôler les mouvements du carrousel.
@code
carousel 2, 400, 800
carousel_hand
@endcode
 */
carousel_hand();

/**
 * @~english
 * Add automatic rotation.
 *
 * Allows to add an automatic rotation on the carousel at the speed @p s.
@code
carousel_auto_rotate page_time * 0.4
@endcode
 *
 * @~french
 * Ajoute une rotation automatique.
 *
 * Permet de définir une rotation automatique de vitesse @p s au carrousel.
@code
carousel_auto_rotate page_time * 0.4
@endcode
 */
carousel_auto_rotate(s:real);


/**
 * @}
 */
