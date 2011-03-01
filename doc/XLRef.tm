<TeXmacs|1.0.7.9>

<style|article>

<\body>
  <doc-data|<doc-title|The XLR Programming
  Language>|<\doc-author-data|<author-name|Christophe de
  Dinechin>|<\author-address>
    Taodyne SAS, 2229 Route des Cretes, Sophia Antipolis, France
  </author-address>>
    \;
  </doc-author-data>>

  <section|Introduction>

  XLR is a dynamic language based on meta-programming, i.e. programs that
  manipulate programs. It is designed to enable a natural syntax similar to
  imperative languages descending from Algol (e.g. C, Pascal, Ada, Modula),
  while preserving the power and expressiveness of homoiconic languages
  descending from Lisp (i.e. languages where programs are data).<strong|>

  <paragraph|Benefits>Thanks to this unique approach, XLR makes it easy to
  write simple<strong|> programs, yet there is no upper limit, since you can
  extend the language to suit your own needs.

  In order to keep programs easy to write, the XLR syntax is very simple. It
  will seem very natural to most programmers, except for what it's missing :
  XLR makes little use of parentheses and other punctuation characters.
  Instead, the syntax is based on indentation, and there was a conscious
  design decision to only keep characters that had an active role in the
  meaning of the program, as opposed to a purely syntactic role. As a result,
  XLR programs look a little like pseudo-code, except of course that they can
  be compiled and run.

  This simplicity also makes meta-programming not just possible, but easy.
  All XLR programs<strong|> can be represented with just 8 data types.
  Meta-programming is the key to extensibility. As your needs grow, you won't
  find yourself limited by XLR as you would with many other programming
  languages. Instead of wishing you had this or that feature in the language,
  you can simply add it. Better yet, the process of extending the language is
  so simple that you can now consider language notations or compilation
  techniques that are useful only in a particular context. In short, with
  XLR, creating your own <em|domain-specific languages> (DSLs) is just part
  of normal, everyday programming.

  <paragraph|Examples>The key characteristics of XLR are best illustrated
  with a few short examples, going from simple programming to more advanced
  functional-style programming to simple meta-programming.

  Figure<nbsp><reference|factorial> illustrates the definition of the
  factorial function:

  <big-figure|<\verbatim>
    // Declaration of the factorial notation

    0! -\<gtr\> 1

    N! -\<gtr\> N * (N-1)!
  </verbatim>|<label|factorial>Declaration of the factorial function>

  Figure<nbsp><reference|map-reduce-filter> illustrates functions usually
  known as <em|map>, <em|reduce> and <em|filter>, which take other functions
  as arguments. In XLR, <em|map>, <em|reduce> and <em|filter> operations all
  use an infix <verbatim|with> notatoin with slightly different forms for the
  parameters :

  <big-figure|<\verbatim>
    // Map: Computing the factorial of the first 10 integers

    // The result is 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880

    (N-\<gtr\>N!) with 0..9

    \;

    // Reduce: Compute the sum of the first 5 factorials, i.e. 409114

    (X,Y -\<gtr\> X+Y) with (N-\<gtr\>N!) with 0..5

    \;

    // Filter: Displaying the factorials that are multiples of 3

    // The result is <verbatim|6, 24, 120, 720, 5040, 40320, 362880>

    (N when N mod 3 = 0) with (N-\<gtr\>N!) with 0..9
  </verbatim>|<label|map-reduce-filter>Map, reduce and filter>

  Figure<nbsp><reference|if-then> illustrates the XLR <strong|>definition of
  the <verbatim|if>-<verbatim|then>-<verbatim|else> statement, which will
  serve as our first introduction to meta-programming. Here, we tell the
  compiler how to transform a particular form of the source code (the
  if-then-else statement). Note how this transformation uses the same
  <verbatim|-\<gtr\>> notation we used to declare a factorial function in
  Figure<nbsp><reference|factorial>. This shows how, in XLR, meta-programming
  integrates transparently with regular programming.

  <big-figure|<\verbatim>
    // Declaration of if-then-else

    if true then TrueClause else FalseClause \ \ -\<gtr\> TrueClause

    if false then TrueClause else FalseClause \ -\<gtr\> FalseClause
  </verbatim>|<label|if-then>Declaration of if-then-else>

  The next sections will clarify how these operations work.

  <\section>
    Syntax
  </section>

  XLR source text is encoded using UTF-8. Source code is parsed into a an
  abstract tree known as <em|XL0>. XL0 trees consist of four literal node
  types (integer, real, text and symbol) and four structured node types
  (prefix, postfix, infix and block). Note that line breaks most often parse
  as infix operators, and that indentation most often parses as blocks.

  The precedence of operators is given by the <verbatim|xl.syntax>
  configuration file, and can also be changed dynamically in the source code.
  This is detailed in Section<nbsp><reference|precedence>.

  <subsection|Spaces and indentation>

  Spaces and tabs are not significant, but may be required to separate
  operator or name symbols. For example, there is no difference between
  <verbatim|A B> (one space) and <verbatim|A \ \ \ B> (four spaces), but it
  is different from <verbatim|AB> (zero space).

  Spaces and tabs are significant at the beginning of lines. XLR will use
  them to determine the level of indentation from which it derives program
  structures (off-side rule), as illustrated in
  Figure<nbsp><reference|off-side-rule>. Only space or tabs can be used for
  indentation, but not both in a same source file.

  <big-figure|<\verbatim>
    if A \<less\> 3 then

    \ \ \ \ write "A is too small"

    else

    \ \ \ \ write "A is too big"
  </verbatim>|<label|off-side-rule>Off-side rule: Using indentation to mark
  program structure.>

  <subsection|Comments and spaces>

  Comments are section of the source text which is not parsed. The ignored
  source text begins with a comment separator, and finishes with a comment
  terminator.

  With the default configuration<\footnote>
    In other words, with the <verbatim|xl.syntax> configuration file as
    shipped with XLR.
  </footnote>, comments are similar to C++ comments: they begin with
  <verbatim|/*> and finish with <verbatim|*/> or they begin with
  <verbatim|//> and finish at the end of line. This is illustrated in
  Figure<nbsp><reference|comments>.

  <big-figure|<\verbatim>
    // This is a single-line comment

    /* This particular comment

    \ \ \ can be placed on multiple lines */
  </verbatim>|<label|comments>Single-line and multi-line comments>

  <subsection|<label|literals>Literals>

  Four literal node types represent atomic values, i.e. values which cannot
  be decomposed into smaller units from an XLR point of view. They are:

  <\enumerate-numeric>
    <item>Integer constants

    <item>Real constants

    <item>Text literals

    <item>Symbols and names
  </enumerate-numeric>

  <paragraph|Integer constants>Integer constants<\footnote>
    At the moment, XL uses the largest native integer type on the machine
    (generally 64-bit) in its internal representations. The scanner detects
    overflow in integer constants.
  </footnote> such as <verbatim|123> consist of one or more digits
  (<verbatim|0123456789>) interpreted as unsigned radix-10 values. Note that
  <verbatim|-3> is not an integer literal but a prefix <verbatim|-> preceding
  the integer literal. The constant is defined by the longest possible
  sequence of digits.

  Integer constants can be expressed in any radix between 2 and 36. Such
  constants begin with a radix-10 integer specifying the radix, followed by a
  hash <verbatim|#> sign, followed by valid digits in the given radix. For
  instance, <verbatim|2#1001> represents the same integer constant as
  <verbatim|9>. If the radix is larger than 10, letters are used to represent
  digits following <verbatim|9>. For example, <verbatim|16#FF> is the
  hexadecimal equivalent of <verbatim|255>.

  The underscore character <verbatim|_> can be used to separate digits, but
  do not change the value being represented. For example <verbatim|1_000_000>
  is a more legible way to write <verbatim|1000000>, and
  <verbatim|16#FFFF_FFFF> is the same as <verbatim|16#FFFFFFFF>. Underscore
  characters can only separate digits, i.e. <verbatim|1__3>, <verbatim|_3> or
  <verbatim|3_> are not valid integer constants.

  <big-figure|<\verbatim>
    12

    1_000_000

    16#FFFF_FFFF

    2#1001_1001_1001_1001
  </verbatim>|Valid integer constants>

  <paragraph|Real constants>Real constants such as <verbatim|3.14> consist of
  one or more digits (<verbatim|0123456789>), followed by a single dot
  <verbatim|.> followed by one or more digits (<verbatim|0123456789>). Note
  that there must be at least one digit after the dot, i.e. <verbatim|1.> is
  not a valid real constant.

  Real constants can have a radix and use underscores to separate digits like
  integer constants. For example <verbatim|2#1.1> is the same as
  <verbatim|1.5> and <verbatim|3.141_592_653> is an approximation of
  <math|\<pi\>>.

  Optionally, a real constant can have an exponent, which consists of an
  optional hash sign <verbatim|#>, followed by the character <verbatim|e> or
  <verbatim|E>, followed by optional plus <verbatim|+> or minus <verbatim|->
  sign, followed by one or more decimal digits <verbatim|0123456789>. For
  example, <verbatim|1.0e-3> is the same as <verbatim|0.001> and
  <verbatim|1.0E3> is the same as <verbatim|1000.0>.

  The exponent value is always given in radix-10, and indicates a power of
  the given radix. For example, <verbatim|2#1.0e3> represents
  <math|2<rsup|3>>, in other words it is the same as <verbatim|8.0>.

  The hash sign is required for any radix greater than 14, since in that case
  the character <verbatim|e> or <verbatim|E> is also a valid digit. For
  instance, <verbatim|16#1.0E1> is approximately the same as
  <verbatim|1.05493>, whereas <verbatim|16#1.0#E1> is the same as
  <verbatim|16.0>.

  <\big-figure>
    <\verbatim>
      1.0

      3.1415_9265_3589_7932

      2#1.0000_0001#e-128
    </verbatim>
  </big-figure|Valid real constants>

  <paragraph|Text literals>Text is any valid UTF-8 sequence of printable or
  space characters surrounded by text delimiters, such as
  <verbatim|"Hello<nbsp>Möndé">. Except for line-terminating characters, the
  behavior when a text sequence contains control characters or invalid UTF-8
  sequences is unspecified. However, implementations are encouraged to
  preserve the contents of such sequences.

  The base text delimiters are the single quote <verbatim|'> and the double
  quote <verbatim|">. They can be used to enclose any text that doesn't
  contain a line-terminating character. The same delimiter must be used at
  the beginning and at the end of the text. For example, <verbatim|"Shouldn't
  break"> is a valid text surrounded by double quotes, and <verbatim|'He said
  "Hi"'> is a valid text surrounded by single quotes.

  In text surrounded by base delimiters, the delimiter can be inserted by
  doubling it. For instance, except for the delimiter, <verbatim|'Shouldn''t
  break'> and <verbatim|"He said ""Hi"""> are equivalent to the two previous
  examples.

  Other text delimiters can be specified, which can be used to delimit text
  that may include line breaks. Such text is called <em|long text>. With the
  default configuration, long text can be delimited with
  <verbatim|\<less\>\<less\>> and <verbatim|\<gtr\>\<gtr\>>.

  When long text contains multiple lines of text, indentation is ignored up
  to the indentation level of the first character in the long text. For
  example, Figure<nbsp><reference|long-text-indent> illustrates how long text
  indent is eliminated from the text being read.

  <big-figure|<\verbatim>
    "Hello World"

    'Où Toto élabora ce plan çi'

    \<less\>\<less\>This text spans

    multiple lines\<gtr\>\<gtr\>
  </verbatim>|Valid text constants>

  <\big-figure>
    <block*|<tformat|<table|<row|<cell|<strong|Source
    code>>|<cell|<strong|Resulting text>>>|<row|<cell| <\verbatim>
      \<less\>\<less\> Long text can

      \ \ \ contain indentation

      \ or not,

      \ \ \ \ \ it's up to you\<gtr\>\<gtr\>
    </verbatim>>|<cell| <\verbatim>
      Long text can

      contain indentation

      or not,

      \ \ it's up to you
    </verbatim>>>>>>

    \;
  </big-figure|<label|long-text-indent>Long text and indentation>

  <with|color|red|BUG: Indentation is not ignored entirely as it should in
  long text.>

  In general, comparing text literals involves only comparison of their
  value, not delimiters.

  <paragraph|Name and operator symbols>Names begin with an alphabetic
  character <verbatim|A>..<verbatim|Z> or <verbatim|a>..<verbatim|z> or any
  non-ASCII UTF-8 character, followed by the longuest possible sequence of
  alphabetic characters, digits or underscores. Two consecutive underscore
  characters are not allowed. Thus, <verbatim|Marylin_Monroe>,
  <verbatim|élaböràtion> or <verbatim|j1> are valid XLR names, whereas
  <verbatim|A-1>, <verbatim|1cm> or <verbatim|A__2> are not.

  Operator symbols, or <em|operators>, begin with an ASCII punctuation
  character<\footnote>
    Non-ASCII punctuation characters or digits are considered as alphabetic.
  </footnote> which does not act as a special delimiter for text, comments or
  blocks. For example, <verbatim|+> or <verbatim|-\<gtr\>> are operator
  symbols. An operator includes more than one punctuation character only if
  it has been declared in the syntax configuration file. For example, unless
  the symbol <verbatim|%,> has been declared in the syntax, <verbatim|3%,4%>
  will contain two operator symbols <verbatim|%> and <strong|,> instead of a
  single <verbatim|%,> operator.

  A special name, the empty name, exists only as a child of empty blocks such
  as <verbatim|()>.

  After parsing, operator and name symbols are treated identically. During
  parsing, they are treated identically except in the expression versus
  statement rule.

  <\big-figure>
    <\verbatim>
      x

      <\verbatim>
        X12_after_transformation

        <math|\<alpha\>>_times_<math|\<pi\>>

        +

        --\<gtr\>

        \<less\>\<less\>\<less\>\<gtr\>\<gtr\>\<gtr\>
      </verbatim>
    </verbatim>
  </big-figure|Examples of valid operator and name symbols>

  <subsection|Structured nodes>

  Four structured node types represent combinations of nodes. They are:

  <\enumerate>
    <item>Infix nodes, representing operations such as <verbatim|A+B> or
    <verbatim|A and B>

    <item>Prefix nodes, representing operations such as <verbatim|+3> or
    <verbatim|sin x>

    <item>Postfix nodes, representing operations such as <verbatim|3%> or
    <verbatim|3 cm>

    <item>Blocks, representing grouping such as <verbatim|(A+B)> or
    <verbatim|{lathe;rinse;repeat}>
  </enumerate>

  <paragraph|Infix nodes>An infix node has two children, one on the left, one
  on the right, separated by a name or operator symbol.

  Infix nodes are used to separate statements with semi-colons <verbatim|;>
  or line breaks <verbatim|NEWLINE>.

  <paragraph|Prefix and postfix nodes>Prefix and postfix nodes have two
  children, one on the left, one on the right, without any separator between
  them. The only difference is in what is considered the ``operation'' and
  what is considered the ``operand''. For a prefix node, the operation is on
  the left and the operand on the right, whereas for a postfix node, the
  operation is on the right and the operand on the left.

  Prefix nodes are used for functions. The default for a name or operator
  symbol that is not explicitly declared in the <verbatim|xl.syntax> file or
  configured is to be treated as a prefix function. For example,
  <verbatim|sin> in the expression <verbatim|sin x> is treated as a function.

  <paragraph|Block nodes>Block nodes have one child bracketed by two
  delimiters.

  The default configuration recognizes the following pairs as block
  delimiters:

  <\itemize>
    <item>Parentheses, as in <verbatim|(A)>

    <item>Brackets, as in <verbatim|[A]>

    <item>Curly braces, as in <verbatim|{A}>

    <item>Indentation, as shown surrounding the <verbatim|write> statements
    in Figure<nbsp><reference|off-side-rule>.
  </itemize>

  <subsection|Parsing rules>

  The XLR parser only needs a small number of rules to parse any text:

  <\enumerate>
    <item>Precedence

    <item>Associativity

    <item>Infix versus prefix versus postfix

    <item>Expression versus statement
  </enumerate>

  These rules are detailed below.

  <paragraph|Precedence>Infix, prefix, postfix and block symbols are ranked
  according to their <em|precedence>, represented as a non-negative integer.
  The precedence can be given by the syntax configuration file,
  <verbatim|xl.syntax>, or by special notations in the source code, as
  detailed in Section<nbsp><reference|precedence>.

  Symbols with higher precedence associate before symbols with lower
  precedence. For instance, if the symbol <verbatim|*> has infix precedence
  value <verbatim|300> and symbol <verbatim|+> has infix precedence value
  <verbatim|290>, then the expression <verbatim|2+3*5> will parse as an infix
  <verbatim|+> whose right child is an infix <verbatim|*>.

  The same symbol may receive a different precedence as an infix, as a prefix
  and as a postfix operator. For example, if the precedence of <verbatim|->
  as an infix is <verbatim|290> and the precedence of <verbatim|-> as a
  prefix is <verbatim|390>, then the expression <verbatim|3 - -5> will parse
  as an infix <verbatim|-> with a prefix <verbatim|-> as a right child.

  The precedence associated to blocks is used to define the precedence of the
  resulting expression. This precedence is used primarily in the ``expression
  versus statement'' rule.

  <paragraph|Associativity>Infix operators can associate to their left or to
  their right.

  The addition operator is traditionally left-associative, meaning that in
  <verbatim|A+B+C>, <verbatim|A> and <verbatim|B> associate before
  <verbatim|C>. The outer infix node has an infix node as its left child
  (with <verbatim|A> and <verbatim|B> as their children) and <verbatim|C> as
  its right child.

  Conversely, the semi-colon in XLR is right-associative, meaning that
  <verbatim|A;B;C> is an infix node with an infix as the right child and
  <verbatim|A> as the left child.

  Operators with left and right associativity cannot have the same
  precedence. To ensure this, XLR gives an even precedence to
  left-associative operators, and an odd precedence to right-associative
  operators. For example, the precedence of <verbatim|+> in the default
  configuration is <verbatim|290> (left-associative), whereas the precedence
  of <verbatim|^> is <verbatim|395> (right-associative).

  <paragraph|Infix versus Prefix versus Postfix>During parsing, XLR needs to
  resolve ambiguities between infix and prefix symbols. For example, in
  <verbatim|-A + B>, the minus sign <verbatim|-> is a prefix, whereas the
  plus sign <verbatim|+> is an infix. Similarly, in <verbatim|A and not B>,
  the <verbatim|and> word is infix, whereas the <verbatim|not> word is
  prefix. The problem is exactly similar for names and operator symbols.

  XLR resolves this ambiguity as follows<\footnote>
    All the examples are given based on the default <verbatim|xl.syntax>
    configuration file.
  </footnote>:

  <\itemize>
    <item>The first symbol in a statement or in a block is a prefix:
    <verbatim|and> in <verbatim|(and x)> is a prefix.

    <item>A symbol on the right of an infix symbol is a prefix:
    <verbatim|and> in <verbatim|A+and B> is a prefix.

    <item>Otherwise, if the symbol has an infix precedence but no prefix
    precedence, then it is interpreted as an infix: <verbatim|and> in
    <verbatim|A and B> is an infix.

    <item>If the symbol has an infix precedence and a prefix precedence, and
    either a space following it, or no space preceding it, then it is an
    infix: the minus sign <verbatim|-> in <verbatim|A - B> is an infix, but
    the same character is a prefix in <verbatim|A -B>.

    <item>Otherwise, if the symbol has a postfix precedence, then it is a
    postfix: <verbatim|%> in <verbatim|3%> is a postfix.

    <item>Otherwise, the symbol is a prefix: <verbatim|sin> in
    <verbatim|3+sin x> is a prefix.
  </itemize>

  In the first, second and last case, a symbol may be identified as a prefix
  without being given an explicit precedence. Such symbols are called
  <em|default prefix>. They receive a particular precedence known as
  <em|function precedence>.

  <paragraph|Expression versus statement>Another ambiguity is related to the
  way humans read text. In <verbatim|write sin x, sin y>, most humans will
  read this as a <verbatim|write> instruction taking two arguments. This is
  however not entirely logical: if <verbatim|write> takes two arguments, then
  why shouldn't <verbatim|sin> also take two arguments? In other words, why
  should this example parse as <verbatim|write(sin(x),sin(y))> and not as
  <verbatim|write(sin(x,sin(y)))>?

  The reason is that we tend to make a distinction between ``statements'' and
  ``expressions''. This is not a distinction that is very relevant to
  computers, but one that exists in most natural languages, which distinguish
  whole sentences as opposed to subject or complement.

  XLR resolves the ambiguity by implementing a similar distinction. The
  boundary is a particular infix precedence, called <em|statement
  precedence>. Intuitively, infix operators with a lower precedence separate
  statements, whereas infix operators with a higher precedence separate
  expressions. For example, the semi-colon <verbatim|;> or <verbatim|else>
  separate statements, whereas <verbatim|+> or <verbatim|and> separate
  instructions.

  More precisely:

  <\itemize>
    <item>If a block's precedence is above statement precedence, its content
    begins as an expression, otherwise it begins as a statement: <verbatim|3>
    in <verbatim|(3)> is an expression, <verbatim|write> in
    <verbatim|{write}> is a statement.

    <item>Right after an infix symbol with a precedence lower than statement
    precedence, we are in a statement, otherwise we are in an expression. The
    name <verbatim|B> in <verbatim|A+B> is an expression, but it is a
    statement in <verbatim|A;B>.

    <item>A similar rule applies after prefix nodes: <verbatim|{X} write A,
    B> will give two arguments to <verbatim|write>, whereas in
    <verbatim|(x-\<gtr\>x+1) sin x, y> the <verbatim|sin> function only
    receives a single argument.

    <item>A default prefix begins a statement if it's a name, an expression
    if it's a symbol: the name <verbatim|write> in <verbatim|write X> begins
    a statement, the symbol <verbatim|+> in <verbatim|+3> begins an
    expression.
  </itemize>

  <subsection|<label|precedence>Syntax configuration>

  The default XLR syntax configuration file looks like
  Figure<nbsp><reference|syntax-file>.

  <big-figure|<\verbatim>
    INFIX

    \ \ \ \ \ \ \ \ 11 \ \ \ \ \ NEWLINE

    \ \ \ \ \ \ \ \ 21 \ \ \ \ \ ;

    \ \ \ \ \ \ \ \ 25 \ \ \ \ \ -\<gtr\>

    \ \ \ \ \ \ \ \ 30 \ \ \ \ \ loop when

    \ \ \ \ \ \ \ \ 35 \ \ \ \ \ else into

    \ \ \ \ \ \ \ \ 40 \ \ \ \ \ then require ensure

    \ \ \ \ \ \ \ \ 41 \ \ \ \ \ with

    \ \ \ \ \ \ \ \ 100 \ \ \ \ STATEMENT

    \ \ \ \ \ \ \ \ 110 \ \ \ \ is as \ while until

    \ \ \ \ \ \ \ \ 120 \ \ \ \ written

    \ \ \ \ \ \ \ \ 130 \ \ \ \ where

    \ \ \ \ \ \ \ \ 200 \ \ \ \ DEFAULT

    \ \ \ \ \ \ \ \ 210 \ \ \ \ ,

    \ \ \ \ \ \ \ \ 221 \ \ \ \ := += -= *= /= ^= \|= &=

    \ \ \ \ \ \ \ \ 230 \ \ \ \ return\ 

    \ \ \ \ \ \ \ \ 240 \ \ \ \ and or xor

    \ \ \ \ \ \ \ \ 250 \ \ \ \ in at

    \ \ \ \ \ \ \ \ 251 \ \ \ \ of to

    \ \ \ \ \ \ \ \ 260 \ \ \ \ ..

    \ \ \ \ \ \ \ \ 270 \ \ \ \ = \<less\> \<gtr\> \<less\>= \<gtr\>=
    \<less\>\<gtr\>

    \ \ \ \ \ \ \ \ 280 \ \ \ \ & \|

    \ \ \ \ \ \ \ \ 290 \ \ \ \ + -

    \ \ \ \ \ \ \ \ 300 \ \ \ \ * / mod rem

    \ \ \ \ \ \ \ \ 395 \ \ \ \ ^

    \ \ \ \ \ \ \ \ 510 \ \ \ \ .

    \ \ \ \ \ \ \ \ 520 \ \ \ \ :

    \;

    PREFIX

    \ \ \ \ \ \ \ \ 30 \ \ \ \ \ data

    \ \ \ \ \ \ \ \ 120 \ \ \ \ exit loop

    \ \ \ \ \ \ \ \ 125 \ \ \ \ case if return while until yield transform

    \ \ \ \ \ \ \ \ 320 \ \ \ \ not in out constant variable const var

    \ \ \ \ \ \ \ \ 340 \ \ \ \ @\ 

    \ \ \ \ \ \ \ \ 380 \ \ \ \ ! ~

    \ \ \ \ \ \ \ \ 390 \ \ \ \ - + * /

    \ \ \ \ \ \ \ \ 400 \ \ \ \ FUNCTION

    \ \ \ \ \ \ \ \ 410 \ \ \ \ function procedure to type iterator

    \ \ \ \ \ \ \ \ 500 \ \ \ \ &

    \;

    POSTFIX

    \ \ \ \ \ \ \ \ 400 \ \ \ \ ! ? % cm inch mm pt px

    BLOCK

    \ \ \ \ \ \ \ \ 5 \ \ \ \ \ \ INDENT UNINDENT

    \ \ \ \ \ \ \ \ 21 \ \ \ \ \ '{' '}'

    \ \ \ \ \ \ \ \ 400 \ \ \ \ '(' ')' '[' ']'

    \;

    TEXT

    \ \ \ \ \ \ \ \ "\<less\>\<less\>" "\<gtr\>\<gtr\>"

    \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 

    COMMENT

    \ \ \ \ \ \ \ \ "//" NEWLINE

    \ \ \ \ \ \ \ \ "/*" "*/"
  </verbatim>|<label|syntax-file>Default syntax configuration file>

  Spaces and indentation are not significant in a syntax configuration file.
  Lexical elements are identical to those of XLR, as detailed in
  Section<nbsp><reference|literals>.

  The significant elements are integer constants, names, symbols and text.
  Integer constants are interpreted as the precedence of names and symbols
  that follow them. Name and symbols can be given either with lexical names
  and symbols, or with text. A few names are reserved for use as keywords in
  the syntax configuration file:

  <\itemize>
    <item><verbatim|INFIX> begins a section declaring infix symbols and
    precedence. In this section:

    <\itemize>
      <item><verbatim|NEWLINE> identifies line break characters in the source
      code

      <item><verbatim|STATEMENT> identifies the precedence of statements

      <item><verbatim|DEFAULT> identifies the precedence for symbols not
      otherwise given a precedence. This precedence should be unique.
    </itemize>

    <item><verbatim|PREFIX> begins a section declaring prefix symbols and
    precedence. In this section:

    <\itemize>
      <item><verbatim|FUNCTION >identifies the precedence for default prefix
      symbols, i.e. symbols identified as prefix that are not otherwise given
      a precedence.
    </itemize>

    <item><verbatim|POSTFIX> begins a section declaring postfix symbols and
    precedence.

    <item><verbatim|BLOCK> begins a section declaring block delimiters and
    precedence. In this section:

    <\itemize>
      <item><verbatim|INDENT> and <verbatim|UNINDENT> are used to mark
      indentation and unindentation.
    </itemize>

    <item><verbatim|TEXT> begins a section declaring delimiters for long
    text.

    <item><verbatim|COMMENT> begins a section declaring delimiters for
    comments. In this section:

    <\itemize>
      <item><verbatim|NEWLINE> identifies line breaks
    </itemize>
  </itemize>

  Syntax information can also be provided in the source code using the
  <verbatim|syntax> name followed by a block, as illustrated in
  Figure<nbsp><reference|source-syntax>.

  <big-figure|<\verbatim>
    // Declare infix 'weight' operator

    syntax (INFIX 350 weight)

    A weight B -\<gtr\> A = B

    \;

    // Declare postfix 'apples' and 'kg'

    syntax

    \ \ \ \ POSTFIX 390 apples kg

    X kg -\<gtr\> X * 1000

    N apples -\<gtr\> N * 0.250 kg

    \;

    // Combine the notations

    if 6 apples weight 1.5 kg then

    \ \ \ \ write "Success!"
  </verbatim>|<label|source-syntax>Use of the <verbatim|syntax> specification
  in a source file>

  <section|Semantics>

  The semantics of XLR is based entirely on the rewrite of XL0 abstract
  syntax trees. The rewrite rules are based on a very limited number set of
  operators. The rewrites operations combine to result in the execution of an
  XLR program.

  <subsection|Operators>

  XLR defines a small number of operators:

  <\itemize>
    <item>Rewrite declarations are used to declare operations. They roughly
    play the role of functions and operator declarations in other programming
    languages.

    <item>Data declarations identify data structures in the program.

    <item>Type declarations define the type of variables.

    <item>Guards limit the validity of rewrite or data declarations.

    <item>Assignment allow a variable value to change over time.

    <item>Sequence operators indicate the order in which computations must be
    performed.
  </itemize>

  <paragraph|Rewrite declarations>The infix <verbatim|-\<gtr\>> operator
  declares a tree rewrite. Figure<nbsp><reference|if-then-else> repeats the
  code in Figure<nbsp><reference|if-then> illustrating how rewrite
  declarations can be used to define the traditional
  <verbatim|if>-<verbatim|then>-<verbatim|else> statement.

  <big-figure|<\verbatim>
    if true then TrueClause else FalseClause \ \ \ -\<gtr\> TrueClause

    if false then TrueClause else FalseClause \ \ -\<gtr\> FalseClause
  </verbatim>|<label|if-then-else>Examples of tree rewrites>

  The tree on the left of the <verbatim|-\<gtr\>> operator is called the
  <em|pattern>. The tree on the right is called the <em|implementation> of
  the pattern. The rewrite declaration indicates that a tree that matches the
  pattern should be rewritten using the implementation.

  The pattern contains <em|constant> and <em|variable> symbols and names:

  <\itemize>
    <item>Infix symbols and names are constant

    <item>Block-delimiting symbols and names are constant

    <item>A name on the left of a prefix is a constant

    <item>A name on the right of a postfix is a constant

    <item>All other names are variable
  </itemize>

  Figure<nbsp><reference|if-then-else-colorized> highlight in blue italic
  type all variable symbols in the declarations of
  Figure<nbsp><reference|if-then-else>.

  <big-figure|<\verbatim>
    if <with|color|blue|<em|true>> then <with|color|blue|<em|TrueClause>>
    else <with|color|blue|<em|FalseClause>> \ \ \ -\<gtr\> TrueClause

    if <em|<with|color|blue|false>> then <em|<with|color|blue|TrueClause>>
    else <em|<with|color|blue|FalseClause>> \ \ -\<gtr\> FalseClause
  </verbatim>|<label|if-then-else-colorized>Examples of tree rewrites>

  Constant symbol and names form the structure of the pattern, whereas
  variable names form the parts of the pattern which can match other trees.
  For example, to match the pattern in Figure<nbsp><reference|if-then-else>,
  the <verbatim|if>, <verbatim|then> and <verbatim|else> words must match
  exactly, but <verbatim|TrueClause> may match any tree, like for example
  <verbatim|write "Hello">.

  A special case is a rewrite declaration where the pattern consists of a
  single name.\ 

  <\itemize>
    <item>If the rewrite declaration is the only child of a block, the
    pattern name is variable, and the result is called a <em|universal
    rewrite rule>, i.e. a rule that applies to any tree. For example
    <verbatim|(x-\<gtr\>x+1)> is a universal rewrite rule that adds one to
    any input tree.

    <item>Otherwise, the name is constant, and the rewrite is said to be a
    <em|named tree>.
  </itemize>

  <paragraph|Data declaration>The <verbatim|data> prefix declares tree
  structures that need not be rewritten further. For instance,
  Figure<nbsp><reference|comma-separated-list> declares that <verbatim|1,3,4>
  should not be evaluated further, because it is made of infix <verbatim|,>
  trees which are declared as <verbatim|data>.

  <\big-figure>
    <\verbatim>
      data a, b
    </verbatim>
  </big-figure|<label|comma-separated-list>Declaring a comma-separated list>

  The tree following a data declaration is a pattern, with constant and
  variable symbols like for rewrite declarations. In
  Figure<nbsp><reference|complex-type>, the names <verbatim|x> and
  <verbatim|y> are variable, but the name <verbatim|complex> is constant
  because it is a prefix. Assuming that addition is implemented for integer
  values, <verbatim|complex(3+4, 5+6)> will evaluate as
  <verbatim|complex(7,11)> but no further. The declaration in
  Figure<nbsp><reference|complex-type> can therefore be interpreted as
  declaring the <verbatim|complex> data type.

  <big-figure|<verbatim|data complex(x,y)>|<label|complex-type>Declaring a
  <verbatim|complex> data type>

  <paragraph|Type declaration>The infix colon <verbatim|:> operator in a
  rewrite or data patten introduces a <em|type declaration>. It indicates
  that the left node has the type indicated by the right node. Types in XLR
  are explained in Section<nbsp><reference|types>.

  Figure<nbsp><reference|type-declaration> shows examples of type
  declarations. The first line declares a <verbatim|complex> type where the
  coordinates must be <verbatim|real>. The second line declares an addition
  between a <verbatim|real> number called <verbatim|a> and an
  <verbatim|integer> number called <verbatim|b> implementing by calling the
  <verbatim|integer_to_real> function on <verbatim|b> (which presumably
  promotes the integer to a real number).

  <big-figure|<\verbatim>
    <\verbatim>
      data complex(x:real, y:real)

      a:real+b:integer -\<gtr\> a + integer_to_real b
    </verbatim>
  </verbatim>|<label|type-declaration>Simple type declarations>

  <with|color|red|REVISIT: Role of type declarations outside of a pattern, if
  any.>

  <paragraph|Guards>The infix <verbatim|when> operator in a rewrite or data
  pattern introduces a <em|guard>, i.e. a boolean condition that must be true
  for the pattern to apply.

  Figure<nbsp><reference|guard> shows an improved definition of the factorial
  function which only applies for non-negative values. Note that the
  definition does not give a type to the variable <verbatim|N>, making it
  possible for <verbatim|N> to be for example a <verbatim|big_integer>.

  <big-figure|<\verbatim>
    0! \ \ \ \ \ \ \ \ \ \ \ -\<gtr\> 1

    N! when N \<gtr\> 0 -\<gtr\> N * (N-1)!
  </verbatim>|<label|guard>Guard limit the validity of operations>

  <with|color|magenta|TODO: Guards are not currently implemented.>

  <paragraph|Assignment>The assignment operator <verbatim|:=> binds the
  variable on its left to the tree on its right. The variable must not
  already be bound to a named tree. In other words, it is not legal to write
  <verbatim|A:=5> following <verbatim|A-\<gtr\>3>.

  If the left side of the assignment is a simple name, no evaluation of the
  tree on the right is done as a result of the assignment. However, if the
  variable name is evaluated for any reason, the variable will subsequently
  be bound to the evaluated result.

  An assignment is valid even if the variable has not previously been bound.
  Assignments declare the variable being assigned to in the code following
  the assignment.

  If the left side of an assignment is a type declaration, the assigned value
  is evaluated and the result compared against the type. Furthermore, any
  subsequent assignment to the same variable will also evaluate the assigned
  value and compare it against the type. If the type doesn't match, the
  assignment will be equivalent to evaluating <verbatim|type_error X>, where
  <verbatim|X> is the tree for the assignment.

  If the left side of an assignment is not a name nor a type declaration, the
  assignment expression follows the normal evaluation rules described in
  Section<nbsp><reference|evaluation>.

  Using an assignment in an expression is equivalent to using the value bound
  to the variable after the assignment. For instance, <verbatim|sin(x:=f(0))>
  is equivalent to <verbatim|x:=f(0)> followed by <verbatim|sin(x)>.

  <with|color|magenta|TODO: Assignment is not currently implemented>

  <\with|color|red>
    REVISIT: What if the left-hand side of assignment is a rewrite? Consider
    this:

    <big-figure|<\verbatim>
      array 0 -\<gtr\> A

      array 1 -\<gtr\> B

      array X := 3
    </verbatim>|<label|array-assign>Assignment to an array>
  </with>

  <paragraph|Sequences>The infix line-break <verbatim|NEWLINE> and semi-colon
  <verbatim|;> operators are used to introduce a sequence between statements.
  They ensure that the left node is evaluated entirely before the evaluation
  of the right node begins.

  Figure<nbsp><reference|sequence> for instance guarantees that the code will
  first <verbatim|write> "A", then <verbatim|write "B">, then write the
  result of the sum <verbatim|f(100)+f(200)>. However, the implementation is
  entirely free to compute <verbatim|f(100)> and <verbatim|f(200)> in any
  order, including in parallel.

  <big-figure|<\verbatim>
    write "A"; write "B"

    write f(100)+f(200)
  </verbatim>|<label|sequence>Code writing <verbatim|A>, then <verbatim|B>,
  then <verbatim|f(100)+f(200)>>

  Note: tasking and threading operations are not defined yet, but they need
  not be defined in the core language as described in this document (any more
  than I/O operations).

  <subsection|<label|evaluation>Evaluation>

  Except for special forms, the evaluation of XLR trees is performed as
  follows:

  <\enumerate>
    <item>The tree to evaluate, <verbatim|T>, is matched against the
    available data and rewrite pattern. <verbatim|3*4+5> will match
    <verbatim|A*B+C> as well as <verbatim|A+B> (since the outermost tree is
    an infix <verbatim|+> as in <verbatim|A+B>).

    <item>Possible matches are tested in <em|scoping order> (defined below)
    against the tree to evaluate. The first matching tree is selected. For
    example, in Figure<nbsp><reference|factorial>, <verbatim|(N-1)!> will be
    matched against the rules <verbatim|0!> and <verbatim|N!> in this order.

    <item>Nodes in each candidate pattern <verbatim|P> are compared to the
    tree <verbatim|T> as follows:

    <\itemize>
      <item>Constant symbols or names in <verbatim|P> are compared to the
      corresponding element in <verbatim|T> and must match exactly. For
      example, the <verbatim|+> symbol in pattern <verbatim|A+B> will match
      the plus <verbatim|+> symbol in expression <verbatim|3*4+5> but not the
      minus <verbatim|-> symbol in <verbatim|3-5>.

      <item>Variables names in <verbatim|P> that are not bound to any value
      and are not part of a type declaration are bound to the corresponding
      fragment of the tree in <verbatim|T>. For example, for the expression
      <verbatim|3!>, the variable <verbatim|N> in
      Figure<nbsp><reference|factorial> will be bound to <verbatim|3>.

      <item>Variable names in <verbatim|P> that are bound to a value are
      compared to the corresponding tree fragment in <verbatim|T> after
      evaluation. For instance, if <verbatim|true> is bound at the point of
      the declaration in Figure<nbsp><reference|if-then-else>, the test
      <verbatim|if A\<less\>3 then X else Y> requires the evaluation of the
      expression <verbatim|A\<less\>3>, and the result will be compared
      against <verbatim|true>.

      <item>This rule applies even if the binding occured in the same
      pattern. For example, the pattern <verbatim|A+A> will match
      <verbatim|3+3> but not <verbatim|3+4>, because <verbatim|A> is first
      bound to <verbatim|3> and then cannot match <verbatim|4>. The pattern
      <verbatim|A+A> will also match <verbatim|(3-1)+(4-2)>: although
      <verbatim|A> may first be bound to the unevaluated value
      <verbatim|3-1>, verifying if the second <verbatim|A> matches requires
      evaluation both <verbatim|A> and the test value.

      <item>Type declarations in <verbatim|P> match if the result of
      evaluating the corresponding fragment in <verbatim|T> has the declared
      type. In that case, the variable being declared is be bound to the
      evaluated value.

      <item>Constant values (integer, real and text) in <verbatim|P> are
      compared to the corresponding fragment of <verbatim|T> after
      evaluation. For example, in Figure<nbsp><reference|factorial>, when the
      expression <verbatim|(N-1)!> is compared against <verbatim|0!>, the
      expression <verbatim|(N-1)> is evaluated in order to be compared to
      <verbatim|0>.

      <item>Infix, prefix and postfix nodes in <verbatim|P> are compared to
      the matching node in <verbatim|T> by comparing their children in
      depth-first, left to right order.
    </itemize>

    The comparison process may cause fragments of the tree to be evaluated.
    Each fragment is evaluated at most once for the process of evaluating the
    tree <verbatim|T>. Once the fragment has been evaluated, the evaluated
    value will be used in any subsequent comparison or variable binding. For
    example, when computing <verbatim|F(3)!>, the evaluation of
    <verbatim|F(3)> is required in order to compare to <verbatim|0!>,
    guaranteeing that <verbatim|N> in <verbatim|N!> will be bound to the
    evaluated value if <verbatim|F(3)> is not equal to <verbatim|0>.

    <item>If there is no match found between any pattern <verbatim|P> and the
    tree to evaluate <verbatim|T>:

    <\itemize>
      <item>Terminal nodes (integer, real, text, names and symbols) evaluate
      as themselves.

      <item>Other nodes <verbatim|X> evaluate as <verbatim|evaluation_error
      X>, which is supposed to implement error handling during evaluation.

      <\with|color|magenta>
        TODO: Implemented as <verbatim|error "No form matches $1", X> today,
        which doesn't allow fine-grained error checking.

        REVISIT: There are a few other options:

        <\itemize>
          <item>Evaluate children

          <item>Evaluate children and retry top-level evaluation (unbounded?)

          <item>Return the tree unchanged
        </itemize>
      </with>
    </itemize>

    <item>Otherwise, variables in the first matching pattern are bound to the
    corresponding fragment of the tree to evaluate. If the fragment was
    evaluated (including as required for comparison with an earlier pattern),
    then the variable is bound with the evaluated version. If the fragment
    was never evaluated, the variable is bound with the tree fragment.

    <item>The implementation corresponding to the pattern in the previous
    step is evaluated.
  </enumerate>

  <paragraph|Special forms>Some forms have a special meaning and are
  evaluated specially:

  <\enumerate>
    <item>A terminal node (integer, real, type, name) evaluates as itself,
    unless there is an explicit rewrite rule for it<\footnote>
      There several use cases for allowing rewrite rules for integer, real or
      text constants, notably to implement data maps such as
      <verbatim|(1-\<gtr\>0; 0-\<gtr\>1)>.
    </footnote>.

    <item>A rewrite rule evaluates as itself.

    <item>A data declaration evaluates as itself

    <item>An assignment binds the variable and evaluates as the named
    variable after assignment

    <item>A prefix with a universal rewrite rule on the left and a tree on
    the right binds the tree on the right to the pattern variable and
    evaluates the implementation. For instance, the prefix
    <verbatim|(x-\<gtr\>x+1) 3> evaluates as <verbatim|4>.
    <with|color|magenta|TODO: Not implemented yet>

    <item>A sequence evaluates the left tree, then evaluates the right tree,
    and evaluates as the result of the latter evaluation.
  </enumerate>

  <subsection|<label|binding>Variable binding and scoping>

  Binding a variable means that the variable references a particular tree.
  The binding is valid for a particular <em|scope>.

  <\itemize>
    <item>For patterns, the scope of a binding includes tree nodes that
    follow the first occurence of the name in the source code, i.e. they
    would be found later in a left-to-right depth-first traversal.

    <item>For rewrite declarations, the scope of the binding includes the
    pattern as described above as well as the implementation.

    <item>For data declarations, the scope of the binding includes the
    pattern as described above.

    <item>For assignments, the scope begins at the assignment and finishes at
    the end of the innermost block in which the assignment first happens.
  </itemize>

  <subsection|<label|types>Types>

  Types are expressions that appear on the right of the colon operator
  <verbatim|:> in type declarations. In XLR, a type identifies the <em|shape>
  of a tree. A value is said to <em|belong> to a type if it matches the shape
  defined by the type.

  <with|color|magenta|REVISIT: The type system is still largely
  unimplemented.>

  <paragraph|Predefined types>The following predefined types are defined like
  the built-in operations described in Section<nbsp><reference|built-ins>:

  <\itemize>
    <item><verbatim|integer> matches integer constants

    <item><verbatim|real> matches real constants

    <item><verbatim|text> matches text constants

    <item><verbatim|symbol> matches names and operator symbols

    <item><verbatim|name_symbol> matches names

    <item><verbatim|infix> matches infix trees

    <item><verbatim|prefix> matches prefix trees

    <item><verbatim|postfix> matches postfix trees

    <item><verbatim|block> matches block trees

    <item><verbatim|tree> matches any tree
  </itemize>

  Note that <verbatim|tree> is treated specially in type declarations in that
  they do not require evaluation of the corresponding tree. In other words,
  <verbatim|x:tree> is equivalent to <verbatim|x> in a pattern.

  <paragraph|Value types>Integer, real and text values can be used as types
  that have a single element, the exact value. For example, <verbatim|x:37>
  is a type declaration which only matches when <verbatim|x> is bound to the
  <verbatim|integer> value <verbatim|37>. This is mostly useful in structure
  and union types.

  <paragraph|Block type>If <verbatim|T> is a type, then any block with
  <verbatim|T> as a child such as <verbatim|(T)> is the same type. Using the
  notation <verbatim|(T)> may be required for precedence reasons.

  <paragraph|Union type>If <verbatim|T1> and <verbatim|T2> are types, then
  <verbatim|T1\|T2> is a type that matches any value belonging either to
  <verbatim|T1> or to <verbatim|T2>. For example, <verbatim|x:(0\|1\|2\|3)>
  indicates that <verbatim|x> can be any of the four values.

  <paragraph|Structure types>If <verbatim|P> is pattern (in the same sense as
  for rewrite rules), <verbatim|type(P)> is a type matching the pattern. For
  example, <verbatim|type(X,Y)> matches <verbatim|3,5> or <verbatim|A+3,write
  X>. Note that <verbatim|type(X\|Y)> matches an infix tree with the
  <verbatim|\|> name, whereas <verbatim|X\|Y> matches type <verbatim|X> or
  type <verbatim|Y>.

  <paragraph|Named types>A complex type expression can be bound to a name,
  which can then be used as a replacement for the entire type expression. For
  example, one can declare a <verbatim|number> type that accepts
  <verbatim|integer> and <verbatim|real> numbers using code similar to
  Figure<nbsp><reference|type-name>:

  <\big-figure>
    <\verbatim>
      number -\<gtr\> integer \| real

      increment x:number -\<gtr\> x+1
    </verbatim>
  </big-figure|<label|type-name>Binding a type expression to a name>

  <paragraph|Rewrite types>The infix <verbatim|-\<gtr\>> operator can be used
  in type expressions to denote rewrites that perform a particular kind of
  tree transformation. Figure<nbsp><reference|rewrite-type> illustrates this
  usage:

  <big-figure|<verbatim|adder : (number + number -\<gtr\>
  number)>|<label|rewrite-type>Declaration of a rewrite type>

  <with|color|magenta|REVISIT: Not sure about the semantics of rewrite
  types.>

  <subsection|<label|built-ins>Built-in operations>

  A number of operations are defined by the core run-time of the language,
  and appear in a scope that precedes any XLR program.

  This section decsribes the minimum list of operations available in any XLR
  program. Operator priorities are defined by the <verbatim|xl.syntax> file
  in Figure<nbsp><reference|syntax-file>. All operations listed in this
  section may be implemented specially in the compiler, or using regular
  rewrite rules defined in a particular file called <verbatim|builtins.xl>
  that is loaded by XLR before evaluating any program, or a combination of
  both.

  <paragraph|Arithmetic>Arithmetic operators for <verbatim|integer> and
  <verbatim|real> values are listed in Table<nbsp><reference|arithmetic>,
  where <verbatim|x> and <verbatim|y> denote integer or real values.
  Arithmetic operators take arguments of the same type and return an argument
  of the same type. In addition, the power operator <strong|^> can take a
  first <verbatim|real> argument and an <verbatim|integer> second argument.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x+y>>|<cell|Addition>>|<row|<cell|<verbatim|x-y>>|<cell|Subtraction>>|<row|<cell|<verbatim|x*y>>|<cell|Multiplication>>|<row|<cell|<verbatim|x/y>>|<cell|Division>>|<row|<cell|<verbatim|x
  rem y>>|<cell|Remainder>>|<row|<cell|<verbatim|x mod
  y>>|<cell|Modulo>>|<row|<cell|<verbatim|x^y>>|<cell|Power>>|<row|<cell|<verbatim|-x>>|<cell|Negation>>|<row|<cell|<verbatim|x%>>|<cell|Percentage>>>>>|<label|arithmetic>Arithmetic
  operations>

  <paragraph|Comparison>Comparison operators can take <verbatim|integer>,
  <verbatim|real> or <verbatim|text> argument, both arguments being of the
  same type, and return a <verbatim|boolean> argument, which can be either
  <verbatim|true> or <verbatim|false>. Text is compared using the
  lexicographic order<\footnote>
    There is currently no locale-dependent text comparison.
  </footnote>.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x=y>>|<cell|Equal>>|<row|<cell|<verbatim|x\<less\>\<gtr\>y>>|<cell|Not
  equal>>|<row|<cell|<verbatim|x\<less\>y>>|<cell|Less-than>>|<row|<cell|<verbatim|x\<gtr\>y>>|<cell|Greater
  than>>|<row|<cell|<verbatim|x\<less\>=y>>|<cell|Less or
  equal>>|<row|<cell|<verbatim|x\<gtr\>=y>>|<cell|Greater or
  equal>>>>>|<label|arithmetic>Comparisons>

  <paragraph|Bitwise arithmetic>Bitwise operators operate on the binary
  representation of <verbatim|integer> values, treating each bit indivudally.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x\<less\>\<less\>y>>|<cell|Shift
  <verbatim|x> left by <verbatim|y> bits>>|<row|<cell|<verbatim|x\<gtr\>\<gtr\>y>>|<cell|Shift
  <verbatim|x> right by <verbatim|y> bits>>|<row|<cell|<verbatim|x and
  y>>|<cell|Bitwise and>>|<row|<cell|<verbatim|x or y>>|<cell|Bitwise
  or>>|<row|<cell|<verbatim|x xor y>>|<cell|Bitwise exclusive
  or>>|<row|<cell|<verbatim|not x>>|<cell|Bitwise
  complement>>>>>|<label|bitwise-arithmetic>Bitwise arithmetic operations>

  <paragraph|Boolean operations>Boolean operators operate on the names
  <verbatim|true> and <verbatim|false>.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x=y>>|<cell|Equal>>|<row|<cell|<verbatim|x\<less\>\<gtr\>y>>|<cell|Not
  equal>>|<row|<cell|<verbatim|x and y>>|<cell|Logical
  and>>|<row|<cell|<verbatim|x or y>>|<cell|Logical
  or>>|<row|<cell|<verbatim|x xor y>>|<cell|Logical exclusive
  or>>|<row|<cell|<verbatim|not x>>|<cell|Logical
  not>>>>>|<label|boolean-operations>Boolean operations>

  <paragraph|Mathematical functions>Mathematical functions operate on
  <verbatim|real> numbers. The <verbatim|random> function can also take two
  <verbatim|integer> arguments, in which case it returns an
  <verbatim|integer> value.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|sqrt
  x>>>>>>|<cell|Square root>>|<row|<cell|<verbatim|sin
  x>>|<cell|Sine>>|<row|<cell|<verbatim|cos
  x>>|<cell|Cosine>>|<row|<cell|<verbatim|tan
  x>>|<cell|Tangent>>|<row|<cell|<verbatim|asin
  x>>|<cell|Arc-sine>>|<row|<cell|<verbatim|acos
  x>>|<cell|Arc-cosine>>|<row|<cell|<verbatim|atan
  x>>|<cell|Arc-tangent>>|<row|<cell|<verbatim|atan(y,x)>>|<cell|Coordinates
  arc-tangent (<verbatim|atan2> in C)>>|<row|<cell|<verbatim|exp
  x>>|<cell|Exponential>>|<row|<cell|<verbatim|expm1 x>>|<cell|Exponential
  minus one>>|<row|<cell|<verbatim|log x>>|<cell|Natural
  logarithm>>|<row|<cell|<verbatim|log2 x>>|<cell|Base 2
  logarithm>>|<row|<cell|<verbatim|log10 x>>|<cell|Base 10
  logarithm>>|<row|<cell|<verbatim|log1p x>>|<cell|Log of one plus
  x>>|<row|<cell|<verbatim|pi>>|<cell|Numerical constant
  <math|\<pi\>>>>|<row|<cell|<verbatim|random>>|<cell|A random value between
  0 and 1>>|<row|<cell|<verbatim|random x,y>>|<cell|A random value between
  <verbatim|x> and <verbatim|y>>>>>>|<label|math-operations>Mathematical
  operations>

  <paragraph|Text functions>Text functions operate on <verbatim|text> values.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|x&y>>>>>>|<cell|Concatenation>>|<row|<cell|<verbatim|length
  x>>|<cell|Length of the text>>|<row|<cell|<verbatim|x at
  y>>|<cell|Character at position <verbatim|y> in
  <verbatim|x>>>>>>|<label|text-operations>Text operations>

  <paragraph|Conversions>Conversions operations transform data from one type
  to another.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|real
  x:integer>>>>>>|<cell|Convert integer to real>>|<row|<cell|<verbatim|real
  x:text>>|<cell|Convert text to real>>|<row|<cell|<verbatim|integer
  x:real>>|<cell|Convert real to integer>>|<row|<cell|<verbatim|integer
  x:text>>|<cell|Convert text to real>>|<row|<cell|<verbatim|text
  x:integer>>|<cell|Convert integer to text>>|<row|<cell|<verbatim|text
  x:real>>|<cell|Convert real to text>>>>>|<label|conversions>Conversions>

  \;

  <paragraph|Date and time>Date and time functions manipulates time. Time is
  expressed with an integer representing a number of seconds since a time
  origin. Except for <verbatim|system_time> which never takes an argument,
  the functions can either take an explicit time represented as an
  <verbatim|integer> as returned by <verbatim|system_time>, or apply to the
  current time in the current time zone.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|hours>>>>>>|<cell|Hours>>|<row|<cell|<verbatim|minutes>>|<cell|Minutes>>|<row|<cell|<verbatim|seconds>>|<cell|Seconds>>|<row|<cell|<verbatim|year>>|<cell|Year>>|<row|<cell|<verbatim|month>>|<cell|Month>>|<row|<cell|<verbatim|day>>|<cell|Day
  of the month>>|<row|<cell|<verbatim|week_day>>|<cell|Day of the
  week>>|<row|<cell|<verbatim|year_day>>|<cell|Day of the
  year>>|<row|<cell|<verbatim|system_time>>|<cell|Current time in
  seconds>>>>>|<label|time-operations>Date and time>

  <paragraph|Tree operations>Tree operations are intended to manipulate
  trees.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|identity
  x>>>>>>|<cell|Returns <verbatim|x>>>|<row|<cell|<verbatim|do
  x>>|<cell|Forces evaluation of <verbatim|x>>>|<row|<cell|<verbatim|x with
  y>>|<cell|Map, Reduce or Filter>>|<row|<cell|<verbatim|x at y>>|<cell|Find
  element at index <verbatim|y> in <verbatim|x>>>|<row|<cell|<verbatim|x..y>>|<cell|Create
  a list of elements between <verbatim|x> and
  <verbatim|y>>>|<row|<cell|<verbatim|x,y>>|<cell|Used to create data
  lists>>|<row|<cell|<verbatim|()>>|<cell|The empty
  list>>>>>|<label|tree-operations>Tree operations>

  The map, reduce and filter operations act on data in an infix-separated
  list <verbatim|y>. The convention is to use comma-separated lists, such as
  <verbatim|1,3,5,6>, but other infix separators can be used.

  <\itemize>
    <item>Map: If <verbatim|x> is a name or a universal rewrite rule, a list
    is built by mapping the name to each element of the list in turn. For
    example, <verbatim|sin with (1,3)> returns the list <verbatim|sin 1, sin
    3>, and <verbatim|(x-\<gtr\>x+1) with (2,4)> returns the list
    <verbatim|3,5>.

    <item>Reduce: If <verbatim|x> is a rewrite rule with an infix on the
    left, the infix is considered as the separator for the list, and list
    elements in <verbatim|y> are reduced by applying the rewrite rule to
    successive elements. For example, <verbatim|(x,y-\<gtr\>x+y) with
    (1,2,3)> returns <verbatim|6>.

    <item>Filter: If <verbatim|x> is a guard, a new list is built by
    filtering elements of <verbatim|y> matching the guard condition. For
    example, <verbatim|(x when x \<less\> 0) with (1,2,-3,2,-1)> returns
    <verbatim|-3,-1>.
  </itemize>

  The notation <verbatim|x..y> is a shortcut for the comma-separated list of
  elements composed of elements between <verbatim|x> and <verbatim|y>
  inclusive. For example, <verbatim|1..3> is the list <verbatim|1,2,3>.

  <paragraph|Data loading operations>Data loading operations read a data file
  and convert it to an XLR parse tree suitable for XLR data manipulation
  functions. The arguments are:

  <\itemize>
    <item>The file <verbatim|f> is specified by its name as <verbatim|text>.

    <item>A prefix name <verbatim|p> can be specified as <verbatim|text>.

    <\itemize>
      <item>If the text is empty, the data is returned as <verbatim|NEWLINE>
      separated rows of comma-separated elements.

      <item>Otherwise, each row is prefixed with the specified prefix.
    </itemize>

    <item>The field separator <verbatim|fs> and row separator <verbatim|rs>
    are <verbatim|text> describing characters to be interepreted as
    separating fields and rows respectively.
  </itemize>

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|load
  f>>|<cell|Load a tree as an XLR program>>|<row|<cell|<verbatim|load_csv f,
  p>>|<cell|Load comma-separated data>>|<row|<cell|<verbatim|load_tsv f,
  p>>|<cell|Load tab-separated data>>|<row|<cell|<verbatim|load_data f, p,
  fs, rs>>|<cell|Load field-separated data>>>>>|<label|data-loading-operations>Data
  loading operations>

  \;

  <subsection|Lazy evaluation>

  Form parameters are evaluated lazily, meaning that the evaluation of one
  fragment of the form does not imply the evaluation of another. This makes
  it possible to evaluate infinite lists.

  <big-figure|<\verbatim>
    data x,y

    integers_starting_from N:integer -\<gtr\> N, integers_starting_from (N+1)
  </verbatim>|Lazy evaluation of an infinite list>

  <subsection|Controlled compilation>

  A special form, <verbatim|compile>, is used to tell the compiler how to
  compile its argument. This makes it possible to implement special
  optimization for often-used forms.

  <big-figure|<\verbatim>
    compile {if Condition then TrueForm else FalseForm} -\<gtr\>

    \ \ \ \ generate_if_then_else Condition, TrueForm, FalseForm \ \ \ \ 
  </verbatim>|Controlled compilation>

  Controlled compilation depends on low-level compilation primitives provided
  by the LLVM infrastructure<\footnote>
    For details, refer to <verbatim|http://llvm.org>.
  </footnote>, and assumes a good understanding of LLVM basic operations.
  Table<nbsp><reference|llvm-operations> shows the correspondance between
  LLVM primitives and primitives that can be used during controlled
  compilation.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|XLR
  Form>>|<cell|<strong|LLVM Entity>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|llvm_value
  x>>>>>>|<cell|<verbatim|Value *>>|<cell|The machine value associated to
  tree <verbatim|x>>>|<row|<cell|<verbatim|llvm_type x>>|<cell|<verbatim|Type
  *>>|<cell|The type associated to tree <verbatim|x>>>|<row|<cell|<verbatim|llvm_struct
  x>>|<cell|<verbatim|StructType *>>|<cell|The structure type for signature
  <verbatim|x>>>|<row|<cell|<verbatim|llvm_function_type
  x>>|<cell|<verbatim|FunctionType *>>|<cell|The function type for signature
  <verbatim|x>>>|<row|<cell|<verbatim|llvm_function
  x>>|<cell|<verbatim|Function *>>|<cell|The machine function associated to
  <verbatim|x>>>|<row|<cell|<verbatim|llvm_global
  x>>|<cell|<verbatim|GlobalValue *>>|<cell|The global value identifying tree
  <verbatim|x>>>|<row|<cell|<verbatim|llvm_bb n>>|<cell|<verbatim|BasicBlock
  *>>|<cell|A basic block with name <verbatim|n>>>|<row|<cell|<verbatim|llvm_type>>|<cell|>|<cell|>>>>>|<label|llvm-operations>LLVM
  operations>

  <subsection|Type inference>

  <section|Example code>

  <section|Implementation notes>

  This section describes the implementation as published at
  <verbatim|http://xlr.sourceforge.net>.

  <subsection|Tree representation>

  The tree representation is performed by the <verbatim|Tree> class, with one
  subclass per node type: <verbatim|Integer>, <verbatim|Real>,
  <verbatim|Text>, <verbatim|Name>, <verbatim|Infix>, <verbatim|Prefix>,
  <verbatim|Postfix> and <verbatim|Block>.

  The <verbatim|Tree> structure has template members <verbatim|GetInfo> and
  <verbatim|SetInfo> that make it possible to associate arbitrary data to a
  tree. Data is stored there using a class deriving from <verbatim|Info>.

  The rule of thumb is that <verbatim|Tree> only contains members for data
  that is used in the evaluation of any tree. Other data is stored using
  <verbatim|Info> entries.

  Currently, data that is directly associated to the <verbatim|Tree>
  includes:

  <\itemize>
    <item>The <verbatim|tag> field stores the kind of the tree as well as its
    position in the source code.

    <item>The <verbatim|info> field is a linked list of <verbatim|Info>
    entries.
  </itemize>

  <subsection|Evaluation of trees>

  Trees are evaluated in a given <em|context>, representing the evaluation
  environment. The context contains a lexical (static) and stack (dynamic)
  part.

  <\enumerate>
    <item>The <em|lexical context> represents the declarations that precede
    the tree being evaluated in the source code. It can be determined
    statically.

    <item>The <em|dynamic context> represents the declarations that were
    introduced as part of earlier evaluation, i.e. in the ``call stack''.
  </enumerate>

  A context is represented by a tree holding the declarations, along with
  associated code.

  <subsection|Tree position>

  The position held in the <verbatim|tag> field is character-precise. To save
  space, it counts the number of characters since the begining of compilation
  in a single integer value.

  The <verbatim|Positions> class defined in <verbatim|scanner.h> maps this
  count to the more practical file-line-column positioning. This process is
  relatively slow, but this is acceptable when emitting error messages.

  <subsection|Actions on trees>

  Recursive operations on tree are performed by the <verbatim|Action> class.
  This class implements virtual functions for each tree type called
  <verbatim|DoInteger>, <verbatim|DoReal>, <verbatim|DoText> and so on.

  <subsection|Symbols>

  The XL runtime environment maintains symbol tables which form a hierarchy.
  Each symbol table has a (possibly <verbatim|NULL>) parent, and contains two
  kinds of symbols: names and rewrites.

  <\itemize>
    <item>Names are associated directly with a tree value. For example,
    <verbatim|X-\<gtr\>0> will associate the value <verbatim|0> to name
    <verbatim|X>.

    <item>Rewrites are used for more complex tree rewrites, e.g.
    <verbatim|X+Y-\<gtr\>add X,Y>.
  </itemize>

  <subsection|Evaluating trees>

  A tree is evaluated as follows:

  <\enumerate>
    <item>Evaluation of a tree is performed by <verbatim|xl_evaluate()> in
    <verbatim|runtime.cpp>.

    <item>This function checks the stack depth to report infinite recursion.

    <item>If <verbatim|code> is <verbatim|NULL>, then the tree is compiled
    first.

    <item>Then, evaluation is performed by calling <verbatim|code> with the
    tree as argument.
  </enumerate>

  The signature for <verbatim|code> is a function taking a <verbatim|Tree>
  pointer and returning a <verbatim|Tree> pointer.

  <subsection|Code generation for trees>

  Evaluation functions are functions with the signature shown in
  Figure<nbsp><reference|rewrite-code>:

  <big-figure|<verbatim|Tree * (*eval_fn) (eval_fn eval, Tree
  *self)>|<label|rewrite-code>Signature for rewrite code with two variables.>

  Unfortunately, the signature in Figure<nbsp><reference|rewrite-code> is not
  valid in C or C++, so we need a lot of casting to achieve the desired
  effect.

  \;

  In general, the <verbatim|code> for a tree takes the tree as input, and
  returns the evaluated value.

  However, there are a few important exceptions to this rule:

  <paragraph|Right side of a rewrite>If the tree is on the right of a rewrite
  (i.e. the right of an infix <verbatim|-\<gtr\>> operator), then
  <verbatim|code> will take additional input trees as arguments.
  Specifically, there will be one additional parameter in the code per
  variable in the rewrite rule pattern.

  For example, if a rewrite is <verbatim|X+Y-\<gtr\>foo X,Y>, then the
  <verbatim|code> field for <verbatim|foo X,Y> will have <verbatim|X> as its
  second argument and <verbatim|Y> as its third argument, as shown in
  Figure<nbsp><reference|rewrite-code>.

  \;

  \;

  In that case, the input tree for the actual expression being rewritten
  remains passed as the first argument, generally denoted as <verbatim|self>.

  <paragraph|Closures>If a tree is passed as a <verbatim|tree> argument to a
  function, then it is encapsulated in a <em|closure>. The intent is to
  capture the environment that the passed tree depends on. Therefore, the
  associated <verbatim|code> will take additional arguments representing all
  the captured values. For instance, a closure for <verbatim|write X,Y> that
  captures variables <verbatim|X> and <verbatim|Y> will have the signature
  shown in Figure<nbsp><reference|closure-code>:

  <big-figure|<verbatim|Tree * (*code) (Tree *self, Tree *X, Tree
  *Y)>|<label|closure-code>Signature for rewrite code with two variables.>

  At runtime, the closure is represented by a prefix tree with the original
  tree on the left, and the captured values cascading on the right. For
  consistency, the captured values are always on the left of a
  <verbatim|Prefix> tree. The rightmost child of the rightmost
  <verbatim|Prefix> is set to an arbitrary, unused value (specifically,
  <verbatim|false>).

  Closures are built by the function <verbatim|xl_new_closure>, which is
  generally invoked from generated code. Their <verbatim|code> field is set
  to a function that reads all the arguments from the tree and invokes the
  code with the additional arguments.

  For example, <verbatim|do> takes a <verbatim|tree> argument. When
  evaluating <verbatim|do write X,Y>, the tree given as an argument to
  <verbatim|do> depends on variable <verbatim|X> and <verbatim|Y>, which may
  not be visible in the body of <verbatim|do>. These variables are therefore
  captured in the closure. If its values of <verbatim|X> and <verbatim|Y> are
  <verbatim|42> and <verbatim|Universe>, then <verbatim|do> receives a
  closure for <verbatim|write X,Y> with arguments <verbatim|42> and
  <verbatim|Universe>.

  \;

  <subsection|Tail recursion>

  <subsection|Partial recompilation>

  \;

  \;
