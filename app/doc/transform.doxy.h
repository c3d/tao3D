// *****************************************************************************
// transform.doxy.h                                                Tao3D project
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
// (C) 2011-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2013,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011, Soulisse Baptiste <soulisse@polytech.unice.fr>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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
 * @addtogroup transforms Geometric transforms
 * @ingroup Graphics
 *
 * This group deals with transformation of the space.
 * Transformations are based on OpenGL capacity.
 * Transformations are
 * - rotation, expressed in degrees. Rotations follows the right-hand rule, so if the rotation axis
 *      points toward the user, the rotation will be counterclockwise.
 * - translation, expressed in pixels
 * - scaling, expressed with a scaling factor
 *
 * Those transformations are applied to the whole system : origin and axes are modified by transformation.
 * So the user should focus on rotation and translation order because
 * a translation and then a rotation do not give the same location as rotation
 * and then translation, as shown in the following image and sample.
 * @image html rotation_translation.png
 *
 * Examples: <a href="examples/transform_samples.ddd">transform_samples.ddd</a>
 *
 * @~french
 * @addtogroup transforms Transformations géométriques
 * @ingroup Graphics
 *
 * Ce groupe contient des primitives permettant d'effectuer des transformations
 * géométriques :
 * - rotations, en degrés. Les rotatins suivent la règle de la main droite,
 * donc si l'axe pointe vers l'utilisateur (axe Z), la rotation est dans le
 * sens anti-horaire.
 * - rotations, en pixels.
 * - redimensionnement, exprimé sous forme d'un facteur d'échelle.
 *
 * Ces transformattions s'applique à la scène 3D complète. Donc, attention à
 * l'ordre des opérations. Une translation, suivie d'une rotation n'a pas le
 * même effet qu'une rotation suivie d'un translation, comme le montre
 * l'exemple ci-dessous.
 * @image html rotation_translation.png
 *
 * Exemples : <a href="examples/transform_samples.ddd">transform_samples.ddd</a>
 *
 * @~
 * @{
 */

/**
 * @~english
 * Scales along the three axes.
 * @~french
 * Redimensionne la scène selon les trois axes.
 * @~
 * @image html scale.png
 */
scale (x:real, y:real, z:real);

/**
 * @~english
 * Scales along the three axes with the same value.
 * Equivalent to:
 * @~french
 * Redimensionne la scène selon les trois axes avec la même valeur.
 * Équivalent à :
 * @~
 * @code scale xyz, xyz, xyz @endcode
 * @image html scale.png
 */
scale (xyz:real);

/**
 * @~english
 * Scales along the X axis.
 * Equivalent to:
 * @~french
 * Redimensionne la scène selon l'axe X.
 * Équivalent à :
 * @~
 * @code scale x, 1, 1 @endcode
 */
scalex (x:real);

/**
 * @copydoc scalex
 */
scale_x (x:real);

/**
 * @~english
 * Scales along the Y axis.
 * Equivalent to:
 * @~french
 * Redimensionne la scène selon l'axe Y.
 * Équivalent à :
 * @~
 * @code scale 1, y, 1 @endcode
 */
scaley (y:real);

/**
 * @copydoc scaley
 */
scale_y (y:real);

/**
 * @~english
 * Scales along the Z axis.
 * Equivalent to:
 * @~french
 * Redimensionne la scène selon l'axe Z.
 * Équivalent à :
 * @~
 * @code scale 1, 1, z @endcode
 */
scalez (z:real);

/**
 * @copydoc scalez
 */
scale_z (z:real);

/**
 * @~english
 * Rotates along an arbitrary axis.
 *
 * Rotation of @p ra degrees around the axis determined by the
 * (@p rx, @p ry, @p rz) vector.
 *
 * Rotations follows the right-hand rule, so if the vector
 * (@p rx, @p ry, @p rz)
 * points toward the user, the rotation will be counterclockwise.
 *
 * @~french
 * Rotation autour d'un axe arbitraire.
 *
 * Rotation de @p ra degrés autour de l'axe déterminé par le vecteur
 * (@p rx, @p ry, @p rz).
 *
 * Les rotations suivent la règle de la main droite, donc si le vecteur
 * (@p rx, @p ry, @p rz) pointe vers l'utilisateur, la rotation se fait dans
 * le sens anti-horaire.
 *
 * @~
 * @image html rotation.png
 */
rotate (ra:real, rx:real, ry:real, rz:real);

/**
 * @~english
 * Rotates around the X axis.
 * Equivalent to:
 * @~french
 * Rotation autour de l'axe X.
 * Équivalent à :
 * @~
 * @code rotate rx, 1, 0, 0 @endcode
 */
rotatex (rx:real);

/**
 * @copydoc rotatex
 */
rotate_x (rx:real);

/**
 * @~english
 * Rotates around the Y axis.
 * Equivalent to:
 * @~french
 * Rotation autour de l'axe Y.
 * Équivalent à :
 * @~
 * @code rotate ry, 0, 1, 0 @endcode
 */
rotatey (ry:real);

/**
 * @copydoc rotatey
 */
rotate_y (ry:real);

/**
 * @~english
 * Rotates around the Z axis.
 * Equivalent to:
 * @~french
 * Rotation autour de l'axe Z.
 * Équivalent à :
 * @~
 * @code rotate rz, 0, 0, 1 @endcode
 */
rotatez (rz:real);

/**
 * @copydoc rotatez
 */
rotate_z (rz:real);

/**
 * @~english
 * Translates along a vector.
 * @~french
 * Translation d'un vecteur quelconque.
 * @~
 * @image html translation.png
 */
translate (x:real, y:real, z:real);

/**
 * @~english
 * Translates along the X axis.
 * Equivalent to:
 * @~french
 * Translation le long de l'axe X.
 * Équivalent à :
 * @~
 * @code translate x, 0, 0 @endcode
 */
translatex (x:real);

/**
 * @copydoc translatex
 */
translate_x (x:real);

/**
 * @~english
 * Translates along the Y axis.
 * Equivalent to:
 * @~french
 * Translation le long de l'axe Y.
 * Équivalent à :
 * @~
 * @code translate 0, y, 0 @endcode
 */
translatey (y:real);

/**
 * @copydoc translatey
 */
translate_y (y:real);

/**
 * @~english
 * Translates along the Z axis.
 * Equivalent to:
 * @~french
 * Translation le long de l'axe Z.
 * Équivalent à :
 * @~
 * @code translate 0, 0, z @endcode
 */
translatez (z:real);

/**
 * @copydoc translatez
 */
translate_z (z:real);

/**
 * @~english
 * Reset the geometric transform to original state.
 * @~french
 * Annule les transformations géométriques.
 */
reset_transform ();

/**
 * @~english
 * Returns the current model matrix 4x4 which convert from object space to
 * world space.
 * The default value is the identity matrix.
 * @return 16 real values, separated by commas.
 *
 * @~french
 * Renvoie la matrice modèle (4x4).
 * La valeur par défaut est la matrice identité.
 * @return 16 valeurs réelles, séparées par des virgules.
 */
tree model_matrix ();

/**
 * @~english
 * Define a clip plane
 * A clip plane selects which element should be drawn.
 * Defining a clip plane with arguments @a a, @a b, @a c and @a d will
 * eliminate points for which coordinates @a x, @a y and @a z
 * verify a*x+b*y+c*z+d>0.
 * Several clip planes (at least 6) can be defined simutaneously,
 * each one identified by the given @a planeID. Plane identifiers start at 0.
 *
 * @~french
 * Définit un plan de découpe définit les éléments qui doivent être tracés.
 * Un plan de découpe défini avec les arguments @a a, @a b, @a c et @a d
 * élimine les points de coordonnées x, y, z tels que a*x+b*y+c*z+d>0.
 * Plusieurs plans de découpe (au moins 6) peuvent être définis ensemble,
 * chacun identifié par son propre @a planeID. L'identifiant de plan
 * de découpe commence à 0.
 */
clip_plane(planeID:integer, a:real, b:real, c:real, d:real);


/** @} */
