/**
 * @~english
 * @mainpage Tao Presentations Documentation
 *
 * Welcome to the online documentation for Tao Presentations.
 *
 * @section secWhatYouWillFind What is in this manual
 *
 * In this manual, you will find reference information on:
 *   - the format and structure of Tao documents,
 *   - the constructs (@a builtins) that you can use to write documents,
 *   - the XLR language, which is the basis of Tao documents.
 *
 * The reference pages also contain some examples.
 *
 * You may find more information and the latest news about Tao Presentations on
 * our web site, <a href="http://taodyne.com/">taodyne.com</a>.
 *
 * @~french
 * @mainpage Documentation de Tao Presentations
 *
 * Bienvenue dans la documentation de Tao Presentations.
 *
 * @section secWhatYouWillFind Contenu de ce manuel
 *
 * Dans ce manuel, vous trouverez des informations sur :
 *   - le format et la structure des documents Tao,
 *   - les expressions (@a builtins) que vous pouvez utiliser pour écrire
 * des documents,
 *   - le langage XLR, qui est la base des documents Tao.
 * 
 * @~
 * @ref getting_started \n
 * @ref tao_documents \n
 * @ref xlref \n
 * @ref imports \n
 * @ref platform_notes \n
 * @ref credits
 */

/**
 * @~english
 * @page getting_started Getting started
 *
 * The easiest way to learn Tao Presentations is probably to start with
 * sample documents and change them to your liking. This documentation is
 * there to help you understand specific aspects of the document description
 * language as you progress.
 *
 * But first, you may want to watch the following introduction video
 * (french commentary):
 *
 * @~french
 * @page getting_started Pour bien démarrer
 *
 * La façon la plus simple d'apprendre Tao Presentations est probablement
 * de partir d'exemples, et de les modifier comme vous le souhaitez.
 * Cette documentation vous permettra d'apprendre progressivement le langage
 * de description des documents.
 *
 * Mais pour commencer, nous vous suggérons de regarder la vidéo suivante :
 *
 * @~
 * @htmlonly
 * <a href="http://youtube.com/watch?v=LpPwjB46E5I">
 * @endhtmlonly
 * @image html youtube_tuto.png
 * @htmlonly
 * </a>
 * @endhtmlonly
 *
 * @~english
 * A number of examples are available as @a templates. To create a new Tao
 * document from a template, click <tt>File > New from Template Chooser...</tt>
 * and follow the wizard. When done, click <tt>View > Document Source</tt> to
 * bring up a simple source code editor.
 * You can edit Tao Presentation documents with any text editor that supports
 * the UTF-8 encoding. Please make sure you do select this format when saving
 * your file, or non-ASCII characters may not show up correctly in Tao.
 *
 * @note Currently, only the main <tt>.ddd</tt> document file can be edited
 * with the bundled editor. Composite documents (see below) should be modified
 * with an external text editor.
 *
 * @~french
 * De nombreux exemples sont disponibles sous forme de @a modèles. Pour créer
 * un nouveau document à partir d'un modèle, sélectionnez le menu
 * <tt>Fichier > Nouveau à partir d'un mmodèle...</tt> et suivez l'assistant.
 * Ensuite, utilisez <tt>Affichage > Source du document</tt> pour ouvrir un
 * éditeur de code source.
 * Pour modifier un document Tao Presentations, vous pouvez utiliser n'importe
 * quel éditeur de texte qui permet de lire et enregistrer le format UTF-8.
 * Assurez-vous que c'est bien ce format qui est sélectionné lorsque vous
 * enregistrez votre document, faute de quoi certains caractères spéciaux ou
 * accentués ne s'afficheront pas correctement dans Tao.
 *
 * @note Actuellement, seul le document principal <tt>.ddd</tt> peut être
 * visualisé et modifié dans l'éditeur intégré. Les documents composites
 * (cf. ci-dessous) peuvent être modifiés en utilisant un éditeur externe.
 */
