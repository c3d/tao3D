/**
 * @~english
 * @taomoduledescription{TaoQuiz, TaoQuiz module}
 * <tt>import TaoQuiz</tt>.@n
 *
 * TaoQuiz module is to be used with the tao_quizz web server.
 * It allows a presenter to ask questions and to users to answer.
 * Questions and answers can contain images as URL.
 * Multichoice question and open text question are available, as well as
 * different kind of behaviors:
 *     * Multichoice question can support many answers per user, or
 *       one answer per user which can be the first or the last the user click.
 *     * A score can be assigned to each answer and the total score of each user
 *       is automaticaly computed.
 *
 * Each question can be time bombed.
 *
 * Each user can have its answer or the stat as feedback.
 *
 * And finaly, an interaction like common drawing using the mouse is possible.
 *
 * Each question must be on a different slide.
 * Each question has two modes:
 *    * the question mode: only the question is displayed.
 *    * the answer mode: question and all the responses are shown.
 *
 * To switch from one to another, hit '/'.
 *
 * @endtaomoduledescription{TaoQuiz}
 *
 * @~french
 * @taomoduledescription{TaoQuiz, module TaoQuiz}
 * <tt>import TaoQuiz</tt>.@n
 *
 * Le module TaoQuiz doit être utilisé avec le server web tao_quizz.
 * Il permet à un présentateur de poser des questions à des utilisateurs.
 *
 * Questions et réponses peuvent contenir des images sous forme d'URL.
 *
 * Les questions peuvent être à choix multiples ou à réponse ouverte.
 * Le comportement du questionnaire est complètement configurable :
 * Pour un QCM, on peut décider de ne garder que la première réponse
 * de chaque utilisateur, ou la dernière ou toutes.
 *
 * À chaque réponse proposée peut être associé un score qui permet
 * le calcul automatique d'un total par utilisateur.
 *
 * Chaque question peut être limité en temps.
 *
 * Pour chacune des questions, le présentateur décide si l'utilisateur voit
 * en retour sa propre réponse, des statistiques sur toutes les réponses
 * ou rien du tout.
 *
 * Et enfin une interaction de type dessin collectif à la souris est possible.
 *
 * Chaque question doit être sur une page différente.
 *
 * Chaque question à deux modes :
 *    * Le mode question : seule la question est affichée.
 *    * Le mode réponse : la question et toutes les réponses sont affichées.
 *
 * Pour passer d'un mode à l'autre, appuyez sur '/'.
 *
 * @endtaomoduledescription{TaoQuiz}
 *
 * @~
 * @{
 */

/** @name MultiChoice Question
 *
 * @~english
 * Functions to use and configure Multi choice question.
 *
 * On Tao the question and each answer are displayed. On the answer part, a
 * graph is displayed in front of each response.
 *
 * On the user device, the question and answers are shown.
 *
 * @~french
 * Fonctions pour utiliser et configurer les Questions à Choix Multiple.
 *
 * Sur Tao, la question et les réponses sont affichées. En mode 'réponse', un
 * graph est dessiné devant chaque réponses.
 *
 * Sur l'appareil de l'utilisateur, la question et les différents choix possibles sont affichés.
 */
/**@{*/


/**
 * @~english
 * MultiChoice Question
 *
 * Displays a multi choice question. Possible answers will be given with
 * @see answer.
 *
 * @param Question the text of the question.
 * @param Img the URL of the image of the question, if any.
 * @param Body answers and quiz behavior.
 *
 * @~french
 * Question à choix multiple.
 *
 * Affiche une question à choix multiple. Les réponses possibles seront données
 * par la commande @see answer.
 * @param Question Le texte de la question.
 * @param Img L'URL de l'image illustrant la question si il y en a une.
 * @param Body Les réponses possibles et paramètres de comportement du quizz.
 */
MCQ_quizz (text Question, text Img, tree Body);
MCQ_quizz (text Question, tree Body);


/**
 * @~english
 * Displays one of the possible answers for MCQ.
 *
 * @param T text of the response
 * @param Img the URL of the image, if any.
 * @param Score The score associated with this response
 *
 * @~french
 * Affiche une des réponses possibles dans les QCM.
 *
 * @param T Text de la réponse
 * @param Img L'URL de l'image illustrant la réponse.
 * @param Score Score associé à la réponse.
 */
answer (text T, integer Score);
answer (text T, text Img);
answer (text T);
answer (text T, text Img, integer Score);

/**
 * @~english
 * Get the number of vote for the answer I.
 * @param I the index of the answer (starting at 0).
 *
 * @~french
 * Retourne le nombre de vote pour la réponse I.
 * @param I L'index de la réponse.
 */
quizz_score (integer I);


/**
 * @~english
 * 'unique' mode.
 *
 * For each user, only the first answer is registered.
 *
 * @~french
 * Mode 'unique'.
 *
 * La première réponse uniquement de chaque utilisateur est enregistrée.
 */
quizz_set_unique();


/**
 * @~english
 * 'last' mode.
 *
 * For each user, only the last answer is registered.
 *
 * @~french
 * Mode 'last'.
 *
 * La dernière réponse de chaque utilisateur est enregistrée.
 */
quizz_set_last();

/**
 * @~english
 * 'multiple' mode.
 *
 * For each user, all answers are registered.
 *
 * @~french
 * Mode 'multiple'.
 *
 * Toutes les réponses sont enregistrées.
 */
quizz_set_multiple();

/**
 * @~english
 * Use bar graph to show score of each response.
 *
 * @~french
 * Utilise un histogramme devant chaque réponse pour montrer son score.
 */
bargraph();
/**
 * @~english
 * Use bar pie chart to show score of each response.
 *
 * @~french
 * Utilise un camembert devant chaque réponse pour montrer son score.
 */
