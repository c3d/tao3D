/**
 * @~english
 * @taomoduledescription{RegExp, Regular Expressions}
 *
 * Analyze text using regular expressions
 *
 * This module makes it easy to analyze chunks of text using regular
 * expressions (RE). It contains simple RE matching @see regexp_match,
 * RE search @see regexp_search, as well as more complicated forms of
 * RE-based parsing @see regexp_parse. The module supports RE capture,
 * i.e. the ability to extract information based on matches,
 * @see regexp_captured.
 *
 * The regular expression language being used is the Qt Regexp2
 * variant, documented here: http://qt-project.org/doc/qt-4.8/qregexp.html
 *
 * @endtaomoduledescription{RegExp}
 *
 * @~french
 * @taomoduledescription{RegExp, Analyse de texte basée sur des expressions régulières}
 *
 * Analyse de texte basée sur des expressions régulières 
 *
 * Ce module permet d'analyser des fragments de texte en utilisant des
 * expressions régulières (ER). Il permet de vérifier si un texte
 * correspond à une ER @see regexp_search, la recherche d'une ER dans
 * un texte @see regexp_search, mais aussi des analyses plus complexes
 * utilisant des ER @see regexp_parse. Le module permet la capture
 * d'éléments dans une ER, @see regexp_captured.
 *
 * Le langage d'expressions régulières utilisé est la variante Regexp2
 * de Qt, documentée ici: http://qt-project.org/doc/qt-4.8/qregexp.html
 *
 * @endtaomoduledescription{RegExp}
 *
 * @~
 * @{
 */


/**
 * @~english
 * Returns true if a given input matches a regular expression.
 * @param input The text to search
 * @param pattern The regular expression to search for in @ref input
 *
 * @~french
 * Vérifie si un texte contient une certaine expression régulière.
 * @param input Le texte d'entrée à explorer
 * @param pattern L'expression régulière à rerchercher dans @ref input.
 *
 */

regexp_match(text input, text pattern);

/**
 * @~english
 * Returns true if a given input matches a regular expression. This
 * form is infix, i.e. you write @code input matches pattern @encode.
 * 
 * @param input The text to search
 * @param pattern The regular expression to search for in @ref input
 *
 * @~french
 * Vérifie si un texte contient une certaine expression
 * régulière. Cette forme est infixe, c'est à dire qu'on écrit
 * @code input matches pattern @code.
 *
 * @param input Le texte d'entrée à explorer
 * @param pattern L'expression régulière à rerchercher dans @ref input.
 *
 */

matches(text input, text pattern);

/**
 * @~english
 * Returns the 0-based position of the given pattern in the input, or -1 if
 * the pattern is not found.
 * @param input The text to search
 * @param pattern The regular expression to search for in @ref input
 *
 * @~french
 * Retourne la position, démarrant à 0, de l'expression régulière dans
 * le texte donné, ou bien -1 si l'expression régulière n'est pas trouvée.
 * @param input Le texte d'entrée à explorer
 * @param pattern L'expression régulière à rerchercher dans @ref input.
 *
 */

regexp_match(text input, text pattern);