/**
 * @~english
 * @page tao_documents Tao Presentation documents
 *
 * A Tao document is a text file with the <tt>.ddd</tt> extension. The
 * character encoding is <a href="http://en.wikipedia.org/wiki/UTF-8">UTF-8</a>
 * which enables the use of international characters anywhere in the document.
 *
 * In addition to the main <tt>.ddd</tt> file, a Tao document may be composed
 * of other files:
 *  - Secondary files imported into the main file, in order to make the
 *    document more modular and easier to work with. @n
 *    These files may be <tt>.ddd</tt> files themselves, as long as they are
 *    valid Tao documents on their own. For instance, you may want to subdivide
 *    a long or complex presentation into several shorter ones, and then create
 *    a main <tt>.ddd</tt> file that will simply import all the others:
@code
import "first_part.ddd"
import "second_part.ddd"
@endcode
 *    When an imported file cannot be opened as a Tao document on its own,
 *    it is usual to name it with a <tt>.xl</tt> extension to denote that the
 *    file is XL(R) language source.
 *  - Pictures. Typically (but not necessarily) stored under the @c images
 *    subdirectory. Several file formats are supported (PNG, JPEG, and so on;
 *    refer to the @ref image primitive for details).
 *  - Fonts. TrueType or OpenType fonts may be stored in the @c fonts
 *    subdirectory.
 *
 * When it opens a document, Tao Presentation will automatically import the
 * files @c xl:user.xl and @c xl:theme.xl, in that order, if they exist
 * (see the @ref secSearchPaths "search paths" section below for details on the
 * meaning of @c xl:).
 *
 * @~french
 * @page tao_documents Les documents Tao Presentation
 *
 * Un document Tao est un fichier texte avec l'extension <tt>.ddd</tt>. Les
 * caractères sont encodés en 
 * <a href="http://fr.wikipedia.org/wiki/UTF-8">UTF-8</a> ce
 * qui permet d'utiliser des caractères internationaux n'importe où dans le
 * document.
 *
 * En plus du fichier principal <tt>.ddd</tt>, un document Tao peut être
 * composé d'autres fichiers:
 *  - Des fichiers secondaires, importés par le fichier principal, de manière
 *    à rendre le document plus modulaire et plus facile à faire évoluer. @n
 *    Ces fichiers peuvent être eux-même des fichiers <tt>.ddd</tt>, s'ils sont
 *    des documents Tao valides. Vous pouvez par exemple diviser une
 *    présentation longue ou complexe en plusieurs documents plus courts, puis
 *    créer un fichier <tt>.ddd</tt> principal qui importe les autres :
@code
import "first_part.ddd"
import "second_part.ddd"
@endcode
 *    Lorsqu'un fichier importé ne peut pas être ouvert directement comme un
 *    document Tao, il est habituel de le nommer avec l'extension <tt>.xl</tt>
 *    pour mettre en évidence qu'il s'agit de source XL(R).
 *  - Des images. En principe, les fichiers images sont dans le sous-répertoire
 *    @c images. Plusieurs formats de fichier peuvent être lus par Tao
 *    Presentations : PNG, JPEG, et d'autres. Consultez la fonction @ref image
 *    pour plus de détails.
 *  - Des polices de caractères. Des fichiers TrueType ou OpenType peuvent être
 *    stockés dans le sous-répertoire @c fonts.
 *
 * Lorsque Tao Presentation ouvre un document, les fichiers @c xl:user.xl et
 * @c xl:theme.xl sont automatiquement importés, dans cet ordre, s'ils
 * existent (le paragraphe @ref secSearchPaths explique le sens de @c xl:).
 *
 * @~english
 * @section secDocStructure Document structure
 *
 * Tao documents are written in the Tao language, which enables to
 * describe the structure of the document (by creating items such as:
 * pages, slides, text boxes, rectangles and so on), and at the same
 * time define its dynamic properties such as how the document changes
 * with time or external events (key press or mouse position, for
 * instance).
 *
 * You can group items and give them names using the rewrite operator @c -> and an
 * indented block, like this:
@code
my_items ->
    first_item
    second_item
@endcode
 * It means that writing @c my_items is equivalent to @c first_item followed by
 * @c second_item. You can also use parameters, such as in:
@code
my_circle Radius ->
    color "red"
    circle 0, 0, Radius
@endcode
 * Here, @c Radius may be considered as a parameter and @c my_circle is like a
 * function (or procedure).
 * @warning The unqualified word @c Radius in the left part of the @c ->
 * operator is in the same lexical scope as @c my_circle. Therefore, suppose
 * you have previously defined a @c Radius symbol, for instance
 * <tt>Radius -> 50</tt>:
@code
Radius -> 50
my_circle Radius ->
    color "red"
    circle 0, 0, Radius
@endcode
 Then, the definition of @c my_circle would evaluate
 * to:
@code
my_circle 50 ->
    color "red"
    circle 0, 0, 50
@endcode
 * ...which is legal, but probably not what you would expect: in the end,
 * only <tt>my_circle 50</tt> would be defined and could be used. Trying to
 * call @c my_circle with any other value (for instance, 100) would result
 * in an error:
@verbatim
No form matches my_circle 100
@endverbatim
 * To avoid such issues, it is recommended to use different naming
 * conventions for parameters and other symbols. In this documentation,
 * we always capitalize the parameters so they do not conflict with
 * builtins.
 * In our example, we could also qualify the @c Radius parameter to make
 * it obvious that is is a parameter:
@code
my_circle Radius:real ->
    color "red"
    circle 0, 0, Radius
@endcode
 *
 * @~french
 * @section secDocStructure Structure d'un document
 *
 * Les documents Tao sont écris dans le langage Tao, qui permet de décrire
 * la structure du document (en créant des éléments tels que : pages,
 * transparent, boîte de texte, rectangle, etc.), et en même temps de définir
 * ses propriétés dynamiques, telles que comment le document change au cours
 * du temps ou lorsque des événements externes se déroulent (par exemple,
 * l'appui sur une touche du clavier ou le déplacement de la souris).
 *
 * Vous pouvez grouper des éléments et leur donner un nom en utilisant la
 * fléche @c -> de la façon suivante :
@code
mes_articles ->
    premier_article
    second_article
@endcode
 * Celà signifie que @c mes_articles est équivalent à @c premier_article
 * suivi de @c second_article. Vous pouvez également passer des paramètres,
 * par exemple :
@code
mon_cercle Rayon ->
    color "red"
    circle 0, 0, Rayon
@endcode
 * Ici, @c Rayon peut être considéré comme un paramètre et @c mon_cercle est
 * comme une function (ou une procédure).
 * @warning Le nom @c Rayon à gauche de la flèche @c ->
 * est dans le même portée lexicale que @c mon_cercle. Par conséquent,
 * supposons que vous ayez précédemment défini un symbole @c Rayon,
 * par exemple <tt>Rayon -> 50</tt> :
@code
Rayon -> 50
mon_cercle Rayon ->
    color "red"
    circle 0, 0, Rayon
@endcode
 * Dans ce cas, la définition de @c mon_cercle se transformerait en :
@code
mon_cercle 50 ->
    color "red"
    circle 0, 0, 50
@endcode
 * ...ce qui est valide, mais probablement pas ce que vous attendiez.
 * En effet, seul <tt>mon_cercle 50</tt> serait définit et pourrait
 * être utilisé. Si vous essayiez d'appeler
 * @c mon_cercle avec n'importe quelle autre valeur (par exemple, 100)
 * vous obtiendriez une erreur :
@verbatim
No form matches my_circle 100
@endverbatim
 * Pour éviter de tels problèmes, il est recommendé d'utiliser des conventions
 * de nommage différentes pour les paramètres et les autres symboles.
 * Dans cette documentation,
 * nous utilisons toujours une majuscule pour les paramètres, de telle sorte
 * qu'ils n'entrent pas en conflit avec les fonctions prédéfinies (toujours
 * en minuscules).
 * Pour revenir à notre exemple, nous aurions aussi pu qualifier le paramètre
 * @c Rayon pour mettre en évidence le fait qu'il s'agit d'un paramètre :
@code
mon_cercle Rayon:real ->
    color "red"
    circle 0, 0, Rayon
@endcode
 *
 * @~english
 * @section secXLRef The XLR Language
 *
 * The XLR programming language is the basis of the Tao document syntax.
 * Detailed information is available in the
 * \ref xlref "XLR language reference guide".
 * However, a deep knowledge of XL is not needed to write
 * Tao documents.
 *
 * @~french
 * @section secXLRef Le Langage XL
 *
 * Le langage de programmation XL (ou XLR) est la base des documents
 * Tao Presentations.
 * Des informations détaillées sont disponibles dans
 * \ref xlref "le guide de référence du langage XLR" (en anglais).
 * Toutefois, une
 * connaissance approfondie de XL n'est pas nécessaire pour écrire des
 * documents Tao.
 *
 * @~english
 * @section secBuiltins Tao Builtins and Primitives
 *
 * A @a builtin is any language construct that is not user-defined. For
 * example, @ref page is a builtin defined by the Tao program.
 *
 * Two categories of builtins can be distinguished:
 *  - @ref TaoBuiltins "Tao builtins" are implemented by the Tao Presentations
 *    program itself, and are imediately available in a document,
 *  - @ref Modules "Module builtins" are defined in @a modules, and require
 *    an @ref import statement before they can be used.
 *
 * When a builtin is implemented in native code (C or C++), it is called a
 * @a primitive.
 *
 * @~french
 * @section secBuiltins Les builtins et primitives Tao
 *
 * On appelle @a builtin une construction du langage qui n'est pas définie
 * par l'utilisateur. Par exemple, @ref page est un builtin défini par le
 * programme Tao Presentations.
 *
 * On distingue deux catégories de builtins :
 *  - @ref TaoBuiltins "Les builtins Tao" sont définis par le programme
 *    Tao Presentations lui-même, et sont immédiatement disponibles dans
 *    un document,
 *  - @ref Modules "Les builtins de modules" sont définis dans des
 *    @a modules, et nécessitent une commande @ref import avant de pouvoir
 *    être utilisés.
 *
 * Quand un builtin est implémenté par du code natif (C ou C++), on l'appelle
 * @a primitive.
 *
 * @~english
 * @section secVariables Variables
 *
 * To declare and initialise a variable with the current version of Tao
 * Presentations, write the following:
 @code
count -> 0  // Create variable of type integer (0 is an integer)
count := 1  // Initialize it

size -> 0.0  // Create variable of type real (0.0 is a real)
size := 10.0 // Initialize it
 @endcode
 * Make sure you do not assign an integer to a variable created as a real,
 * and vice versa. Otherwise weird things will occur, like the variable not
 * having the expected value.
 *
 * @~french
 * @section secVariables Variables
 *
 * Pour déclarer et initialiser une variable dans la version actuelle de
 * Tao Presentations, utilisez la notation suivante :
 @code
compteur -> 0  // Crée une variable de type entier (0 est un entier)
compteur := 1  // Initialise la variable

taille -> 0.0  // Crée une variable de type réel (0.0 est un réel)
taille := 10.0 // Initialise la variable
 @endcode
 * Prenez garde à ne pas affecter un entier à une variable créée avec un
 * type réel, ou vice versa. Sinon, la variable risque de ne pas avoir la
 * valeur attendue.
 *
 * @~english
 * @section secExecModel Execution and Drawing Model
 *
 * In order to display a document, Tao @a executes the document (or parts of
 * it). The steps are the following:
 *
 * <ol>
 * <li> @b Compile and @b execute the document once. This step creates a
 *      hierarchy of objects of two kinds: @a Drawings and @a Layouts.
 *      <ul>
 *        <li> A Drawing is anything that may be drawn. For example,
 *             executing the @ref rectangle builtin creates a Drawing object
 *             of type Rectangle.
 *        <li> A Layout is a special kind of drawing that may contain other
 *             Drawings. The following builtins create Layouts: @ref page,
 *             @ref shape, @ref locally, @ref group, @ref anchor, @ref table,
 *             @ref table_cell.
 *      </ul>
 *      In addition to creating objects, the execution of some primitives
 *      makes the containing layout sensitive to some external events. @n
 *      For example, @ref seconds will request re-execution of the
 *      containing layout one second later, while @ref mouse_x will cause
 *      its re-execution upon reception of a <em>mouse move</em> event.
 * <li> @b Draw the Layout and Drawing hierarchy created by the document
 *      execution. @n
 *      For example, drawing a Rectangle object consists in displaying this
 *      rectangle on the screen with all the attributes that were selected
 *      during the the program execution (such as: @ref color or
 *      @ref line_color).
 * <li> @b Wait for an external event to occur.
 * <li> @b Dispatch the event from the top to the bottom of the Layout
 *      hierarchy. When a Layout is encountered that has subscribed to the
 *      event, @b re-execute the part of the document that is under that
 *      layout.
 * <li> @b Draw the complete Layout and Drawing hierarchy.
 * </ol>
 *
 * The following example shows how events trigger partial execution.
 *
@code
writeln "1: minutes = " & text minutes
locally
    writeln "2: seconds = " & text seconds
locally
    writeln "3: mouse_x = " & text mouse_x
@endcode
 *
 * The above code first writes three lines of text, one for each
 * message. This corresponds to the first, complete execution of the
 * document. @n
 * Then, line number 2 shows up every second. Indeed, the @ref seconds
 * builtin has associated a one-second timer to the Layout object created by
 * the first @ref locally block, its inner code is therefore re-executed
 * periodically.
 * Similarly, line number 3 is printed whenever the mouse is moved. Note
 * that moving the mouse does not cause line number 2 to be executed again,
 * and the second changing does not cause line number 3 to be run either. @n
 * When the minute changes, however, all three lines are displayed again.
 * This is because the @ref minutes builtin has attached a one-minute timer
 * to the main Layout object. Since the two @ref locally blocks are nested
 * inside the main Layout of the document, they get re-executed too.
 *
 * @~french
 * @section secExecModel Modèle de tracé et d'exécution
 *
 * Pour afficher un document, Tao @a exécute le document (ou certaines parties
 * de celui-ci) selon les étapes suivantes :
 *
 * <ol>
 * <li> @b Compiler et @b exécuter le document une première fois.
 *       Cette étape crée une hiérarchie d'objets de deux types : des
 *      @a Drawings et des @a Layouts.
 *      <ul>
 *        <li> Un Drawing est un élément quelconque qui peut être tracé. Par
 *             exemple, l'exécution du builtin @ref rectangle crée un objet
 *             Drawing de type Rectangle.
 *        <li> Un Layout est une forme spéciale de Drawing qui peut lui-même
 *             contenir d'autres Drawings. Les builtins suivants créent des
 *             Layouts: @ref page,
 *             @ref shape, @ref locally, @ref group, @ref anchor, @ref table,
 *             @ref table_cell.
 *      </ul>
 *      En plus de créer des objets, l'exécution de certaines primitives
 *      rend le Layout dans lequel l'objet se trouve sensible à des événements
 *      extérieurs. @n
 *      Par exemple, @ref seconds provoque la ré-exécution du Layout
 *      une seconde plus tard, tandis que @ref mouse_x provoque sa
 *      ré-exécution lorsque la souris se déplace.
 * <li> @b Tracer la hiérarchie d'objets Layout et Drawing créée par
 *      l'exécution du document. @n
 *      Par exemple, pour tracer un objet Rectangle il faut afficher ce
 *      rectangle avec tous les attributs qui ont été sélectionnés lors de
 *      l'exécution du programme, comme @ref color ou @ref line_color.
 * <li> @b Attendre qu'un événement survienne.
 * <li> @b Distribuer l'événement de haut en bas dans la hiérarchie d'objets
 *      Layout. Quand l'événement rencontre un Layout qui est sensible à cet
 *      événement, @b ré-exécuter la partie du document qui se trouve sous
 *      ce Layout.
 * <li> @b Tracer la hiérarchie complète d'objets Layout et Drawing.
 * </ol>
 *
 * L'exemple qui suit montre comment des événements déclenchent l'exécution
 * partielle du document.
 *
@code
writeln "1: minutes = " & text minutes
locally
    writeln "2: seconds = " & text seconds
locally
    writeln "3: mouse_x = " & text mouse_x
@endcode
 *
 * Le code ci-dessus affiche d'abord trois lignes de texte, une pour chaque
 * message. Celà correspond à la première exécution complète du document. @n
 * Puis, la ligne numéro 2 s'affiche à chaque seconde. En effet, le builtin
 * @ref seconds associe un @a timer à l'objet Layout créé par le premier
 * bloc @ref locally, le code qui se trouve à l'intérieur de ce bloc est
 * donc ré-exécuté périodiquement.
 * De la même façon, la ligne numéro 3 est affichée lorsqu'on bouge la souris.
 * Notez que celà ne provoque pas un nouvel affichage de la ligne 2, pas plus
 * que le changement de seconde ne provoque un nouvel affichage de la ligne
 * numéro 3. @n
 * Lorsque la minute change, par contre, les trois lignes sont affichée à
 * nouveau. Celà s'explique parce que le builtin @ref minutes a associé un
 * @a timer d'une minute à l'objet Layout principal. Puisque les deux blocs
 * @ref locally sont imbriqués dans le Layout principal du document, ils
 * sont également ré-exécutés.
 *
 * @~english
 * @section secSearchPaths Search Paths
 *
 * Some primitives that take a file name or file path accept a special syntax
 * called a <em>search path prefix</em>. For instance:
@code
image 0, 0, 100%, 100%, "image:file.jpg"
@endcode
 * The @c image: prefix indicates where to find the file. Tao will look for
 * @c file.jpg in several folders and use the first file it can find.
 * Here is where Tao will look for files for each prefix:
 * - @b image: the image folders registered by @ref Modules "modules", the
 *   @c images subfolder in the current document folder, the current document
 *   folder.
 * - @b xl: the current document folder, the user's Tao folder, the application
 *   folder.
 * - @b system: the application folder.
 *
 * The user's Tao folder is <tt>%%LOCALAPPDATA%\\Taodyne\\Tao Presentations</tt> on Windows,
 * <tt>$HOME/Library/Application Support/Taodyne/Tao Presentations</tt> on MacOSX,
 * and <tt>$HOME/.local/share/data/Taodyne/Tao Presentations</tt> on Linux.
 * Replace <tt>Tao Presentations</tt> with <tt>Tao Presentations Player</tt>
 * if you are using the player's edition.
 * The application folder is where Tao Presentation is installed.
 * 
 * @~french
 * @section secSearchPaths Chemins additionnels
 *
 * Certaines primitives qui prennent pour paramètre un nom de fichier
 * acceptent une syntaxe spéciale appelée un <em>préfixe de chemin
 * additionnel</em>. Par exemple :
@code
image 0, 0, 100%, 100%, "image:file.jpg"
@endcode
 * Le préfixe @c image: indique comment trouver le fichier. Tao va chercher
 * @c file.jpg dans plusieurs dossiers, et utiliser le premier fichier trouvé.
 * Voici les dossiers dans lesquels Tao va chercher pour chaque préfixe :
 * - @b image: les dossiers d'images déclarés par les modules chargés par le
 *   document, puis le dossier @c images dans le dossier du document courant,
 *   et enfin le dossier du document courant.
 * - @b xl: le dossier du document courant, puis le dossier utilisateur Tao,
 *   et enfin le dossier de l'application Tao.
 * - @b system: le dossier de l'application Tao.
 *
 * Le chemin du dossier utilisateur Tao est
 * <tt>%%LOCALAPPDATA%\\Taodyne\\Tao Presentations</tt> sous Windows,
 * <tt>$HOME/Library/Application Support/Taodyne/Tao Presentations</tt> sous MacOSX,
 * et <tt>$HOME/.local/share/data/Taodyne/Tao Presentations</tt> sous Linux.
 * Remplacez <tt>Tao Presentations</tt> par <tt>Tao Presentations Player</tt>
 * si vous utilisez le lecteur Tao.
 */