</body>

<\initial>
  <\collection>
    <associate|par-hyphen|normal>
  </collection>
</initial>

<\references>
  <\collection>
    <associate|arithmetic|<tuple|2|14>>
    <associate|array-assign|<tuple|19|?>>
    <associate|auto-1|<tuple|1|1>>
    <associate|auto-10|<tuple|2.2|2>>
    <associate|auto-100|<tuple|38|?>>
    <associate|auto-101|<tuple|39|?>>
    <associate|auto-102|<tuple|26|?>>
    <associate|auto-103|<tuple|5.8|?>>
    <associate|auto-104|<tuple|5.9|?>>
    <associate|auto-105|<tuple|5.8|?>>
    <associate|auto-106|<tuple|5.9|?>>
    <associate|auto-11|<tuple|5|2>>
    <associate|auto-12|<tuple|2.3|3>>
    <associate|auto-13|<tuple|3|3>>
    <associate|auto-14|<tuple|6|3>>
    <associate|auto-15|<tuple|4|3>>
    <associate|auto-16|<tuple|7|4>>
    <associate|auto-17|<tuple|5|4>>
    <associate|auto-18|<tuple|8|4>>
    <associate|auto-19|<tuple|9|4>>
    <associate|auto-2|<tuple|1|1>>
    <associate|auto-20|<tuple|6|4>>
    <associate|auto-21|<tuple|10|4>>
    <associate|auto-22|<tuple|2.4|5>>
    <associate|auto-23|<tuple|7|5>>
    <associate|auto-24|<tuple|8|5>>
    <associate|auto-25|<tuple|9|5>>
    <associate|auto-26|<tuple|2.5|5>>
    <associate|auto-27|<tuple|10|6>>
    <associate|auto-28|<tuple|11|6>>
    <associate|auto-29|<tuple|12|6>>
    <associate|auto-3|<tuple|2|1>>
    <associate|auto-30|<tuple|13|7>>
    <associate|auto-31|<tuple|2.6|8>>
    <associate|auto-32|<tuple|11|8>>
    <associate|auto-33|<tuple|12|8>>
    <associate|auto-34|<tuple|3|9>>
    <associate|auto-35|<tuple|3.1|9>>
    <associate|auto-36|<tuple|14|9>>
    <associate|auto-37|<tuple|13|9>>
    <associate|auto-38|<tuple|14|9>>
    <associate|auto-39|<tuple|15|10>>
    <associate|auto-4|<tuple|1|1>>
    <associate|auto-40|<tuple|15|10>>
    <associate|auto-41|<tuple|16|10>>
    <associate|auto-42|<tuple|16|10>>
    <associate|auto-43|<tuple|17|10>>
    <associate|auto-44|<tuple|17|10>>
    <associate|auto-45|<tuple|18|10>>
    <associate|auto-46|<tuple|18|10>>
    <associate|auto-47|<tuple|19|11>>
    <associate|auto-48|<tuple|19|12>>
    <associate|auto-49|<tuple|20|12>>
    <associate|auto-5|<tuple|2|1>>
    <associate|auto-50|<tuple|3.2|12>>
    <associate|auto-51|<tuple|20|12>>
    <associate|auto-52|<tuple|3.3|12>>
    <associate|auto-53|<tuple|3.4|13>>
    <associate|auto-54|<tuple|21|13>>
    <associate|auto-55|<tuple|22|13>>
    <associate|auto-56|<tuple|23|13>>
    <associate|auto-57|<tuple|24|13>>
    <associate|auto-58|<tuple|25|13>>
    <associate|auto-59|<tuple|26|13>>
    <associate|auto-6|<tuple|3|2>>
    <associate|auto-60|<tuple|21|13>>
    <associate|auto-61|<tuple|27|13>>
    <associate|auto-62|<tuple|22|13>>
    <associate|auto-63|<tuple|3.5|13>>
    <associate|auto-64|<tuple|28|14>>
    <associate|auto-65|<tuple|1|14>>
    <associate|auto-66|<tuple|29|14>>
    <associate|auto-67|<tuple|2|14>>
    <associate|auto-68|<tuple|30|14>>
    <associate|auto-69|<tuple|3|14>>
    <associate|auto-7|<tuple|2|2>>
    <associate|auto-70|<tuple|31|14>>
    <associate|auto-71|<tuple|4|15>>
    <associate|auto-72|<tuple|32|15>>
    <associate|auto-73|<tuple|5|15>>
    <associate|auto-74|<tuple|33|15>>
    <associate|auto-75|<tuple|6|15>>
    <associate|auto-76|<tuple|34|15>>
    <associate|auto-77|<tuple|7|15>>
    <associate|auto-78|<tuple|35|15>>
    <associate|auto-79|<tuple|8|16>>
    <associate|auto-8|<tuple|2.1|2>>
    <associate|auto-80|<tuple|36|16>>
    <associate|auto-81|<tuple|9|16>>
    <associate|auto-82|<tuple|37|16>>
    <associate|auto-83|<tuple|10|16>>
    <associate|auto-84|<tuple|3.6|16>>
    <associate|auto-85|<tuple|23|17>>
    <associate|auto-86|<tuple|3.7|17>>
    <associate|auto-87|<tuple|24|17>>
    <associate|auto-88|<tuple|11|17>>
    <associate|auto-89|<tuple|3.8|17>>
    <associate|auto-9|<tuple|4|2>>
    <associate|auto-90|<tuple|4|17>>
    <associate|auto-91|<tuple|5|?>>
    <associate|auto-92|<tuple|5.1|?>>
    <associate|auto-93|<tuple|5.2|?>>
    <associate|auto-94|<tuple|5.3|?>>
    <associate|auto-95|<tuple|5.4|?>>
    <associate|auto-96|<tuple|5.5|?>>
    <associate|auto-97|<tuple|5.6|?>>
    <associate|auto-98|<tuple|5.7|?>>
    <associate|auto-99|<tuple|25|?>>
    <associate|binding|<tuple|3.3|12>>
    <associate|bitwise-arithmetic|<tuple|3|14>>
    <associate|boolean-operations|<tuple|4|14>>
    <associate|built-ins|<tuple|3.5|13>>
    <associate|closure-code|<tuple|26|?>>
    <associate|comma-separated-list|<tuple|15|9>>
    <associate|comments|<tuple|5|2>>
    <associate|complex-type|<tuple|16|10>>
    <associate|conversions|<tuple|7|15>>
    <associate|data-loading-operations|<tuple|10|16>>
    <associate|evaluation|<tuple|3.2|11>>
    <associate|factorial|<tuple|1|1>>
    <associate|footnote-1|<tuple|1|2>>
    <associate|footnote-2|<tuple|2|2>>
    <associate|footnote-3|<tuple|3|4>>
    <associate|footnote-4|<tuple|4|6>>
    <associate|footnote-5|<tuple|5|12>>
    <associate|footnote-6|<tuple|6|13>>
    <associate|footnote-7|<tuple|7|16>>
    <associate|footnr-1|<tuple|1|2>>
    <associate|footnr-2|<tuple|2|2>>
    <associate|footnr-3|<tuple|3|4>>
    <associate|footnr-4|<tuple|4|6>>
    <associate|footnr-5|<tuple|5|12>>
    <associate|footnr-6|<tuple|6|13>>
    <associate|footnr-7|<tuple|7|16>>
    <associate|guard|<tuple|18|10>>
    <associate|if-then|<tuple|3|1>>
    <associate|if-then-else|<tuple|13|9>>
    <associate|if-then-else-colorized|<tuple|14|9>>
    <associate|iterations|<tuple|3|?>>
    <associate|literals|<tuple|2.3|2>>
    <associate|llvm-operations|<tuple|11|17>>
    <associate|long-text-indent|<tuple|9|4>>
    <associate|map-reduce-filter|<tuple|2|1>>
    <associate|math-operations|<tuple|5|14>>
    <associate|mathematical-functions|<tuple|5|?>>
    <associate|off-side-rule|<tuple|4|2>>
    <associate|precedence|<tuple|2.6|6>>
    <associate|rewrite-code|<tuple|25|?>>
    <associate|rewrite-type|<tuple|22|13>>
    <associate|sequence|<tuple|20|10>>
    <associate|simpleprog|<tuple|8|3>>
    <associate|source-syntax|<tuple|12|8>>
    <associate|syntax-file|<tuple|11|7>>
    <associate|text-operations|<tuple|6|15>>
    <associate|time-operations|<tuple|8|15>>
    <associate|tree-operations|<tuple|9|15>>
    <associate|type-declaration|<tuple|17|10>>
    <associate|type-name|<tuple|21|13>>
    <associate|types|<tuple|3.4|12>>
    <associate|xlsyntax|<tuple|2.1|?>>
  </collection>