/**
 * @~english
 * Parse the given input using a number of regular expressions.
 *
 * The @a code parameter must be a block containing a sequence of
 * @code regexp -> code @endcode declarations, optionally by code that
 * is used when no regexp matches.

 * The input is matched against all the given regular expressions. The
 * various matches are sorted according to position, and the code
 * associated with the first match is then evaluated. If no match is
 * found, then the code at the end of the block is used.
 *
 * For example, to draw a circle if a text contains "circle" or
 * "cercle", and draw a rectangle if the text contains "rect" or
 * "rectangle", and draw a picture otherwise, then you use the
 * following code:
 *
 * @code
 * pattern_match InputText,
 *     "c[ie]rcle"      -> circle 0, 0, 100
 *     "rect(angle)?"   -> rectangle 0, 0, 100, 100
 *     image 0, 0, 1, 1, "MyImage.jpg"
 * @endcode
 *
 * By default, regular expressions are "greedy". You can prefix a
 * regular expression with a unary minus to make it ungreedy. This is
 * useful for example when parsing text like
 * @code "A <b>bold</b> text in <b>two places</b>" @endcode.
 *
 * For example, you can get the "bold" text as follows:
 * @code
 * pattern_match InputText,
 *     - "<b>.*</b>"    -> draw_bold_text regexp_captured 1
 * @endcode
 * 
 * @param input The text to search
 * @param code  The block of code describing the patterns
 *
 * @return The value returned by the code that was executed.
 *
 *
 * @~french
 * Analyse le texte donné en utilisant des expresssions régulières.
 *
 * Le paramètres @a code doit être un bloc contenant une séquence de
 * déclarations de la forme
 * @code regexp -> code @endcode, éventuellement suivie par du code
 * qui sera utilisé quand aucune des expressions régulières ne convient.
 *
 * Le texte donné en entrée est comparé à chacune des expressions
 * régulières. Les différentes expressions qui correspondent sont
 * ensuite triées par position, et le code correspondant à la première
 * position est évalué. Si aucune des expressions régulières ne
 * correspond, le code placé à la fin du bloc est utilisé.
 *
 * Par exemple, pour tracer un cercle si un texte contient "circle" ou
 * "cercle", et tracer un rectangle si le texte contient "rect" ou
 * "rectangle", et tracer une image, vous pouvez utiliser le code suivant.
 *
 * @code
 * pattern_match InputText,
 *     "c[ie]rcle"      -> circle 0, 0, 100
 *     "rect(angle)?"   -> rectangle 0, 0, 100, 100
 *     image 0, 0, 1, 1, "MyImage.jpg"
 * @endcode
 *
 * Par défaut, les expressions régulières sont "gourmandes", c'est à
 * dire qu'elles essaient de capturer le maximum de texte. On peut
 * préfixer une expression régulière par le signe moins pour capturer
 * le plus petit texte. C'est utile par exemple pour analyser un texte
 * comme * @code "Un texte <b>gras</b> à <b>deux endroits</b>" @endcode.
 *
 * Par exemple, on peut obtenir le texte "gras", on peut utiliser:
 * @code
 * pattern_match InputText,
 *     - "<b>.*</b>"    -> draw_bold_text regexp_captured 1
 * @endcode
 * 
 * @param input Le texte à analyser
 * @param code  Le bloc de code décrivant les actions et expressions régulières
 * 
 * @return La valeur retournée par le code exécuté
 */

regexp_parse(text input, tree code);


/**
 * @~english
 * Parse all occurences of regular expressions, return last evaluated value
 *
 * This works like @ref regexp_parse, but it keeps analyzing the text
 * and returns the value of the last code evaluated.
 *
 * For example, if you want to draw a circle for each occurence of
 * "circle" or "cercle" in a text, and return the number of items
 * drawn, you can use something like:
 *
 * @code
 * count := 0
 * regexp_parse_last InputText,
 *     "c[ie]rcle" -> circle 0, 0, 5; translatex 10; count := count + 1
 * @endcode
 *
 * @param input The text to search
 * @param code  The block of code describing the patterns
 *
 * @return The value returned by the last code that was executed.
 *
 *
 * @~french
 * Anlyse toutes les occurences d'expressions régulières, retourne la
 * dernière valeur évaluée.
 *
 * Cette fonction est similaire à @ref regexp_parse, mais continue à
 * évaluer le texte et retourne la dernière valeur évaluée.
 *
 * Par exemple, pour dessiner un cercle pour chaque occurence de
 * "cercle" ou "circle" dans un texte, et retourner le nombre
 * d'éléments tracés, vous pouvez utiliser:
 *
 * @code
 * count := 0
 * regexp_parse_last InputText,
 *     "c[ie]rcle" -> circle 0, 0, 5; translatex 10; count := count + 1
 * @endcode
 *
 * @param input Le texte à analyser
 * @param code  Le bloc de code décrivant les actions et expressions régulières
 * 
 * @return La valeur retournée par le dernier code exécuté
 */

regexp_parse_last(text input, tree code);

/**
 * @~english
 * Create a list with all values returned by evaluation of pattern matches.
 *
 * This works like @ref regexp_parse, but it builds a list out of all
 * evaluated values.
 *
 * For example, if you want to draw a list containing a "Vowel" for every
 * occurence of a vowel, and "Consonent" for every consonent, you can use:
 *
 * @code
 * regexp_parse_all InputText,
 *     "[aeiouy]" -> "Vowel"
 *     "[a-z]     -> "Consonent"
 *     "."        -> "Other"
 * @endcode
 *
 * @param input The text to search
 * @param code  The block of code describing the patterns
 *
 * @return A list containing all values returned by the executed codes
 *
 * @~french
 * Crée une liste à partir des valeurs retournées pour toutes les
 * expressions régulières trouvées
 *
 * Cette fonction est similaire à @ref regexp_parse, mais elle crée
 * une liste avec chacune des valeurs retournées.
 *
 * Par exemple, si on veut faire une liste contenant "Voyelle" pour
 * chaque voyelle, et "Consonne" pour chaque consonne, on peut utiliser:
 *
 * @code
 * regexp_parse_all InputText,
 *     "[aeiouy]" -> "Voyelle"
 *     "[a-z]     -> "Consonne"
 *     "."        -> "Autre"
 * @endcode
 *
 * @param input Le texte à analyser
 * @param code  Le bloc de code décrivant les actions et expressions régulières
 * 
 * @return Une liste contenant les valeurs retournées par les codes exécutés
 */

