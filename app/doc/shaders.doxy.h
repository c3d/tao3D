// *****************************************************************************
// shaders.doxy.h                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************
/**
 * @~english
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
@code
// Writing shader code inline
shader_program
    fragment_shader <<
      // Write GLSL fragment shader code here
    >>
// Loading shader code from a file
shader_program
    fragment_shader_file "my_shader.fs"
@endcode
 *
 * @par Example
 *
 * The following code (<a href="examples/shaders.ddd">shaders.ddd</a>) uses
 * a vertex shader to transform a sphere and make it bumpy or wavy. Just
 * move the mouse pointer over the window and see the sphere distort.
 * @include shaders.ddd
 *
 * @image html shaders.png "Shaders demo: shaders.ddd"
 *
 * @~french
 * @addtogroup Shaders Shaders OpenGL
 * @ingroup Graphics
 *
 * Tao Presentions fournit des primitives pour créer et utiliser des shaders
 * OpenGL.
 * Un @a shader est un programme exécuté sur le GPU (processeur de la carte
 * graphique) à une étape bien définie du pipeline graphique.
 * Les primitives décrites ici permettent d'écrire des shaders dans le language
 * GLSL (<em> OpenGL Shading Language</em>). Pour plus de détails sur le
 * langage, vous pouvez faire référence à la
 * <a href="http://www.opengl.org/documentation/glsl/">
 * documentation OpenGL</a> (en anglais).
 *
 * Le code source GLSL peut être donné directement dans le document Tao
 * Presentations, ou bien lu depuis un fichier. Dans le premier cas, on utilise
 * en général les balises de texte << et >>, qui permettent d'écrire du texte
 * sur plusieurs lignes et ne posent pas de problème en cas d'utilisation de
 * guillemets ("). Si, par contre, vous chargez votre code source GLSL depuis
 * un fichier, notez que le chemin est relatif au dossier du document.
 * Par exemple :
@code
// Pour créer un fragment shader avec le code dans le document
shader_program
    fragment_shader <<
      // Code GLSL ici
    >>
// Pour créer un fragment shader depuis un fichier source
shader_program
    fragment_shader_file "mon_shader.fs"
@endcode
 *
 * @par Exemple
 *
 * Le code ci-dessous (<a href="examples/shaders.ddd">shaders.ddd</a>) utilise
 * un <em>vertex shader</em> pour transformer une sphère et la rendre bosselée
 * ou ondulée. Déplacez la souris au-dessus de la sphère et observez les
 * déformations provoquées par le shader.
 * @include shaders.ddd
 *
 * @image html shaders.png "Démonstration d'un vertex shader"
 *
 * @~
 * @{
 */

/**
 * @~english
 * Defines a new shader program and make it current.
 *
 * A shader program is a container for one or several shaders. @p c is a block
 * of code in which you may use @ref vertex_shader, @ref vertex_shader_file,
 * @ref fragment_shader, @ref fragment_shader_file, @ref geometry_shader, and/or
 * @ref geometry_shader_file to create individual shaders.
 * To disable the current shader program, use the following code:
@code
shader_program false
@endcode
 *
 * @attention No texture is activated by defaut. In order to use
 * texture coordinates in a shader without texture, it is necessary to
 * set an empty one just before with the following sample code.
 @code
 // Set a empty texture in order to activate the
 // texture coordinates on the current texture unit.
 texture ""
 @endcode
 *
 * @~french
 * Définit et active un programme de shaders.
 *
 * Un programme de shaders est formé d'un ou plusieurs shaders. @p c est un
 * bloc de code dans lequel vous pouvez utiliser @ref vertex_shader,
 * @ref vertex_shader_file, @ref fragment_shader, @ref fragment_shader_file,
 * @ref geometry_shader, et/ou @ref geometry_shader_file pour créer et
 * ajouter des shaders.
 * Pour désactiver le shader courant, utiliser le code suivant:
@code
shader_program false
@endcode
 *
 * @attention Aucune texture n'est activée par défaut. Si vous désirez utiliser
 * les coordonnées de texture dans un shader sans utiliser une texture
 * explicite, vous devez utiliser la texture vide juste avant d'activer le
 * shader :
 @code
 // Activation d'une texture vide pour que les coordonnées de texture
 // soient disponibles dans l'unité de texture courante.
 texture ""
 @endcode
 */
shader_program(c:code);

/**
 * @~english
 * Adds a new vertex shader to the current shader program.
 * The shader source is given in the @p src parameter.
 * @~french
 * Ajoute un nouveau vertex shader au programme de shaders courant.
 * Le code source du shader est donné par le paramètre @p src.
 */
vertex_shader(src:text);

/**
 * @~english
 * Loads and adds a new vertex shader to the current shader program.
 * The shader source is read from file @p f.
 * @~french
 * Charge le code source et ajoute un nouveau vertex shader au programme de
 * shaders courant.
 * Le code source est lu depuis le fichier @p f.
 */
vertex_shader_file(f:text);

/**
 * @~english
 * Adds a new fragment shader to the current shader program.
 * The shader source is given in the @p src parameter.
 * @~french
 * Ajoute un nouveau fragment shader au programme de shaders courant.
 * Le code source du shader est donné par le paramètre @p src.
 */