piegraph();

/**@}*/

/**
 * @~english
 * Draw the number of vote.
 *
 * In question part, show only the total number of reveived vote,
 * in answer part add a pie chart showing the answers' repartition.
 *
 * @~french
 * Dessine le nombre de votes.
 *
 * Dans la partie question, montre uniquement le nombre total de vote.
 * Dans la partie réponse, donne le nombre total de votes et la répartition
 * entre les réponse avec un camenbert.
 */
total_answer_graph();

/**
 * @~english
 * Set the user feedback.
 *
 * It is what the user can see on its own screen after its answer.
 * @param T it can be one of the following values
 *    * no   -> no feedback (default value)
 *    * self -> its own answer
 *    * stat -> statistic on all answers from all users
 *
 * @~french
 * Retour utilisateur.
 *
 * Paramètre ce que voie l'utilisateur une fois qu'il a répondu à la question.
 * @param T Ce paramètre peut prendre une des valeurs suivantes :
 *    * no   -> Pas de retour (valeur par défaut)
 *    * self -> Sa propre réponse.
 *    * stat -> Statistique sur toutes les réponses.
 */
quizz_set_user_feedback (text T);

/**
 * @~english
 * Number of vote.
 *
 * Returns the total number of vote for the current question.
 *
 * @~french
 * Nombre de votes.
 *
 * Retourne le nombre total de votes.
 */
quizz_total();


/**
 * @~english
 * Deactivate the voting possibilities when result is being displayed.
 *
 * @~french
 * Stop les réponses.
 * La question et les réponses sont toujours affichées, mais les utilisateurs
 * ne peuvent plus répondre.
 */
quizz_deactivate_on_answer (integer Time);
quizz_deactivate_on_answer ();


/** @name Open Text Question
 *
 * @~english
 * Functions to use and configure open text question.
 *
 * On Tao, question part, only the question is shown. In response part, each
 * answer is displayed in a small box layout in column.
 *
 * On the user device, the question and a input text box is displayed.
 *
 * @~french
 * Fonctions pour utiliser et configurer des questions à réponses ouvertes.
 *
 * Sur Tao, en mode question, seule la question est affichée. En mode réponse,
 * chaque réponse est affichée dans une boîte de texte. Elles sont organisées
 * en colonne.
 *
 * Sur l'appareil de l'utilisateur, la question et un champ d'entrée de texte
 * sont affichés.
 */
/**@{*/

/**
 * @~english
 * Open Text Question
 *
 * Displays an open text question.
 *
 * @param Question the text of the question.
 * @param Img the URL of the image of the question, if any.
 * @param Body quiz behavior.
 *
 * @~french
 * Question à réponse ouvertes
 *
 * Affiche une question à réponse ouverte.
 *
 * @param Question Le texte de la question.
 * @param Img L'URL del'image illustrant la question.
 * @param Body Définition du comportement de la question.
 */
OTQ_quizz (text Question, text Img, tree Body);
OTQ_quizz (text Question, tree Body);

/**
 * @~english
 * Display the answers of open text question.
 *
 * @~french
 * Affiche les réponses de tous les utilisateurs.
 */
quizz_show_text_answers();

/**
 * @~english
 * X size of the text box to display.
 *
 * @~french
 * Largeur de la boîte de texte affichée.
 */
int Delta_X = 400;
/**
 * @~english
 * Y size of the text box to display.
 *
 * @~french
 * Hauteur de la boîte de texte affichée.
 */
int Delta_Y = 60;
/**
 * @~english
 * Number of text box to display per column.
 *
 * @~french
 * Nombre de boîte de texte par colonne.
 */
int BoxPerCol = 10;

/**@}*/

/** @name Mouse pad
 *
 * @~english
 * Functions to use and configure the mouse pad.
 *
 * Instead of question to answer, a mouse pad can be displayed on the user's
 * device, and the mouse motion is sent to the Tao presentation.
 *
 * The mouse pad is made of 9 squares. Except the middle one, they are each
 * settable with text and image.
 *
 * @~french
 * Fonctions pour utiliser et configurer un gestionnaire de souris.
 *
 * Au lieu d'attendre une réponse en un click, on peut faire afficher une souris
 * sur l'écran de l'utilisateur, et ses mouvements sont envoyés à
 * Tao presentations.
 *
 * L'espace souris est constitué de 9 carrés. Excepté celui du centre qui est
 * réservé à la souris, les carrés sont configurables.
 */
/**@{*/

/**
 * @~english
 * Mouse pad.
 *
 * Displays a mouse pad.
 *
 * @param Title Title or question.
 * @param Img the URL of the image, if any.
 * @param Body pad configuration.
 *
 * @~french
 * Affiche le pad.
 *
 * @param Title Titre ou question.
 * @param Img Url de l'image si nécessaire.
 * @param Body Configuration.

 */
Mouse_quizz (text Title, text Img, tree Body);
Mouse_quizz (text Title, tree Body);


/**
 * @~english
 * Configuration for squares of the mouse pad.
 * The mouse pad is made of 9 squares. Except the middle one, they are each
 * settable with text and image.
 * @param Square is combination of [tmb][lcr] for
 * Top, Middle, Bottom and Left, Center, Right.
 * @param T is the text to display in that square.
 * @param Image is the URL of the image to display in that square.
 *
 * @~french
 * Configuration des carrés.
 *
 * @param Square Une combinaison de [tmb][lcr] pour Top (Haut), Middle (Moyen),
 * Bottom (Bas), Left (Gauche), Center (Centre), Right (Droite).
 * @param T Le texte a afficher dans le carré
 * @param Image L'URL de l'image a afficher.
 */
quizz_set_mouse_pad (text Square, text T, text Image);
/**@}*/


/**
 * @}
 */