</references>

<\auxiliary>
  <\collection>
    <\associate|figure>
      <tuple|normal|<label|if-then>Declaration of
      if-then-else|<pageref|auto-2>>

      <tuple|normal|<label|factorial>Declaration of the factorial
      function|<pageref|auto-3>>

      <tuple|normal|<label|map-reduce-filter>Map, reduce and
      filter|<pageref|auto-4>>

      <tuple|normal|<label|off-side-rule>Off-side rule: Using indentation to
      mark program structure.|<pageref|auto-7>>

      <tuple|normal|<label|comments>Single-line and multi-line
      comments|<pageref|auto-9>>

      <tuple|normal|Valid integer constants|<pageref|auto-12>>

      <tuple|normal|Valid real constants|<pageref|auto-14>>

      <tuple|normal|Valid text constants|<pageref|auto-16>>

      <tuple|normal|<label|long-text-indent>Long text and
      indentation|<pageref|auto-17>>

      <tuple|normal|Examples of valid operator and name
      symbols|<pageref|auto-19>>

      <tuple|normal|<label|syntax-file>Default syntax configuration
      file|<pageref|auto-30>>

      <tuple|normal|<label|source-syntax>Use of the
      <with|font-family|<quote|tt>|language|<quote|verbatim>|syntax>
      specification in a source file|<pageref|auto-31>>

      <tuple|normal|<label|if-then-else>Examples of tree
      rewrites|<pageref|auto-35>>

      <tuple|normal|<label|if-then-else-colorized>Examples of tree
      rewrites|<pageref|auto-36>>

      <tuple|normal|<label|comma-separated-list>Declaring a comma-separated
      list|<pageref|auto-38>>

      <tuple|normal|<label|complex-type>Declaring a
      <with|font-family|<quote|tt>|language|<quote|verbatim>|complex> data
      type|<pageref|auto-39>>

      <tuple|normal|<label|type-declaration>Simple type
      declarations|<pageref|auto-41>>

      <tuple|normal|<label|guard>Guard limit the validity of
      operations|<pageref|auto-43>>

      <tuple|normal|<label|array-assign>Assignment to an
      array|<pageref|auto-45>>

      <tuple|normal|<label|sequence>Code writing
      <with|font-family|<quote|tt>|language|<quote|verbatim>|A>, then
      <with|font-family|<quote|tt>|language|<quote|verbatim>|B>, then
      <with|font-family|<quote|tt>|language|<quote|verbatim>|f(100)+f(200)>|<pageref|auto-47>>

      <tuple|normal|<label|type-name>Binding a type expression to a
      name|<pageref|auto-58>>

      <tuple|normal|<label|rewrite-type>Declaration of a rewrite
      type|<pageref|auto-60>>

      <tuple|normal|Lazy evaluation of an infinite list|<pageref|auto-83>>

      <tuple|normal|Controlled compilation|<pageref|auto-85>>

      <tuple|normal|<label|rewrite-code>Signature for rewrite code with two
      variables.|<pageref|auto-97>>

      <tuple|normal|<label|closure-code>Signature for rewrite code with two
      variables.|<pageref|auto-100>>
    </associate>
    <\associate|table>
      <tuple|normal|<label|arithmetic>Arithmetic
      operations|<pageref|auto-63>>

      <tuple|normal|<label|arithmetic>Comparisons|<pageref|auto-65>>

      <tuple|normal|<label|bitwise-arithmetic>Bitwise arithmetic
      operations|<pageref|auto-67>>

      <tuple|normal|<label|boolean-operations>Boolean
      operations|<pageref|auto-69>>

      <tuple|normal|<label|math-operations>Mathematical
      operations|<pageref|auto-71>>

      <tuple|normal|<label|text-operations>Text operations|<pageref|auto-73>>

      <tuple|normal|<label|conversions>Conversions|<pageref|auto-75>>

      <tuple|normal|<label|time-operations>Date and time|<pageref|auto-77>>

      <tuple|normal|<label|tree-operations>Tree operations|<pageref|auto-79>>

      <tuple|normal|<label|data-loading-operations>Data loading
      operations|<pageref|auto-81>>

      <tuple|normal|<label|llvm-operations>LLVM operations|<pageref|auto-86>>
    </associate>
    <\associate|toc>
      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|1<space|2spc>Introduction>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-1><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|2<space|2spc>Syntax>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-5><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|2.1<space|2spc>Spaces and indentation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-6>>

      <with|par-left|<quote|1.5fn>|2.2<space|2spc>Comments and spaces
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-8>>

      <with|par-left|<quote|1.5fn>|2.3<space|2spc><label|literals>Literals
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-10>>

      <with|par-left|<quote|6fn>|Integer constants
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-11><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Real constants
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-13><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Text literals
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-15><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Name and operator symbols
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-18><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|2.4<space|2spc>Structured nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-20>>

      <with|par-left|<quote|6fn>|Infix nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-21><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Prefix and postfix nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-22><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Block nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-23><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|2.5<space|2spc>Parsing rules
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-24>>

      <with|par-left|<quote|6fn>|Precedence
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-25><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Associativity
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-26><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Infix versus Prefix versus Postfix
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-27><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Expression versus statement
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-28><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|2.6<space|2spc><label|precedence>Syntax
      configuration <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-29>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|3<space|2spc>Semantics>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-32><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|3.1<space|2spc>Operators
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-33>>

      <with|par-left|<quote|6fn>|Rewrite declarations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-34><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Data declaration
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-37><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Type declaration
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-40><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Guards <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-42><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Assignment
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-44><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Sequences
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-46><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|3.2<space|2spc><label|evaluation>Evaluation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-48>>

      <with|par-left|<quote|6fn>|Special forms
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-49><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|3.3<space|2spc><label|binding>Variable
      binding and scoping <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-50>>

      <with|par-left|<quote|1.5fn>|3.4<space|2spc><label|types>Types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-51>>

      <with|par-left|<quote|6fn>|Predefined types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-52><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Value types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-53><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Block type
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-54><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Union type
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-55><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Structure types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-56><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Named types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-57><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Rewrite types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-59><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|3.5<space|2spc><label|built-ins>Built-in
      operations <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-61>>

      <with|par-left|<quote|6fn>|Arithmetic
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-62><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Comparison
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-64><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Bitwise arithmetic
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-66><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Boolean operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-68><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Mathematical functions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-70><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Text functions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-72><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Conversions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-74><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Date and time
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-76><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Tree operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-78><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Data loading operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-80><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|3.6<space|2spc>Lazy evaluation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-82>>

      <with|par-left|<quote|1.5fn>|3.7<space|2spc>Controlled compilation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-84>>

      <with|par-left|<quote|1.5fn>|3.8<space|2spc>Type inference
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-87>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|4<space|2spc>Example
      code> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-88><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|5<space|2spc>Implementation
      notes> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-89><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|5.1<space|2spc>Tree representation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-90>>

      <with|par-left|<quote|1.5fn>|5.2<space|2spc>Evaluation of trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-91>>

      <with|par-left|<quote|1.5fn>|5.3<space|2spc>Tree position
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-92>>

      <with|par-left|<quote|1.5fn>|5.4<space|2spc>Actions on trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-93>>

      <with|par-left|<quote|1.5fn>|5.5<space|2spc>Symbols
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-94>>

      <with|par-left|<quote|1.5fn>|5.6<space|2spc>Evaluating trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-95>>

      <with|par-left|<quote|1.5fn>|5.7<space|2spc>Code generation for trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-96>>

      <with|par-left|<quote|6fn>|Right side of a rewrite
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-98><vspace|0.15fn>>

      <with|par-left|<quote|6fn>|Closures
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-99><vspace|0.15fn>>

      <with|par-left|<quote|1.5fn>|5.8<space|2spc>Tail recursion
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-101>>

      <with|par-left|<quote|1.5fn>|5.9<space|2spc>Partial recompilation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-102>>
    </associate>
  </collection>
</auxiliary>