/**
 * @~english
 * @page xlref XLR Language Reference
 *
 * @htmlinclude XLRef.html
 *
 * @~french
 * @page xlref Guide de référence du language XLR
 *
 * @htmlinclude XLRef.html
 */
/**
 * @~english
 * @page imports Importing Files and Modules
 *
 * The extensible nature of Tao Presentation helps you define and organize your
 * own re-usable pieces of documents. There are several ways of organizing your
 * files.
 *
 * @section secImport Importing files
 *
 * The simplest way to group your own presentations in a reuseable fashion, is
 * to use the @ref import keyword to refer to an external file from any Tao
 * document.
 *
 * For instance, let's assume that you are writing a presentation
 * called @c SalesReport.ddd. In this document, you want to include some
 * introductory slides, then actual sales report for two regions: the US and
 * Europe. You would like to be able to tailor your presentation to your
 * audience and show only the US report, or only the Europe part, or
 * the whole presentation.
 *
 * One way of doing this is to create three files: @c SalesReport.ddd
 * (the main document), @c US.ddd (the slides for the US region) and
 * @c Europe.ddd (for Europe), and use the @c import statement. For this
 * usage, @c import expects a file name enclosed in double quotes:
@code
// SalesReport.ddd

page "Introduction",
    // ...

import "US.ddd"
import "Europe.ddd"
@endcode
 * With this technique, you can open any of the three @c .ddd files
 * and thus show exactly the information you want.
 *
 * Note that imported files need not be complete, standalone presentations.
 * You may use the @c import keyword to bring in definitions or
 * utilities you want to re-use often. Consider for instance the
 * following example:
@code
// Example.ddd
import "my_definitions.xl"
page "Introduction",
    text_box 0, 0, window_width, window_height,
        vertical_align_center
        align_center
        my_text_color
        text "This is "
        my_highlight "highlighted"
        text " text"
@endcode
@code
// my_definitions.xl
my_text_color -> color "black"
my_highlight T:text ->
    color "red"
    text T
    my_text_color
@endcode
 * Just make sure that the file @c my_definitions.xl is located in the same
 * directory as the main document, @c Example.ddd (or any other file that
 * imports it). All the
 * definitions you need are in one central location, @c my_definitions.xl.
 *
 * @note It is customary to give your files a @c .xl extension when they
 * contain XL code, but are not complete Tao Presentations documents (which
 * preferably take the @c .ddd extension).
 *
 * @section secUserDirectory The Next Step: User Definitions
 *
 * The @c import keyword is fine to help you group custom definitions
 * in a file along with your main document. But on its own, is not very
 * convenient when you need to share definitions
 * between completely different documents. For this situation, we would like to
 * have one central location where we could store our @c .xl files, and
 * use them from any Tao document.
 *
 * Well, the @c import statement allows just that. Simply use the @c xl:
 * search path prefix. For instance, create a file called
 * @c my_definitions.xl, save it in the user-specific Tao folder
 * (see @ref secSearchPaths "above"), and use it as follows:
@code
import "xl:my_definitions.xl"
@endcode
 *
 * @section secUserModule User Modules
 *
 * Let's assume that you have written some nice XL definitions for
 * Tao Presentations, and you would like to share them with other users of
 * the program. Then you're ready to create a
 * @ref Modules "Tao Presentations module".
 *
 * There are several advantages to using a module rather than a simple
 * XL file stored in the user's Tao folder:
 *  - You can add a description, your name and your web site in a standard
 * way, so that these information appear in the module preference dialog.
 *  - A module has a version number. This makes it easier to detect
 * compatibility problems with old documents, as your module changes.
 * It also enables the automatic upgrade of your installed modules if
 * you have downloaded them from a network location (through the
 * "Check for updates" button in the module preference dialog). For more
 * information on module version numbers, see the @ref import primitive.
 *  - Modules can contain native code in the form of shared libraries,
 * which makes them a very powerful way to extend the Tao program. Native
 * modules are not covered in this documentation but in the Tao Module SDK,
 * which is available in the Tao application folder.
 *
 * @subsection secCreatingModule Creating A Module
 *
 * To create a new module, you may use a copy of an existing module as a
 * starting point. The Tao Visuals module is a good candidate for that
 * purpose; here is how to proceed.
 *  - Locate the folder of the the Tao Visuals module: open the preference
 * dialog, click Module options, then click the info icon for to the
 * Tao Visuals entry. An information page opens. Click the link next
 * to "Installed in:" to open a file browser window in the module folder.
 *  - Copy the whole @c tao_visuals folder into the @c modules folder
 * of your user account. More specifically, you should end up with:
 *    - Windows: <tt>%%LOCALAPPDATA%\\Taodyne\\Tao Presentations\\modules\\tao_visuals</tt>.
 *      To open <tt>%%LOCALAPPDATA%</tt> easily on Windows, just type <tt>%%LOCALAPPDATA%</tt>
 *      in the Start menu.
 *    - MacOSX: <tt>$HOME/Library/Application Support/Taodyne/Tao Presentations/modules/tao_visuals</tt>.
 *     To open <tt>$HOME</tt> on MacOSX, open a new Finder window.
 *    - <tt>$HOME/.local/share/data/Taodyne/Tao Presentations/modules/tao_visuals</tt> on Linux.
 *  - Rename the module. You must rename the directory
 * as well as the main XL %file: <tt>tao_visuals/tao_visuals.xl</tt> should become
 * <tt>my_module/my_module.xl</tt>.
 *  - Edit the module's contents. Change the identifier (<tt>id=</tt>). Use a
 * Universally Unique Identifier (UUID) to make sure your module won't conflict
 * with any To module. You may use the @c uuidgen command or find a website
 * that will do that for you. Change the @c name, @c description,
 * @c import_name, @c version etc. as you wish. You may also provide
 * translations. And, of course, replace all the definitions by your own.
 *  - Provide your own icon (<tt>icon.png</tt>) or just delete the icon file if
 * you don't have an icon handy.
 *  - Restart Tao Presentations and check the module info page: your module
 * should normally be in the list. If it isn't, check the troubleshooting
 * instructions below.
 *
 * @subsection secModuleTS Troubleshooting Modules
 *
 * You can check which modules are detected by selecting the Preferences menu
 * (Tao Presentations>Preferences on MacOSX, Help>Preferences on Windows).
 * In this dialog box you can also disable or enable modules.
 *
 * Modules from the user's home directory are loaded first, followed by
 * modules in the application directory. Therefore, if two modules with the
 * same identifier are found under both locations, the user's one is preferred
 * and the other one is ignored.
 *
 * Module debugging can be turned on as follows:
 * - Select the Debug group in the Preferences dialog, click the "modules"
 *   checkbox, click "Save". Restart Tao.
 * - Through the command line option: <tt>-tmodules</tt>
 *
 * Debug traces are sent to the standard error output (@c stderr). Their
 * final destination depends on the platform and whether you started Tao from
 * a command line prompt or using the graphical user interface, as explained
 * below.
 *
 * @~french
 * @page imports Importer des fichiers et de modules
 *
 * La nature extensible de Tao Presentations vous aide à définir et organiser
 * vos propres éléments de documents. Il existe plusieurs façons d'organiser
 * vos fichiers.
 *
 * @section secImport L'import de fichiers
 *
 * La manière la plus simple de regrouper vos fichiers Tao Presentations et de
 * les réutiliser est d'utiliser le mot-clé @ref import qui permet de faire
 * référence à un fichier externe depuis n'importe quel document Tao.
 *
 * Par exemple, supposons que vous écriviez une présentation intitulée
 * @c RapportDesVentes.ddd. Dans ce document, vous voulez présenter des
 * pages d'introduction, puis détailler des chiffres de vente pour deux
 * régions, les États-Unis et l'Europe. Vous aimeriez pouvoir adapter votre
 * présentation à votre public, et montrer seulement le rapport concernant
 * les États-Unis, ou seulement l'Europe, ou la présentation complète.
 *
 * Une façon d'obtenir ce résultat est de créer trois fichiers :
 * @c RapportDesVentes.ddd (le document principal), @c US.ddd (les pages
 * concernant la région US) et @c Europe.ddd (pour l'Europe). Dans le document
 * principal vous utilisez la directive @c import. Pour cet usage, @c import
 * attend un nom de fichier entre guillemets :
@code
// RapportDesVentes.ddd

page "Introduction",
    // ...

import "US.ddd"
import "Europe.ddd"
@endcode
 * Grâce à cette technique, vous pouvez ouvrir n'importe lequel des trois
 * fichiers @c .ddd et ainsi montrer exactement l'information que vous
 * désirez.
 *
 * Notez que les fichiers importés peuvent ne pas être des présentations
 * complètes et autonomes. Vous pouvez utiliser le mot-clé @c import pour
 * inclure des définitions que vous utilisez souvent. Par exemple :
@code
// Exemple.ddd
import "mes_définitions.xl"
page "Introduction",
    text_box 0, 0, window_width, window_height,
        vertical_align_center
        align_center
        ma_couleur_de_texte
        text "On peut "
        en_évidence "mettre en évidence"
        text " du texte"
@endcode
@code
// mes_définitions.xl
ma_couleur_de_texte -> color "black"
en_évidence T:text ->
    color "red"
    text T
    ma_couleur_de_texte
@endcode
 * Assurez vous que le fichier @c mes_définitions.xl se trouve dans le même
 * répertoire que le fichier qui l'importe (@c Exemple.ddd).
 * Toutes les définitions dont vous avez besoin se trouvent rassemblées en
 * un seul endroit, @c mes_définitions.xl.
 *
 * @note On utilise en général l'extension @c .xl pour les fichiers qui
 * contiennent du code XL, mais ne sont pas des documents Tao Presentations
 * complets (qui ont de préférence l'extension @c .ddd).
 *
 * @section secUserDirectory L'étape suivante : les définitions utilisateur
 *
 * Le mot-clé @c import est parfait pour vous aider à grouper des définitions
 * dans un fichier séparé du document principal. Mais cette technique seule
 * n'est pas très pratique pour partager des définitions dans plusieurs
 * documents complètement différents. Pour répondre à ce besoin, il faudrait
 * pouvoir disposer d'un endroit ou déposer nos fichiers @c .xl, pour
 * ensuite les utiliser depuis n'importe quel document.
 *
 * La directive @c import vous permet de faire celà. Il vous suffit
 * d'utiliser le préfixe @c xl:. Par exemple, créez un fichier
 * @c mes_définitions.xl, enregistrez-le dans le dossier utilisateur Tao
 * (cf. @ref secSearchPaths "ci-dessus"), et utilisez-le comme suit :
@code
import "xl:mes_définitions.xl"
@endcode
 *
 * @section secUserModule Les modules utilisateur
 *
 * Supposons que vous ayez écrit des définitions que vous vouliez partager
 * avec d'autres utilisateurs de Tao. Pour celà vous pouvez créer un
 * @ref Modules "module Tao Presentations".
 *
 * Il y a plusieurs avantages à utiliser un module plutôt qu'un simple
 * fichier XL qui se trouverait dans le dossier utilisateur Tao.
 *  - Vous pouvez ajouter une description, votre nom et votre site web. Ces
 * informations apparaissent dans la boîte de préférences des modules.
 *  - Un module possède un numéro de version, ce qui permet de détecter
 * tout problème de compatibilité avec les anciens documents, lorsque votre
 * module change. Ce numéro de version permet également la mise à jour
 * des modules installés s'ils ont été installés depuis le réseau
 * (bouton Mises à jour). Pour plus d'informations sur les numéros de
 * version, consultez la documentation de la primitive @ref import.
 *  - Les modules peuvent contenir du code natif sous la forme de librairies
 * dynamiques, ce qui en fait un moyen très puissant pour étendre le
 * programme Tao. Les modules natifs ne sont pas présentés en détail dans
 * la présente documentation, mais vous pourrez trouver plus d'informations
 * dans le kit de développement disponible dans le répertoire d'installation
 * de Tao Presentations.
 *
 * @subsection secCreatingModule Création d'un module
 *
 * Pour créer un nouveau module, vous pouvez utiliser une copie d'un module
 * existant comme point de départ. « Visuels Tao » est un bon candidat. Voici
 * comment faire.
 *  - Ouvrez le dossier du module : ouvrez la boîte de préférences, cliquez
 * sur la page des options des modules, puis sur l'icône d'information du
 * module « Visuels Tao ». La boîte d'information contient un lien qui permet
 * d'ouvrir le répertoire du module.
 *  - Copiez le répertoire @c tao_visuals dans le répertoire @c modules de
 * votre compte utilisateur. Plus précisément, vous obtiendrez :
 *    - Windows: <tt>%%APPDATA%\\Taodyne\\Tao Presentations\\modules\\tao_visuals</tt>.
 *      Pour ouvrir <tt>%%APPDATA%</tt> facilement, tapez simplement
 *      <tt>%%APPDATA%</tt> dans le menu Démarrer.
 *    - MacOSX: <tt>$HOME/Library/Application Support/Taodyne/Tao Presentations/modules/tao_visuals</tt>.
 *      <tt>$HOME</tt> est votre répertoire utilisateur.
 *    - <tt>$HOME/.local/share/data/Taodyne/Tao Presentations/modules/tao_visuals</tt> sur Linux.
 *  - Renommez le module. Vous devez renommer le dossier mais également le
 * fichier XL principal : <tt>tao_visuals/tao_visuals.xl</tt> doit devenir
 * <tt>mon_module/mon_module.xl</tt>.
 *  - Modifiez le contenu du module. Changez l'identifiant (<tt>id=</tt>).
 * Utilisez un identifiant unique, par exemple un UUID afin d'éviter que
 * votre module entre en conflit avec un autre module Tao.
 * Vous pouvez utiliser la commande @c uuidgen ou utiliser un site web
 * qui le fera pour vous. Changez le nom (@c name), la description
 * (@c description), le nom d'import (@c import_name), éventuellement la
 * version (@c version), etc. comme vous le souhaitez. Les textes doivent
 * si possible être en anglais et vous pouvez fournir des traductions plus
 * bas. Et bien sûr, remplacez le code XL par le vôtre.
 *  - Fournissez une icone (<tt>icon.png</tt>) ou supprimez le fichier si
 * vous n'avez pas d'icône.
 *  - Redémarrez Tao Presentations et vérifiez la page d'information des
 * modules. Votre module devrait normalement être dans la liste. Si ce n'est
 * pas le cas, référez-vous aux instructions ci-dessous.
 *
 * @subsection secModuleTS En cas de problème avec les modules
 *
 * Vous pouvez vérifier quels modules sont détectés en ouvrant la fenêtre de
 * préférences des modules : Tao Presentations>Preferences sous MacOSX et
 * Aide>Préférences sous Windows. Vous pouvez aussi activer ou désactiver des
 * modules.
 *
 * Les modules du dossier utilisateur sont chargés en premier, suivis
 * des modules qui se trouvent dans le dossier de l'application Tao. Par
 * conséquent, si deux modules qui possèdent le même identifiant se trouvent
 * dans les deux dossiers, c'est le module utilisateur qui est chargé.
 *
 * Les traces de debug des modules s'activent de la façon suivante :
 * - Selectionnez le groupe Debug dans la boîte de préférences des modules.
 *   Cochez la case « modules » puis cliquez sur Sauver. Redémarrez Tao.
 * - Il est également possible d'utiliser sur la ligne de commande l'option :
 *   <tt>-tmodules</tt>
 *
 * Les traces de debug sont envoyées sur le canal d'erreur standard
 * (@c stderr). Leur destination réelle dépend de la plateforme et du fait
 * que vous ayez  démarré Tao Presentations depuis l'interpréteur de
 * commandes ou en utilisant l'interface graphique, comme expliqué ici @ref platform_notes.
 */