regexp_parse_all(text input, tree code);


/**
 * @~english
 * Create a text by concatenating all returned values for matched
 * regular expressions.
 *
 * This works like @ref regexp_parse, but it concatenates all the
 * returned value for matches together as a single text.
 *
 * For example, if you want to extract the plain text from an HTML or
 * XML stream, removing all the tags, you can use the following:
 *
 * @code
 * regexp_parse_text InputText,
 *     - "<.*>"  -> ""
 *     - "[^<]+" -> regexp_captured 0
 * @endcode
 *
 * @param input The text to search
 * @param code  The block of code describing the patterns
 *
 * @return A text concatenating all returned values
 *
 * @~french
 * Crée un texte en concaténant les valeurs retournées pour chaque
 * expression régulière trouvée.
 *
 * Cette fonction est similaire à @ref regexp_parse, mais en
 * assemblant toutes les valeurs retournées en un seul texte.
 *
 * Par exemple, pour extraire le texte d'un fichier HTML ou XML, vous
 * pouvez utiliser le code suivant:
 *
 * @code
 * regexp_parse_text InputText,
 *     - "<.*>"  -> ""
 *     - "[^<]+" -> regexp_captured 0
 * @endcode
 *
 * @param input Le texte à analyser
 * @param code  Le bloc de code décrivant les actions et expressions régulières
 * 
 * @return Un texte cumulant toutes les valeurs retournées
 */

regexp_parse_text(text input, tree code);


/**
 * @~english
 * Return captured text after a pattern match
 *
 * Text corresponding to patterns placed between parentheses is
 * @e captured and can be returned by this function. Index 1
 * corresponds to the first captured pattern, index 2 corresponds to
 * the second captured pattern. Index 0 corresponds to the entire
 * captured text.
 *
 * @param index The index of the captured text to return
 *
 * @return The captured text
 *
 * @~french
 * Retourne le texte capturé quand une expression régulière correspond
 *
 * Le texte correspondant aux parties d'une expression régulière entre
 * parenthèses est @e capturé et peut être retourné par cette
 * fonction. L'index 1 correspond au premier texte capturé, l'index 2
 * au deuxième, et l'index 0 à l'intégralité du texte couvert par
 * l'expression régulière.
 *
 * @param index L'index du texte capturé à retourner
 * 
 * @return Le texte capturé
 */

regexp_captured(integer index);


/**
 * @~english
 * Return the position of captured text after a pattern match
 *
 * This function returns the position in the original text of each
 * captured text. If the given index is 0, it returns the position
 * where the regular expression was matched.
 *
 * @param index The index of the captured text
 *
 * @return The position of the captured text
 *
 * @~french
 * Retourne la position à laquelle une expression régulière correspond
 *
 * Cette fonction retourne la position dans le texte original de
 * chaque texte capturé. Si l'index est 0, elle retourne la position
 * où l'expression régulière entière a été trouvée.
 *
 * @param index L'index du texte capturé
 * 
 * @return La position du texte capturé
 */

regexp_position(integer index);


/**
 * @~english
 * The length of text matched by a regular expression
 *
 * @return The length of the matched text
 *
 * @~french
 * La longueur du texte correspondant à une expression régulière
 *
 * @return La longueur du texte trouvé
 */

regexp_matched_length();


/**
 * @~english
 * Escapes text for use in a regular expression
 *
 * @param input The input text to escape
 * @return A text where all occurences of characters that have special
 * meaning in a regular expression have been protected.
 *
 * @~french
 * Protège un texte pour qu'il soit utilisable dans une expression régulière
 *
 * @param input Le texte à protéger
 * @return Un texte où toute occurence de charactères qui ont un sens
 * particulier pour les expressions régulières ont été protégés.
 */

regexp_escape();


/**
 * @}
 */
