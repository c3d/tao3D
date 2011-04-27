/**
 * @defgroup Filters Filters
 * @ingroup Modules
 *
 * Apply filters on current texture.
 *
 * This module allows you to apply some basic filtering on your textures.
 *
 * For example, the following code will show how to detect outlines and add some iterative blur to it.
 * @include filters.ddd
 *
 * @image html filters.png "Output of filters.ddd"
 * @{
 */

/**
 * Apply a gaussian filter on current texture.
 * It can be used to add blur on an image for instance.
 *
 * @param iter maximum number of iteration of the filter.
 */
gaussian(integer iter);

/**
 * Apply a mean filter on the current texture.
 * It can be used to remove noise of an image for instance.
 *
 * @param iter maximum number of iteration of the filter.
 */
mean(integer iter);

/**
 * Apply a embross filter on the current texture.
 * It can be used to simulate 3D in an image for instance.
 *
 * @param iter maximum number of iteration of the filter.
 */
emboss(integer iter);

/**
 * Apply a sharpness filter on the current texture.
 * It can be used to highlight details of an image for instance.
 *
 * @param iter maximum number of iteration of the filter.
 */
sharpness(integer iter);

/**
 * Apply a laplacian filter on the current texture.
 * It can be used to detect oulines of an image for instance.
 *
 * @param iter maximum number of iteration of the filter.
 */
laplacian(integer iter);


/**
* @}
*/