/**
 * @~english
 * @page platform_notes Platform Notes
 *
 * @section secStdoutStderr Standard Output, Standard Error
 *
 * Errors are normally shown in the application's graphical user interface,
 * either through dialog boxes or in the error display pane. However, some
 * diagnostic message are sent only to the standard output or standard error
 * channels:
 *  - Debug traces,
 *  - Messages sent by the write and writeln builtins.
 *
 * The destination of the standard output and standard error are different
 * if you are using Tao Presentations on MacOSX or on Windows.
 *
 * @subsection secWindows Windows
 *
 * The standard output and standard error channels of Tao Presentations are
 * normally redirected to the file @c tao.log in the @c Tao folder under the
 * current users's document folder. This file is overwritten each time the
 * application is started.
 *
 * However, if the application is started from a command prompt the standard
 * output and standard error will remain connected to the command prompt
 * window, and will not be sent to @c tao.log.
 *
 * @subsection secMacOSX MacOSX
 *
 * The standard output and standard error channels of Tao Presentations are
 * normally sent to log files and can be seen with the MacOSX @c console
 * application.
 *
 * However, if the application is started from a terminal, standard output
 * and standard error will remain connected to the terminal window, and
 * will not appear in the console.
 *
 * @~french
 * @page platform_notes Notes de plateforme
 *
 * @section secStdoutStderr Sortie standard, erreur standard
 *
 * Les erreurs sont normalement affichées par l'interface graphique de
 * l'application, soit par des boîtes de dialogue, soit dans la zone
 * d'affichage des erreurs de la fenêtre principale. Toutefois, certains
 * messages ne sont envoyés que sur les canaux de sortie ou d'erreur standards
 * (stdout, stderr) :
 *  - Les traces de debug,
 *  - Les messages envoyés par les builtins write and writeln.
 *
 * @subsection secWindows Windows
 *
 * Les canaux de sortie et d'erreur standard de Tao Presentations sont
 * normalement redirigés vers le fichier @c tao.log dans le dossier @c Tao
 * sous le dossier des documents de l'utilisateur. Ce fichier est recréé
 * à chaque démarrage de l'application.
 *
 * Toutefois, si l'application est démarrée depuis une invite de commande,
 * les messages s'affichent dans la fenêtre qui a servi à démarrer Tao et ne
 * sont pas envoyés vers @c tao.log.
 *
 * @subsection secMacOSX MacOSX
 *
 * Les canaux de sortie et d'erreur standard de Tao Presentations sont
 * normalement redirigés vers des fichiers et peuvent être lu avec l'application
 * MacOSX @c console.
 *
 * Toutefois, si l'application est démarrée depuis un terminal,
 * les messages s'affichent dans celui-ci et ne sont pas visibles dans
 * l'application console.
 */
