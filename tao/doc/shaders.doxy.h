/**
 * @addtogroup Shaders OpenGL shaders
 * @ingroup Graphics
 *
 * Tao Presentions provides primitives to work with OpenGL shaders.
 * A @a shader is a piece of code executed on the GPU at a specific stage of
 * the graphics pipeline. The Tao primitives described in this page support
 * shader programs written in the OpenGL Shading Language (GLSL). For more
 * information on the language, please refer to the
 * <a href="http://www.opengl.org/documentation/glsl/">GLSL reference
 * documentation</a>.
 *
 * GLSL source code can be given either inline, or read from a file. In the
 * former case, you will typically use the << and >> text delimiters, because
 * they allow multi-line text. In the latter case, please note that the file
 * path is relative to the current document directory.
 * For example:
 * @code
// Writing shader code inline
shader_program
    fragment_shader <<
      // Write GLSL fragment shader code here
    >>
// Loading shader code from a file
shader_program
    fragment_shader_file "my_shader.fs"
 * @endcode
 *
 * @par Example
 *
 * The following code (<a href="examples/shaders.ddd">shaders.ddd</a>) uses
 * a vertex shader to transform a sphere and make it bumpy or wavy. Just
 * move the mouse pointer over the window and see the sphere distort.
 * @include shaders.ddd
 *
 * @image html shaders.png "Shaders demo: shaders.ddd"
 * @{
 */

/**
 * Defines a new shader program and make it current.
 *
 * A shader program is a container for one or several shaders. @p c is a block
 * of code in which you may use @ref vertex_shader, @ref vertex_shader_file,
 * @ref fragment_shader, @ref fragment_shader_file, @ref geometry_shader, and/or
 * @ref geometry_shader_file to create individual shaders.
 */
boolean shader_program(c:code);

/**
 * Adds a new vertex shader to the current shader program.
 * The shader source is given in the @p src parameter.
 */
boolean vertex_shader(src:text);

/**
 * Loads and adds a new vertex shader to the current shader program.
 * The shader source is read from file @p f.
 */
boolean vertex_shader_file(f:text);

/**
 * Adds a new fragment shader to the current shader program.
 * The shader source is given in the @p src parameter.
 */
boolean fragment_shader(src:text);

/**
 * Loads and adds a new fragment shader to the current shader program.
 * The shader source is read from file @p f.
 */
boolean fragment_shader_file(f:text);

/**
 * Adds a new geometry shader to the current shader program.
 * The shader source is given in the @p src parameter.
 */
boolean geometry_shader(src:text);

/**
 * Loads and adds a new geometry shader to the current shader program.
 * The shader source is read from file @p f.
 */
boolean geometry_shader_file(f:text);

/**
 * Sets uniform or attribute variables in the current shader program.
 *
 * This primitive looks for a named uniform or attribute variable in the
 * shaders contained in the current shader program. If the variable is
 * found, its value is set accordingly.
 *
 * For example:
 * @code
shader_set foo := 0.5 * sin time  // Sets variable foo
shader_set bar := (0.0; 1.0)      // Sets bar[0] to 0.0 and bar[1] to 1.0
 * @endcode
 */
boolean shader_set(c:code);

/**
 * Returns the errors and warnings for the current shader program.
 *
 * When warnings or errors occur during the compilation phase of
 * a shader, or during the link phases of the shaders into the
 * current shader program, the diagnostic message are stored and can
 * be later returned by this primitive.
 */
text shader_log();

/**
 * @}
 */