fragment_shader(src:text);

/**
 * @~english
 * Loads and adds a new fragment shader to the current shader program.
 * The shader source is read from file @p f.
 * @~french
 * Charge le code source et ajoute un nouveau fragment shader au programme de
 * shaders courant.
 * Le code source est lu depuis le fichier @p f.
 */
fragment_shader_file(f:text);

/**
 * @~english
 * Adds a new geometry shader to the current shader program.
 * The shader source is given in the @p src parameter.
 * @~french
 * Ajoute un nouveau geometry shader au programme de shaders courant.
 * Le code source du shader est donné par le paramètre @p src.
 */
geometry_shader(src:text);

/**
 * @~english
 * Loads and adds a new geometry shader to the current shader program.
 * The shader source is read from file @p f.
 * @~french
 * Charge le code source et ajoute un nouveau geometry shader au programme de
 * shaders courant.
 * Le code source est lu depuis le fichier @p f.
 */
geometry_shader_file(f:text);

/**
 * @~english
 * Gets the input type of the geometry shader.
 * The input type is one of the following values:
 * @~french
 * Renvoie le type d'entrée du geometry shader.
 * Le type est l'une des valeurs suivantes :
 * @~
 *  - GL_POINTS =  0
 *  - GL_LINES = 1
 *  - GL_TRIANGLES = 4
 *  - GL_LINES_ADJACENCY_EXT = 10
 *  - GL_TRIANGLES_ADJACENCY_EXT = 12
 */
integer geometry_shader_input();

/**
 * @~english
 * Sets the input type of the geometry shader.
 * @~french
 * Définit le type d'entrée du geometry shader.
 * @~
 * @see geometry_shader_input().
 */
geometry_shader_input(type:integer);

/**
 * @~english
 * Gets the output type of the geometry shader.
 * The input type is one of the following values:
 * @~french
 * Renvoie le type de sortie du geometry shader.
 * Le type est l'une des valeurs suivantes :
 * @~
 *  - GL_POINTS =  0
 *  - GL_LINE_STRIP = 3
 *  - GL_TRIANGLE_STRIP = 5
 */
integer geometry_shader_output();

/**
 * @~english
 * Sets the output type of the geometry shader.
 * @~french
 * Définit le type de sortie du geometry shader.
 * @~
 * @see geometry_shader_output().
 */
geometry_shader_output(type:integer);

/**
 * @~english
 * Gets the maximum number of output vertices of the geometry shader.
 * This is the maximum number of points any instance of the shader will emit.
 * The limit of ouput vertices depends on the implementation and on the number
 * of scalar values in the output type.
 * However, the total number of these values can not exceed 1024 (GLSL
 * specification).
 * @~french
 * Renvoie le nombre maximum de sommets en sortie du geometry shader.
 * Il s'agit du nombre maximal de points que n'importe quelle instance du
 * shader est susceptible d'emettre. La limite en termes de nombre de sommets
 * dépend de l'implémentation et du nombre de valeurs scalaires dans le type
 * de sortie du shader. Toutefois, le nombre maximal de valeurs ne peux
 * excéder 1024 d'aprè la spécification GLSL.
 */
integer geometry_shader_count();

/**
 * @~english
 * Sets the maximum number of output vertices of the geometry shader.
 * @~french
 * Définit le nombre maximum de sommets en sortie du geometry shader.
 * @~
 * @see geometry_shader_count()
 */
geometry_shader_count(n:integer);


/**
 * @~english
 * Sets uniform or attribute variables in the current shader program.
 *
 * This primitive looks for a named uniform or attribute variable in the
 * shaders contained in the current shader program. If the variable is
 * found, its value is set accordingly.
 *
 * For example:
@code
shader_set foo := 0.5 * sin time  // Sets variable foo
shader_set bar := (0.0; 1.0)      // Sets bar[0] to 0.0 and bar[1] to 1.0
@endcode
 * @~french
 * Définit une variable uniforme ou un attribut du programme de shaders
 * courant.
 *
 * Cette primitive recherche une variable uniforme ou un attribut dont le nom
 * est précisé. Si la variable est trouvée, sa valeur est mise à jour.
 *
 * Par example:
@code
shader_set foo := 0.5 * sin time  // Définit la valeur de la variable foo
shader_set bar := (0.0; 1.0)      // Définit bar[0] = 0.0 et bar[1] = 1.0
@endcode
 */
shader_set(c:code);

/**
 * @~english
 * Returns the errors and warnings for the current shader program.
 *
 * When warnings or errors occur during the compilation phase of
 * a shader, or during the link phases of the shaders into the
 * current shader program, the diagnostic message are stored and can
 * be later returned by this primitive.
 *
 * @~french
 * Renvoie les erreurs et les avertissements du programme de shaders courant.
 *
 * Lorsque des avertissements ou des erreurs surviennent durant la phase de
 * compilation ou d'édition de liens des shaders, les messages sont stockés
 * et peuvent ensuite être lus par cette primitive.
 */
text shader_log();

/**
 * @}
 */