/**
 * @~english
 * @page credits Credits/Licenses
 *
 * Tao Presentations uses the following components:
 *
 *   - <b>Qt</b>: Qt is a C++ toolkit for cross-platform application
 *   development. Qt is an open source project. See
 *   <a href="http://qt-project.org/">qt-project.org</a> for more information.
 *   - <b>LLVM</b>: The LLVM Project is a collection of modular and reusable
 *   compiler and toolchain technologies. See
 *   <a href="http://llvm.org/">llvm.org</a> for more information.
 *   - <b>XLR</b>: XLR is a dynamically-compiled language based on parse tree
 *   rewrites. See <a href="http://xlr.sf.net/">xlr.sf.net</a> for more
 *   information.
 *   - <b>Qt Solutions</b>: Tao Presentations contains portions of
 *   the Qt Solutions component. These portions are subject to the
 *   following licensing terms:
 * @verbinclude "QtSolutionsLicense.txt"
 *   - <b>Qt Assistant</b>: The Tao Presentations Help application is derived
 *   from the Qt Assistant of the Qt Toolkit. This
 *   application is covered by the GNU General Public License version 3.0.
 *   The source code is published as a Gitorious project:
 *   <a href="http://gitorious.org/taodyne-open-source/tao_help_viewer">
 *   http://gitorious.org/taodyne-open-source/tao_help_viewer</a>.
 * @~french
 * @page credits Remerciements/Licences
 *
 * Tao Presentations utilise les composants suivants :
 *
 *   - <b>Qt</b>: Qt est une boîte à outils C++ pour le développement
 *   d'applications multi-plateformes. Qt est un logiciel open source. Cf.
 *   <a href="http://qt-project.org/">qt-project.org</a> pour plus d'informations.
 *   - <b>LLVM</b>: Le projet LLVM est un ensemble de technologies de
 *   compilation et d'outils modulaires et réutilisables. Cf.
 *   <a href="http://llvm.org/">llvm.org</a> pour plus d'informations.
 *   - <b>XLR</b>: XLR est un langage compilé dynamiquement basé sur des
 *   réécritures d'arbres. Cf. <a href="http://xlr.sf.net/">xlr.sf.net</a>
 *   pour plus d'informations.
 *   - <b>Qt Solutions</b>: Tao Presentations contient des portions du
 *   composant Qt Solutions. Ces portions sont soumises à la license
 *   suivante :
 * @verbinclude "QtSolutionsLicense.txt"
 *   - <b>Qt Assistant</b>: L'application d'aide de Tao Presentations est
 *   dérivée de <em>Qt Assistant</em> qui fait partie du <em>Qt Toolkit</em>.
 *   Cette application est soumise à la <em>GNU General Public
 *   License</em> version 3.0. Le code source est publié sous le projet Gitorious :
 *   <a href="http://gitorious.org/taodyne-open-source/tao_help_viewer">
 *   http://gitorious.org/taodyne-open-source/tao_help_viewer</a>.
 */

/**
 * @~english
 * @addtogroup TaoBuiltins Tao Presentations Builtins
 *
 * Core functions for Tao Presentations.
 *
 * Tao documents are written using the XL language. Tao builtins are XL forms
 * that you must use to describe your document. They are further divided into
 * categories as shown above.
 *
 * @~french
 * @addtogroup TaoBuiltins Primitives de Tao Presentations
 *
 * Fonctions principales de Tao Presentations
 *
 * Les documents Tao Presentations sont écrits en utilisant le langage XL.
 * Les primitives  de Tao sont des formes XL que vous devez utiliser pour
 * décrire votre document. Elles sont réparties en catégories comme décrit
 * ci-dessus.
 * 
 * @~
 *
 * @todo Document Git-related primitives (branch/commit chooser commands and
 *       checkout)?
 * @todo Document "error" primitives?
 * @todo Document "tree management" primitives? (insert, delete_selection,
 *       set_attribute)
 * @todo Document the z-order management primitives? (bring_to_front, etc.)
 * @todo Document the group management primitives? (group, group_selection,
 *       ungroup_selection)
 */
