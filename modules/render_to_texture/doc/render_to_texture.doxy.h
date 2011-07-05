/**
 * @defgroup RenderToTexture Render to Texture
 * @ingroup Modules
 *
 * Make render to texture.
 *
 * This module allows you to create a new texture without doing a copy from the current one modified by a sample code.
 *
 * For instance, the following code apply an effect defined by a shader several times on the current texture.
 * However three functions are exatly identical.
 * @include render_to_texture.ddd
 *
 * @{
 *
 * @note To improve performance in Tao, it is recommended to render to files the result texture before to reload it in Tao.
 */

/**
 * The total number of iterations.
 *
 * This number is used to define an iterative render.
 *
 * @note There is a limit of 30 iterations.
 */
integer MAX_ITER = 0;

/**
 * Allows to render a scene to a texture resource instead of just the back buffer.
 * That texture resource can be used then to achieve numerous effects.
 *
 * @param contents defines code applied on the current texture.
 */
render_to_texture(contents:tree);

/**
 * Allows to make an iterative render of a scene to a texture
 * in order to apply a same effect several times.
 *
 * @param contents defines code applied on the current texture.
 *
 */
iterate_render(contents:tree);

/**
* @}
*/
