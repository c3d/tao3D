/**
 * @addtogroup time Time and time conversions
 * @ingroup TaoBuiltins
 * This group holds time relative primitives.
 *
 * Most of those primitives uses a timestamp as input. This timestamp is a real representing a number
 *  of seconds with a precision of milliseconds. The time is given in seconds since epoch. 
 * The timestamp of the current time can be retreived by the @c time primitive.
 * @{
 */

/**
 * @fn time ()
 * @brief Return a time in second, including milliseconds.
 *
 * Returns the current document's time or the time at wich the document was frozen.
 * The returned value is a real in seconds, with millisecond precision.
 * The time is given in seconds since epoch.
 * @returns Return a fractional time, including milliseconds
 */
time ();

/**
 * @~english
 * @fn year_day ( t)
 * @brief Extract year day (1-365) from timestamp
 *
 *  Gives the day number in the year.
 *
 * @param t [real] a timestamp in seconds.
 * @returns the day number in the year.
 */
/**
 * @~french
 * @fn year_day ( t)
 * @brief Extrait du timestamp le numero du jour dans l'annee (1-365).
 *
 * Donne le numero du jour dans l'annee.
 *
 * @param t [real] le timestamp en secondes.
 * @returns le numero du jour dans l'annee.
 */
year_day ( t);

/**
 * @fn week_day ( t)
 * @brief Extract week day (1-7) from timestamp
 *
 *  Gives the day number in the week, starting on monday.
 *
 * @param t [real] a timestamp in seconds.
 * @returns the day in the week.
 */
week_day ( t);

/**
 * @fn year ( t)
 * @brief Extract year from timestamp
 *
 *  Gives the year relatively to J.C.
 *
 * @param t [real] a timestamp in seconds.
 * @return the year
 */
year ( t);

/**
 * @fn month ( t)
 * @brief Extract month (1-12) from timestamp
 *
 *  Gives the month number in the year.
 *
 * @param t [real] a timestamp in seconds.
 * @return the month number.
 */
month ( t);

/**
 * @fn day ( t)
 * @brief Extract day (1-31) from timestamp
 *
 * Gives the day number in the month.
 *
 * @param t [real] a timestamp in seconds.
 * @return the day in the month.
 */
day ( t);

/**
 * @fn hours ( t)
 * @brief Extract hours (0-23) from timestamp
 *
 *  Gives the hour from the timestamp.
 *
 * @param t [real] a timestamp in seconds.
 * @return the hour.
 */
hours ( t);

/**
 * @fn minutes ( t)
 * @brief Extract minutes (0-59) from timestamp
 *
 *  Gives the minutes from timestamp.
 *
 * @param t [real] a timestamp in seconds.
 * @return the number of minutes.
 */
minutes ( t);

/**
 * @fn seconds ( t)
 * @brief Extract seconds (0-59) from timestamp
 *
 *  Gives the seconds from timestamp.
 *
 * @param t [real] a timestamp in seconds.
 * @return the number of seconds.
 */
seconds ( t);

/**
 * @fn day ()
 * @brief Extract day (1-31) from current time
 *
 * Gives the day number in the month.
 *
 * @param t [real] a timestamp in seconds.
 * @return the day in the month.
 */
day ();

/**
 * @fn hours ()
 * @brief Return hours (0-23) of current time
 *
 *  Gives the hour of the day
 * @return the current hour.
 */
hours ();

/**
 * @fn minutes ()
 * @brief Return minutes (0-59) of current time
 *
 *  Gives the minutes in the hour
 * @return the current minutes
 */
minutes ();

/**
 * @fn month ()
 * @brief Return current month number (1-12)
 *
 *  Gives the month number in the year.
 * @return the month number.
 */
month ();

/**
 * @fn seconds ()
 * @brief Return seconds of current time (0-59)
 *
 *  Gives the seconds in the current minute.
 *  
 * @return the number of seconds.
 */
seconds ();

/**
 * @fn week_day ()
 * @brief Return current day of week (1-7)
 *
 *  Gives the day number in the week, starting on monday.
 *  
 * @returns the day in the week.
 */
week_day ();

/**
 * @fn year ()
 * @brief Return current year
 *
 *  Gives the year relatively to J.C.
 *  
 * @return the current year
 */
year ();

/**
 * @fn year_day ()
 * @~english
 * @brief Return current day in year (1-365).
 *
 *  Gives the day number in the year.
 * @returns the day number in the year.

 * @~french
 * @brief Retourne le numero du jour dans l'annee courante (1-365).
 *
 *  Donne le numero du jour dans l'annee.
 * @returns le numero du jour dans l'annee.
 */
year_day ();

/**
 * @fn page_time ()
 * @brief Returns the page time
 *
 *  Returns the time of the current page relatively to the current document's time, 
 * or relatively to the frozen time.
 * 
 * @returns Return the page time in second, with milliseconds precision.
 */
page_time ();

/**
 * @fn after ( d, c)
 * @brief Wait for time to execute code
 *
 *  Execute the given code only after the specified amount of time.
 *
 * @param d [real] The delay in second. Sensitive upto millisecond.
 * @param c [tree] The code to execute.
 * @returns The real elapsed time.
 */
after ( d, c);

/**
 * @fn every ( i, d, c)
 * @brief Code executed every interval.
 *
 *  Code executed every interval.
 *
 * @param i [real] The interval in second. Sensitive upto millisecond.
 * @param d [real] The duty in second. Sensitive upto millisecond. 
 * @param c [tree] The code to execute.
 * @returns The real elapsed time.
 */
every ( i, d, c);

/**
 * @fn every ( i, c)
 * @copybrief every(i,d,c)
 *
 *  \p duty is 0.5 when not specified.
 * @copydetails every(i,d,c)
 *
 */
every ( i, c);

/** @} */
