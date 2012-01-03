<TeXmacs|1.0.7.14>

<style|article>

<\body>
  <doc-data|<doc-title|The XLR Programming
  Language>|<\doc-author-data|<author-name|Christophe de
  Dinechin>|<\author-address>
    Taodyne SAS, 1300 Route des Cretes, Sophia Antipolis, France
  </author-address>>
    \;
  </doc-author-data>>

  <section|Introduction>

  XLR is a dynamic language based on meta-programming, i.e. programs that
  manipulate programs. It is designed to enable a natural syntax similar to
  imperative languages descending from Algol (e.g. C, Pascal, Ada, Modula),
  while preserving the power and expressiveness of homoiconic languages
  descending from Lisp (i.e. languages where programs are data).<strong|>

  <subsection|Design objectives>

  XLR is intended to make it easy to write simple<strong|> programs, yet
  impose no upper limit, since you can extend the language to suit your own
  needs. The goal is to make it as easy and robust to extend the language
  with new features or concepts as it is to add new functions or classes in
  more traditional programming languages.

  This design is in response to the following observation: programmers have
  to deal with exponentially-growing program complexity. The reason is that
  the complexity of programs indirectly follows Moore's law, since users want
  to fully benefit from the capabilities of new hardware generations. But our
  brains do not follow a similar exponential law, so we need increasingly
  sophisticated tools to bridge the gap with higher and higher levels of
  abstraction.

  Over time, this lead to a never ending succession of programming paradigms,
  each one intended to make the next generation of hardware accessible to
  programmers. For example, object-oriented programming was primarily fueled
  by the demands of graphical user interfaces.

  The unfortunate side effect of this continuous change in programming
  paradigms is that code designed with an old approach quickly becomes
  obsolete as a new programming model emerges. For example, even if C++ is
  nominally compatible with C, the core development model is so incompatible
  that C++ replicates core functionality of C in a completely different way
  (memory allocation, I/Os, containers, sorts, etc).

  The purpose of XLR is to allow the language to grow naturally over time,
  under programmers' control. XLR actually stands for ``Extensible Language
  and Runtime''. The long term vision is a language made both more powerful
  and easier to use thanks to a large number of community-developed and
  field-tested language extensions.

  <subsection|Keeping the syntax simple.>

  In order to keep programs easy to write and read, the XLR syntax is very
  simple. It will seem very natural to most programmers, except for what it's
  missing: XLR makes little use of parentheses and other punctuation
  characters. Instead, the syntax is based on indentation. There was a
  conscious design decision to keep only symbols that had an active role in
  the meaning of the program, as opposed to a purely syntactic role. XLR
  programs look a little like pseudo-code, except that they can be compiled
  and run.

  This simplicity translates into the internal representations of programs,
  which makes meta-programming not just possible, but easy. Any XLR
  program<strong|> or data can be represented with just 8 data types:
  integer, real, text, name, infix, prefix, posfix and block. For example,
  the internal representation for <verbatim|3 * sin X> is an infix
  <verbatim|*> node with two children, the left child of <verbatim|*> being
  integer <verbatim|3>, and the right child of <verbatim|*> being a prefix
  with the name <verbatim|sin> on the left and the name <verbatim|X> on the
  right. These basic types form an <em|abstract syntax tree> (AST).\ 

  The data structure representing programs is simple enough to make
  meta-programming practical and easy. Meta-programming is the ability for a
  program to deal with programs. In the case of XLR, meta-programming is the
  key to language extensibility. Instead of wishing you had this or that
  feature in the language, you can simply add it. Better yet, the process of
  extending the language is so simple that you can now consider language
  notations or compilation techniques that are useful only in a particular
  context. In short, with XLR, creating your own <em|domain-specific
  languages> (DSLs) is just part of normal, everyday programming.

  ASTs are also central to understanding how XLR programs are executed.
  Conceptually, an XLR program is a transformation of ASTs following a number
  of tree rewrite rules. In other words, there is no real difference in XLR
  between meta-programming and normal program execution, as both are
  represented by tranformations on ASTs.

  <subsection|Examples>

  The key characteristics of XLR outlined above are best illustrated with a
  few short examples, going from simple programming to more advanced
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
  abstract syntax tree format known as <em|XL0>. XL0 trees consist of four
  literal node types (integer, real, text and symbol) and four structured
  node types (prefix, postfix, infix and block). Note that line breaks
  normally parse as infix operators, and that indentation normally parses as
  blocks.

  The precedence of operators is given by the <verbatim|xl.syntax>
  configuration file. It can also be changed dynamically in the source code
  using the <verbatim|syntax> statements. This is detailed in
  Section<nbsp><reference|precedence>. Both methods to define syntax are
  called <em|syntax configuration>.

  The rest of this document will occasionally refer to <em|normal XLR> for
  defaults settings such as the default syntax configuration, as shipped with
  the standard XLR distribution.

  <subsection|Spaces and indentation>

  Spaces and tabs are generally not significant, but may be required to
  separate operator or name symbols. For example, there is no difference
  between <verbatim|A B> (one space) and <verbatim|A \ \ \ B> (four spaces),
  but both are different from <verbatim|AB> (zero space).

  Spaces and tabs are significant at the beginning of lines. XLR will use
  them to determine the level of indentation from which it derives program
  structures (off-side rule), as illustrated in
  Figure<nbsp><reference|off-side-rule>. Both space or tabs can be used for
  indentation, but cannot be mixed for indentation in a single source file.
  In other words, if the first indented line uses spaces, all other
  indentation must be done using spaces, and similarly for tabs.

  <big-figure|<\verbatim>
    if A \<less\> 3 then

    \ \ \ \ write "A is too small"

    else

    \ \ \ \ write "A is too big"
  </verbatim>|<label|off-side-rule>Off-side rule: Using indentation to mark
  program structure.>

  <subsection|Comments and spaces>

  Comments are section of the source text which are typically used for
  documentation purpose and play no role in the execution of the program.
  Comments begin with a comment separator, and finish with a comment
  terminator.

  Comments in normal XLR are similar to C++ comments: they begin with
  <verbatim|/*> and finish with <verbatim|*/>, or they begin with
  <verbatim|//> and finish at the end of line. This is illustrated in
  Figure<nbsp><reference|comments>.

  <big-figure|<\verbatim>
    // This is a single-line comment

    /* This particular comment

    \ \ \ can be placed on multiple lines */
  </verbatim>|<label|comments>Single-line and multi-line comments>

  While comments play no actual role in the execution of a normal XLR
  program, they are actually recorded as attachments in XL0. It is possible
  for some special code to access or otherwise use these comments. For
  example, a documentation generator can read comments and use them to
  construct documentation automatically.

  <subsection|<label|literals>Literals>

  Four literal node types represent atomic values, i.e. values which cannot
  be decomposed into smaller units from an XLR point of view. They are:

  <\enumerate-numeric>
    <item>Integer constants

    <item>Real constants

    <item>Text literals

    <item>Symbols and names
  </enumerate-numeric>

  <subsubsection|Integer constants>

  Integer constants<\footnote>
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

  <subsubsection|Real constants>

  Real constants such as <verbatim|3.14> consist of one or more digits
  (<verbatim|0123456789>), followed by a single dot <verbatim|.> followed by
  one or more digits (<verbatim|0123456789>). Note that there must be at
  least one digit after the dot, i.e. <verbatim|1.> is not a valid real
  constant, but <verbatim|1.0> is.

  Real constants can have a radix and use underscores to separate digits like
  integer constants. For example <verbatim|2#1.1> is the same as
  <verbatim|1.5> and <verbatim|3.141_592_653> is an approximation of
  <math|\<pi\>>.

  A real constant can have an exponent, which consists of an optional hash
  sign <verbatim|#>, followed by the character <verbatim|e> or <verbatim|E>,
  followed by optional plus <verbatim|+> or minus <verbatim|-> sign, followed
  by one or more decimal digits <verbatim|0123456789>. For example,
  <verbatim|1.0e-3> is the same as <verbatim|0.001> and <verbatim|1.0E3> is
  the same as <verbatim|1000.0>. The exponent value is always given in
  radix-10, and indicates a power of the given radix. For example,
  <verbatim|2#1.0e3> represents <math|2<rsup|3>>, in other words it is the
  same as <verbatim|8.0>.

  The hash sign in the exponent is required for any radix greater than 14,
  since in that case the character <verbatim|e> or <verbatim|E> is also a
  valid digit. For instance, <verbatim|16#1.0E1> is approximately the same as
  <verbatim|1.05493>, whereas <verbatim|16#1.0#E1> is the same as
  <verbatim|16.0>.

  <\big-figure>
    <\verbatim>
      1.0

      3.1415_9265_3589_7932

      2#1.0000_0001#e-128
    </verbatim>
  </big-figure|Valid real constants>

  <subsubsection|Text literals>

  Text is any valid UTF-8 sequence of printable or space characters
  surrounded by text delimiters, such as <verbatim|"Hello<nbsp>Möndé">.
  Except for line-terminating characters, the behavior when a text sequence
  contains control characters or invalid UTF-8 sequences is unspecified.
  However, implementations are encouraged to preserve the contents of such
  sequences.

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

  \;

  <big-figure|<\verbatim>
    "Hello World"

    'Où Toto élabora ce plan çi'

    \<less\>\<less\>This text spans

    multiple lines\<gtr\>\<gtr\>
  </verbatim>|Valid text constants>

  When long text contains multiple lines of text, indentation is ignored up
  to the indentation level of the first character in the long text.
  Figure<nbsp><reference|long-text-indent> illustrates how long text indent
  is eliminated from the text being read.

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

  The text delimiters are not part of the value of text literals. Therefore,
  text delimiters are ignored when comparing texts.

  <subsubsection|Name and operator symbols>

  Names begin with an alphabetic character <verbatim|A>..<verbatim|Z> or
  <verbatim|a>..<verbatim|z> or any non-ASCII UTF-8 character, followed by
  the longuest possible sequence of alphabetic characters, digits or
  underscores. Two consecutive underscore characters are not allowed. Thus,
  <verbatim|Marylin_Monroe>, <verbatim|élaböràtion> or <verbatim|j1> are
  valid XLR names, whereas <verbatim|A-1>, <verbatim|1cm> or <verbatim|A__2>
  are not.

  Operator symbols, or <em|operators>, begin with an ASCII punctuation
  character<\footnote>
    Non-ASCII punctuation characters or digits are considered as alphabetic.
  </footnote> which does not act as a special delimiter for text, comments or
  blocks. For example, <verbatim|+> or <verbatim|-\<gtr\>> are operator
  symbols. An operator includes more than one punctuation character only if
  it has been declared in the syntax (typically in the syntax configuration
  file). For example, unless the symbol <verbatim|%,> (percent character
  followed by comma character) has been declared in the syntax,
  <verbatim|3%,4%> will contain two operator symbols <verbatim|%> and
  <strong|,> instead of a single <verbatim|%,> operator.

  A special name, the empty name, exists only as a child of empty blocks such
  as <verbatim|()>.

  After parsing, operator and name symbols are treated identically. During
  parsing, they are treated identically except in the expression versus
  statement rule explained below.

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
    <verbatim|A and B>, where the operator is between its two operands.

    <item>Prefix nodes, representing operations such as <verbatim|+3> or
    <verbatim|sin x>, where the operator is before its operand.

    <item>Postfix nodes, representing operations such as <verbatim|3%> or
    <verbatim|3 cm>, where the operator is after its operand.

    <item>Blocks, representing grouping such as <verbatim|(A+B)> or
    <verbatim|{lathe;rinse;repeat}>, where the operators surround their
    operand.
  </enumerate>

  <subsubsection|Infix nodes>

  An infix node has two children, one on the left, one on the right,
  separated by a name or operator symbol.

  Infix nodes are used to separate statements with semi-colons <verbatim|;>
  or line breaks (referred to as <verbatim|NEWLINE> in syntax configuration).

  <subsubsection|Prefix and postfix nodes>

  Prefix and postfix nodes have two children, one on the left, one on the
  right, without any separator between them. The only difference is in what
  is considered the ``operation'' and what is considered the ``operand''. For
  a prefix node, the operation is on the left and the operand on the right,
  whereas for a postfix node, the operation is on the right and the operand
  on the left.

  Prefix nodes are used for functions. The default for a name or operator
  symbol that is not explicitly declared in the <verbatim|xl.syntax> file or
  configured is to be treated as a prefix function, i.e. to be given a common
  function precedence referred to as <verbatim|FUNCTION> in syntax
  configuration. For example, <verbatim|sin> in the expression <verbatim|sin
  x> is treated as a function.

  <subsubsection|Block nodes>

  Block nodes have one child bracketed by two delimiters.

  Normal XLR recognizes the following pairs as block delimiters:

  <\itemize>
    <item>Parentheses, as in <verbatim|(A)>

    <item>Brackets, as in <verbatim|[A]>

    <item>Curly braces, as in <verbatim|{A}>

    <item>Indentation, as shown surrounding the <verbatim|write> statements
    in Figure<nbsp><reference|off-side-rule>. The delimiters for indentation
    are referred to as <verbatim|INDENT> and <verbatim|UNINDENT> in syntax
    configuration.
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

  <subsubsection|Precedence>

  Infix, prefix, postfix and block symbols are ranked according to their
  <em|precedence>, represented as a non-negative integer. The precedence is
  specified by the syntax configuration, either in the syntax configuration
  file, <verbatim|xl.syntax>, or through <verbatim|syntax> statmeents in the
  source code. This is detailed in Section<nbsp><reference|precedence>.

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
  resulting expression. This precedence given to entire expressions is used
  primarily in the <em|expression versus statement> rule described below.

  <subsubsection|Associativity>

  Infix operators can associate to their left or to their right.

  The addition operator is traditionally left-associative, meaning that in
  <verbatim|A+B+C>, <verbatim|A> and <verbatim|B> associate before
  <verbatim|C>. As a result, the outer infix <verbatim|+> node in
  <verbatim|A+B+C> has an infix <verbatim|+> node as its left child, with
  <verbatim|A> and <verbatim|B> as children, and <verbatim|C> as its right
  child.

  Conversely, the semi-colon in XLR is right-associative, meaning that
  <verbatim|A;B;C> is an infix node with an infix as the right child and
  <verbatim|A> as the left child.

  Operators with left and right associativity cannot have the same
  precedence, as this would lead to ambiguity. To enforce that rule, XLR
  arbitrarily gives an even precedence to left-associative operators, and an
  odd precedence to right-associative operators. For example, the precedence
  of <verbatim|+> in the default configuration is <verbatim|290>
  (left-associative), whereas the precedence of <verbatim|^> is
  <verbatim|395> (right-associative).

  <subsubsection|Infix versus Prefix versus Postfix>

  During parsing, XLR needs to resolve ambiguities between infix and prefix
  symbols. For example, in <verbatim|-A + B>, the minus sign <verbatim|-> is
  a prefix, whereas the plus sign <verbatim|+> is an infix. Similarly, in
  <verbatim|A and not B>, the <verbatim|and> word is infix, whereas the
  <verbatim|not> word is prefix. The problem is therefore exactly similar for
  names and operator symbols.

  XLR resolves this ambiguity as follows<\footnote>
    All the examples given are in normal XL, i.e. based on the default
    <verbatim|xl.syntax> configuration file.
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
  <em|function precedence>, identified by <verbatim|FUNCTION> in the syntax
  configuration.

  <subsubsection|Expression versus statement>

  Another ambiguity is related to the way humans read text. In
  <verbatim|write sin x, sin y>, most humans will read this as a
  <verbatim|write> instruction taking two arguments. This is however not
  entirely logical: if <verbatim|write> takes two arguments, then why
  shouldn't <verbatim|sin> also take two arguments? In other words, why
  should this example parse as <verbatim|write(sin(x),sin(y))> and not as
  <verbatim|write(sin(x,sin(y)))>?

  The reason is that we tend to make a distinction between ``statements'' and
  ``expressions''. This is not a distinction that is very relevant to
  computers, but one that exists in most natural languages, which distinguish
  whole sentences as opposed to subject or complement.

  XLR resolves the ambiguity by implementing a similar distinction. The
  boundary is a particular infix precedence, called <em|statement
  precedence>, denoted as <verbatim|STATEMENT> in the syntax configuration.
  Intuitively, infix operators with a lower precedence separate statements,
  whereas infix operators with a higher precedence separate expressions. For
  example, the semi-colon <verbatim|;> or <verbatim|else> separate
  statements, whereas <verbatim|+> or <verbatim|and> separate instructions.

  More precisely:

  <\itemize>
    <item>If a block's precedence is less than statement precedence, its
    content begins as an expression, otherwise it begins as a statement:
    <verbatim|3> in <verbatim|(3)> is an expression, <verbatim|write> in
    <verbatim|{write}> is a statement.

    <item>Right after an infix symbol with a precedence lower than statement
    precedence, we are in a statement, otherwise we are in an expression. The
    name <verbatim|B> in <verbatim|A+B> is an expression, but it is a
    statement in <verbatim|A;B>.

    <item>A similar rule applies after prefix nodes: <verbatim|{optimize}
    write A,B> gives two arguments to <verbatim|write>, whereas in
    <verbatim|(x-\<gtr\>x+1) sin x,y> the <verbatim|sin> function only
    receives a single argument.

    <item>A default prefix begins a statement if it's a name, an expression
    if it's a symbol: the name <verbatim|write> in <verbatim|write X> begins
    a statement, the symbol <verbatim|+> in <verbatim|+3> begins an
    expression.
  </itemize>

  In practice, there is no need to worry too much about these rules, since
  normal XLR ensures that most text parses as one would expect from daily use
  of English or mathematical notations.

  <subsection|<label|precedence>Syntax configuration>

  The default XLR syntax configuration file, named <verbatim|xl.syntax>,
  looks like Figure<nbsp><reference|syntax-file> and specifies the standard
  operators and their precedence.

  <\big-figure>
    <\verbatim>
      INFIX

      \ \ \ \ \ \ \ \ 11 \ \ \ \ \ NEWLINE

      \ \ \ \ \ \ \ \ 21 \ \ \ \ \ -\<gtr\> =\<gtr\>

      \ \ \ \ \ \ \ \ 31 \ \ \ \ \ else into

      \ \ \ \ \ \ \ \ 40 \ \ \ \ \ loop while until

      \ \ \ \ \ \ \ \ 50 \ \ \ \ \ then require ensure

      \ \ \ \ \ \ \ \ 61 \ \ \ \ \ ;

      \ \ \ \ \ \ \ \ 75 \ \ \ \ \ with

      \ \ \ \ \ \ \ \ 85 \ \ \ \ \ := += -= *= /= ^= \|= &=

      \ \ \ \ \ \ \ \ 100 \ \ \ \ STATEMENT

      \ \ \ \ \ \ \ \ 110 \ \ \ \ is as\ 

      \ \ \ \ \ \ \ \ 120 \ \ \ \ written

      \ \ \ \ \ \ \ \ 130 \ \ \ \ where

      \ \ \ \ \ \ \ \ 200 \ \ \ \ DEFAULT

      \ \ \ \ \ \ \ \ 211 \ \ \ \ when

      \ \ \ \ \ \ \ \ 231 \ \ \ \ ,

      \ \ \ \ \ \ \ \ 240 \ \ \ \ return\ 

      \ \ \ \ \ \ \ \ 250 \ \ \ \ and or xor

      \ \ \ \ \ \ \ \ 260 \ \ \ \ in at contains

      \ \ \ \ \ \ \ \ 271 \ \ \ \ of to

      \ \ \ \ \ \ \ \ 280 \ \ \ \ .. by

      \ \ \ \ \ \ \ \ 290 \ \ \ \ = \<less\> \<gtr\> \<less\>= \<gtr\>=
      \<less\>\<gtr\>

      \ \ \ \ \ \ \ \ 300 \ \ \ \ & \|

      \ \ \ \ \ \ \ \ 310 \ \ \ \ + -

      \ \ \ \ \ \ \ \ 320 \ \ \ \ * / mod rem

      \ \ \ \ \ \ \ \ 381 \ \ \ \ ^

      \ \ \ \ \ \ \ \ 500 \ \ \ \ .

      \ \ \ \ \ \ \ \ 600 \ \ \ \ :

      \;

      PREFIX

      \ \ \ \ \ \ \ \ 30 \ \ \ \ \ data

      \ \ \ \ \ \ \ \ 40 \ \ \ \ \ loop while until

      \ \ \ \ \ \ \ \ 50 \ \ \ \ \ property constraint

      \ \ \ \ \ \ \ \ 121 \ \ \ \ case if return yield transform

      \ \ \ \ \ \ \ \ 350 \ \ \ \ not in out constant variable const var

      \ \ \ \ \ \ \ \ 360 \ \ \ \ ! ~

      \ \ \ \ \ \ \ \ 370 \ \ \ \ - + * /

      \ \ \ \ \ \ \ \ 401 \ \ \ \ FUNCTION

      \ \ \ \ \ \ \ \ 410 \ \ \ \ function procedure to type iterator

      \ \ \ \ \ \ \ \ 420 \ \ \ \ ++ --

      \ \ \ \ \ \ \ \ 430 \ \ \ \ &

      \;

      POSTFIX

      \ \ \ \ \ \ \ \ 400 \ \ \ \ ! ? % cm inch mm pt px

      \ \ \ \ \ \ \ \ 420 \ \ \ \ ++ --

      \;

      BLOCK

      \ \ \ \ \ \ \ \ 5 \ \ \ \ \ \ INDENT UNINDENT

      \ \ \ \ \ \ \ \ 25 \ \ \ \ \ '{' '}'

      \ \ \ \ \ \ \ \ 500 \ \ \ \ '(' ')' '[' ']'

      \;

      TEXT

      \ \ \ \ \ \ \ \ "\<less\>\<less\>" "\<gtr\>\<gtr\>"

      \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ 

      COMMENT

      \ \ \ \ \ \ \ \ "//" NEWLINE

      \ \ \ \ \ \ \ \ "/*" "*/"

      \;

      SYNTAX "C"

      \ \ \ \ \ \ \ \ extern ;
    </verbatim>
  </big-figure|<label|syntax-file>Default syntax configuration file>

  Spaces and indentation are not significant in a syntax configuration file.
  Lexical elements are identical to those of XLR, as detailed in
  Section<nbsp><reference|literals>. The significant elements are integer
  constants, names, symbols and text. Integer constants are interpreted as
  the precedence of names and symbols that follow them. Name and symbols can
  be given either with lexical names and symbols, or with text.

  A few names are reserved for use as keywords in the syntax configuration
  file:

  <\itemize>
    <item><verbatim|INFIX> begins a section declaring infix symbols and
    precedence. In this section:

    <\itemize>
      <item><verbatim|NEWLINE> identifies line break characters in the source
      code

      <item><verbatim|STATEMENT> identifies the precedence of statements

      <item><verbatim|DEFAULT> identifies the precedence for symbols not
      otherwise given a precedence. This precedence should be unique in the
      syntax confguration, i.e. no other symbol should be given the
      <verbatim|DEFAULT> precedence.
    </itemize>

    <item><verbatim|PREFIX> begins a section declaring prefix symbols and
    precedence. In this section:

    <\itemize>
      <item><verbatim|FUNCTION >identifies the precedence for default prefix
      symbols, i.e. symbols identified as prefix that are not otherwise given
      a precedence. This precedence should be unique, i.e. no other symbol
      shoud be given the <verbatim|FUNCTION> precedence.
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

    <item><verbatim|SYNTAX> begins a section declaring external syntax files.
    In normal XLR, a file <verbatim|C.syntax> is used to define the
    precedences for any text between <verbatim|extern> and <verbatim|;>
    symbols. This is used to import C symbols using an approximation of the
    syntax of the C language, as described in
    Section<nbsp><reference|C-library>. The <verbatim|C.syntax> configuration
    file is shown in Figure<nbsp><reference|C-syntax-file>.
  </itemize>

  <\big-figure>
    <\verbatim>
      INFIX

      \ \ \ \ \ \ \ \ 41 \ \ \ \ ,

      \;

      PREFIX

      \ \ \ \ \ \ \ \ 30 \ \ \ \ \ extern ...

      \ \ \ \ \ \ \ \ 400 \ \ \ \ FUNCTION

      \ \ \ \ \ \ \ \ 450 \ \ \ \ short long unsigned signed

      \;

      POSTFIX

      \ \ \ \ \ \ \ \ 100 \ \ \ \ *

      \;

      BLOCK

      \ \ \ \ \ \ \ \ 500 \ \ \ \ '(' ')' '[' ']'

      \;

      COMMENT

      \ \ \ \ \ \ \ \ "//" NEWLINE

      \ \ \ \ \ \ \ \ "/*" "*/"
    </verbatim>
  </big-figure|<label|C-syntax-file>C syntax configuration file>

  Syntax information can also be provided in the source code using the
  <verbatim|syntax> name followed by a block, as illustrated in
  Figure<nbsp><reference|source-syntax>.

  <big-figure|<\verbatim>
    // Declare infix 'weight' operator

    syntax (INFIX 350 weight)

    Obj weight W -\<gtr\> Obj = W

    \;

    // Declare postfix 'apples' and 'kg'

    syntax

    \ \ \ \ POSTFIX 390 apples kg

    X kg -\<gtr\> X * 1000

    N apples -\<gtr\> N * 0.250 kg

    \;

    // Combine the notations declared above

    if 6 apples weight 1.5 kg then

    \ \ \ \ write "Success!"
  </verbatim>|<label|source-syntax>Use of the <verbatim|syntax> specification
  in a source file>

  As a general stylistic rule, it is recommended limit the introduction of
  new operators using <verbatim|syntax> statements, as this can easily
  confuse a reader not familiar with the new notation.

  <section|Language semantics>

  The semantics of XLR is based entirely on the rewrite of XL0 abstract
  syntax trees. Tree rewrite operations define the execution of XLR programs,
  also called <em|evaluation>.

  <\subsection>
    <label|tree-rewrite-operators>Tree rewrite operators
  </subsection>

  There is a very small set of tree rewrite operators that are given special
  meaning in XLR and treated specially by the XLR compiler:

  <\itemize>
    <item>Rewrite declarations are used to declare operations. They roughly
    play the role of functions, operator or macro declarations in other
    programming languages. A rewrite declaration takes the general form
    <verbatim|Pattern-\<gtr\>Implementation> and indicates that any tree
    matching <verbatim|Pattern> should be rewritten as
    <verbatim|Implementation>.

    <item>Data declarations identify data structures in the program. Data
    structures are nothing more than trees that need no further rewrite. A
    data declaration takes the general form of <verbatim|data Pattern>. Any
    tree matching <verbatim|Pattern> will not be rewritten further.

    <item>Type declarations define the type of variables. Type declarations
    take the general form of an infix colon operator <verbatim|Name:Type>,
    with the name of the variable on the left, and the type of the variable
    on the right.

    <item>Guards limit the validity of rewrite or data declarations. They use
    an infix <verbatim|when> with a boolean expression on the right of
    <verbatim|when>, i.e. a form like <verbatim|Declaration when Condition>.

    <item>Assignment change the value associated to a binding. Assignments
    take the form <verbatim|Reference := Value>, where <verbatim|Reference>
    identifies the binding to change.

    <item>Sequence operators indicate the order in which computations must be
    performed. Standard XLR has two infix sequence operators, the semi-colon
    <verbatim|;> and the new-line <verbatim|NEWLINE>.

    <item>Index operators perform particular kinds of tree rewrites similar
    in usage to ``structures'' or ``arrays'' in other programming languages.
    The notations <verbatim|Reference.Field> and <verbatim|Reference[Index]>
    are used to refer to individual elements in a data structure. As we will
    see, these are only convenience notations for specific kinds of tree
    rewrites.
  </itemize>

  <subsubsection|Rewrite declarations>

  The infix <verbatim|-\<gtr\>> operator declares a tree rewrite.
  Figure<nbsp><reference|if-then-else> repeats the code in
  Figure<nbsp><reference|if-then> illustrating how rewrite declarations can
  be used to define the traditional <verbatim|if>-<verbatim|then>-<verbatim|else>
  statement.

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

    <item>A name alone on the left of a rewrite is a constant

    <item>All other names are variable
  </itemize>

  Figure<nbsp><reference|if-then-else-colorized> highlight in blue italic all
  variable symbols in the declarations of
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

  Note that there is a special case for a name as the pattern of a rewrite. A
  rewrite like <verbatim|X-\<gtr\>0> binds <verbatim|X> to value
  <verbatim|0>, i.e. <verbatim|X> is a constant that must match in the tree
  being evaluated.

  It is however possible to create a rewrite with a variable on the left by
  using a type declaration. For example, the rewrite
  <verbatim|(X:real-\<gtr\>X+1)> does not declare the variable <verbatim|X>,
  but an <em|anonymous function> that increments its input.

  Rewrites in a sequence belong to the context for the entire sequence
  (contexts are defined in Section<nbsp><reference|binding>). In other words,
  declarations are visible to prior elements in the sequence and do not need
  to be evaluated, as shown in Figure<nbsp><reference|out-of-order-declarations>,
  which computes <verbatim|4>:

  <big-figure|<\verbatim>
    foo 3

    foo N -\<gtr\> N + 1
  </verbatim>|<label|out-of-order-declarations>Declarations are visible to
  the entire sequence containing them>

  <paragraph|Machine interface>A <em|machine interface> is a rewrite where
  the implementation is a prefix of two names, the first one being
  <verbatim|C> or <verbatim|opcode>. Machine interfaces are described in
  Section<nbsp><reference|machine-interface>.

  <subsubsection|Data declaration>

  The <verbatim|data> prefix declares tree structures that need not be
  rewritten further. For instance, Figure<nbsp><reference|comma-separated-list>
  declares that <verbatim|1,3,4> should not be evaluated further, because it
  is made of infix <verbatim|,> trees which are declared as <verbatim|data>.

  <\big-figure>
    <\verbatim>
      data a,b
    </verbatim>
  </big-figure|<label|comma-separated-list>Declaring a comma-separated list>

  The tree following a data declaration is a pattern, with constant and
  variable symbols like for rewrite declarations. Data declarations only
  limit the rewrite of the tree specified by the pattern, but not the
  evaluation of pattern variables. In other words, pattern variables are
  evaluated normally, as specified in Section<nbsp><reference|evaluation>.
  For instance, in Figure<nbsp><reference|complex-type>, the names
  <verbatim|x> and <verbatim|y> are variable, but the name <verbatim|complex>
  is constant because it is a prefix. Assuming that addition is implemented
  for integer values, <verbatim|complex(3+4, 5+6)> will evaluate as
  <verbatim|complex(7,11)> but no further.

  <big-figure|<verbatim|data complex(x,y)>|<label|complex-type>Declaring a
  <verbatim|complex> data type>

  \ The declaration in Figure<nbsp><reference|complex-type> can be
  interpreted as declaring a <verbatim|complex> data type. There is, however,
  a better way to describe data types in XLR, which is detailed in
  Section<nbsp><reference|type-definition>.

  The word <verbatim|self> can be used to build data forms: <verbatim|data X>
  is equivalent to <verbatim|X-\<gtr\>self>.

  <subsubsection|Type declaration>

  An <em|type declaration> is an infix colon <verbatim|:> operator in a
  rewrite or data pattern with a name on the left and a type on the right. It
  indicates that the named parameter on the left has the type indicated on
  the right. A <em|return type declaration> is an infix <verbatim|=\<gtr\>>
  in a rewrite pattern with a pattern on the left and a type on the right. It
  specifies the value that will be returned by the implementation of the
  rewrite. Types are defined as explained in Section<nbsp><reference|types>.

  Figure<nbsp><reference|type-declaration> shows examples of type
  declarations. To match the pattern for <verbatim|polynom>, the arguments
  corresponding to parameters <verbatim|X> and <verbatim|Z> must be a
  <verbatim|real>, whereas the argument corresponding to parameter
  <verbatim|N> must be <verbatim|integer>. The value returned by
  <verbatim|polynom> will belong to <verbatim|real>.

  <big-figure|<\verbatim>
    <\verbatim>
      polynom X:real, Z:real, N:integer =\<gtr\> real -\<gtr\> (X-Z)^N
    </verbatim>
  </verbatim>|<label|type-declaration>Simple type declarations>

  A type declaration can also be placed on the left of an assignment, see
  Section<nbsp><reference|assignment>.

  <subsubsection|Assignment><label|assignment>

  The assignment operator <verbatim|:=> binds the reference on its left to
  the value of the tree on its right. The tree on the right is evaluated
  prior to the assignment.

  An assignment is valid even if the reference on the left of <verbatim|:=>
  had not previously been bound. The assignment creates a local binding for
  that reference if there was no previous binding, or replaces the value of
  the existing binding if there was one. This is shown in
  Figure<nbsp><reference|local-and-nonlocal-assignment>:\ 

  <\big-figure>
    <\verbatim>
      // Global X

      X := 0

      \;

      // Assigns to global X above

      assigns_to_global -\<gtr\> X := 1

      \;

      // Assigns to local Y, there is no global Y

      assigns_to_local -\<gtr\> Y := 2
    </verbatim>
  </big-figure|<label|local-and-nonlocal-assignment>Local and non-local
  assignments>

  If the left side of an assignment is a type declaration, the assignment
  always creates a new binding in the local scope, as illustrated in . That
  binding has a return type declaration associated with it, so that later
  assignments to that same name will only succeed if the type of the assigned
  value matches the previously declared type. This is shown in
  Figure<nbsp><reference|assign-to-new-local>:

  <big-figure|<\verbatim>
    // Global X

    X := 0

    \;

    // Assign to new local X

    assigns_new -\<gtr\> X:integer := 1
  </verbatim>|<label|assign-to-new-local>Assigning to new local variable>

  Using an assignment in an expression is equivalent to using the value bound
  to the variable after the assignment. For instance, <verbatim|sin(x:=f(0))>
  is equivalent to <verbatim|x:=f(0)> followed by <verbatim|sin(x)>.

  An assignment can override a binding established by a parameter or rewrite
  declaration. However, assigning to a pattern proves tricky because of lazy
  evaluation (see Section<nbsp><reference|lazy-evaluation>), and is best
  avoided. The rationale for this behavior is explained in
  Section<nbsp><reference|index-operators>.

  <big-figure|<\verbatim>
    // Declare rewrites for prefix foo

    foo 0 -\<gtr\> 3

    foo N -\<gtr\> 5

    \;

    // This works as expected

    foo 0 := 32

    \;

    // Illegal: on-demand evaluation of N fails

    foo N := 5

    \;

    // Legal, changes foo 0

    N := 0

    foo N := 44

    \;

    // Legal, creates foo 5

    N := 5

    foo N := 55
  </verbatim>|<label|assignments-cant-override-patterns>Assignments do not
  override patterns>

  <subsubsection|Guards>

  The infix <verbatim|when> operator in a rewrite or data pattern introduces
  a <em|guard>, i.e. a boolean condition that must be true for the pattern to
  apply.

  Figure<nbsp><reference|guard> shows an improved definition of the factorial
  function which only applies for non-negative values. This set of rewrites
  is ignored for a negative <verbatim|integer> value.

  <big-figure|<\verbatim>
    0! \ \ \ \ \ \ \ \ \ \ \ -\<gtr\> 1

    N! when N \<gtr\> 0 -\<gtr\> N * (N-1)!
  </verbatim>|<label|guard>Guard limit the validity of operations>

  A form where the guard cannot be evaluated or evaluates to anything but the
  value <verbatim|true> is not selected. For example, if we try to evaluate
  <verbatim|'ABC'!> the condition <verbatim|N\<gtr\>0> is equivalent to
  <verbatim|'ABC'\<gtr\>0>, which cannot be evaluated unless you added
  specific declarations. Therefore, the rewrite for <verbatim|N!> does not
  apply.

  <subsubsection|Sequences>

  The infix line-break <verbatim|NEWLINE> and semi-colon <verbatim|;>
  operators are used to introduce a sequence between statements. They ensure
  that the left node is evaluated entirely before the evaluation of the right
  node begins.

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

  Items in a sequence can be <em|declarations> or <em|statements>.
  Declarations include rewrite declarations, data declarations, type
  declarations and assignments to a type declaration. All other items in a
  sequence are statements.

  <subsubsection|Index operators><label|index-operators>

  The notation <verbatim|A[B]> and <verbatim|A.B> are used as index
  operators, i.e. to refer to individual items in a collection. The
  <verbatim|A[B]> notation is intended to represent array indexing
  operations, whereas the <verbatim|A.B> notation is intended to represent
  field indexing operations.

  For example, consider the declarations in
  Figure<nbsp><reference|structured-data>.

  <big-figure|<\verbatim>
    MyData -\<gtr\>

    \ \ \ \ Name \ -\<gtr\> "Name of my data"

    \ \ \ \ Value -\<gtr\> 3.45

    \ \ \ \ 1 -\<gtr\> "First"

    \ \ \ \ 2 -\<gtr\> "Second"

    \ \ \ \ 3 -\<gtr\> "Third"
  </verbatim>|<label|structured-data>Structured data>

  In that case, the expression <verbatim|MyData.Name> results in the value
  <verbatim|"Name of my data">. The expression <verbatim|MyData[1]> results
  in the value <verbatim|"First">.

  The two index operators differ when their right operand is a name. The
  notation <verbatim|A.B> evaluates <verbatim|B> in the context of
  <verbatim|A>, whereas <verbatim|A[B]> first evaluates <verbatim|B> in the
  current context, and then applies <verbatim|A> to it (it is actually
  nothing more than a regular tree rewrite). Therefore, the notation
  <verbatim|MyData.Value> returns the value <verbatim|3.45>, whereas the
  value of <verbatim|MyData[Value]> will evaluate <verbatim|Value> in the
  current context, and then apply <verbatim|MyData> to the result. For
  example, if we had <verbatim|Value-\<gtr\>3> in the current context, then
  <verbatim|MyData[Value]> would evaluate to <verbatim|"Third">.

  Users familiar with languages such as C may be somewhat disconcerted by
  XLR's index operators. The following points are critical for properly
  understanding them:

  <\itemize>
    <item>Arrays, structures and functions are all represented the same way.
    The entity called <verbatim|MyData> can be interpreted as an array in
    <verbatim|MyData[3]>, as a structure in <verbatim|MyData.Name>, or as a
    function if one writes <verbatim|MyData 3>. In reality, there is no
    difference between <verbatim|MyData[3]> and <verbatim|MyData 3>: the
    former simply passes a block as an argument, i.e. it is exactly
    equivalent to <verbatim|MyData(3)>, <verbatim|MyData{3}>. Writing
    <verbatim|MyData[3]> is only a way to document an intent to use
    <verbatim|MyData> as an array, but does not change the implementation.

    <item>Data structures can be extended on the fly. For example, it is
    permitted to assign something to a non-existent binding in
    <verbatim|MyData>, e.g. by writing <verbatim|MyData[4]:=3>. The ability
    to add ``fields'' to a data structure on the fly makes it easier to
    extend existing code.

    <item>Data structures can include other kinds of rewrites, for example
    ``functions'', enabling object-oriented data structures. This is
    demonstrated in Section<nbsp><reference|object-oriented-programming>.

    <item>Since the notation <verbatim|A.B> simply evaluates <verbatim|B> in
    the context of <verbatim|A>, the value of <verbatim|MyData.4> is...
    <verbatim|4>: there is no rewrite for <verbatim|4> in <verbatim|MyData>,
    therefore it evaluates a itself.
  </itemize>

  <subsection|<label|binding>Binding References to Values>

  A rewrite declaration of the form <verbatim|Pattern-\<gtr\>Implementation>
  is said to <em|bind> its pattern to its implementation. A sequence of
  declarations is called a <em|context>. For example,
  <verbatim|{x-\<gtr\>3;y-\<gtr\>4}> is a context that binds <verbatim|x> to
  <verbatim|3> and <verbatim|y> to <verbatim|4>.

  <subsubsection|Context Order>

  A context may contain multiple rewrites that hide one another.

  For example, in the context <verbatim|{x-\<gtr\>0;x-\<gtr\>1}>, the name
  <verbatim|x> is bound twice. The evaluation of <verbatim|x> in that context
  will return <verbatim|0> because rewrites are tested in order. In other
  words, the declaration <verbatim|x-\<gtr\>0> <em|shadows> the declaration
  <verbatim|x-\<gtr\>1> in that context.

  For the purpose of finding the first match, a context is traversed depth
  first in left-to-right order, which is called <em|context order>.

  <subsubsection|Scoping>

  The left child of a context is called the <em|local scope>. The right child
  of a context is the <em|enclosing context>. All other left children in the
  sequence are the local scopes of expressions currently being evaluated. The
  first one being the <em|enclosing scope> (i.e. the local scope of the
  enclosing context) and the last one being the <em|global scope>.

  This ensures that local declarations hide declarations from the surrounding
  context, since they are on the left of the right child, while allowing
  local declarations in the left child of the context to be kept in program
  order, so that the later ones are shadowed by the earlier ones.

  The child at the far right of a context is a catch-all rewrite intended to
  specify what happens when evaluating an undefined form.

  <subsubsection|Current context>

  Any evaluation in XLR is performed in a context called the <em|current
  context>. The current context is updated by the following operations:

  <\enumerate>
    <item>Evaluating the implementation of a rewrite creates a scope binding
    all arguments to the corresponding parameters, then a new context with
    that scope as its left child and the old context as its right child. The
    implementation is then evaluated in the newly created context.

    <item>Evaluating a sequence initializes a local context with all
    declarations in that sequence, and creates a new current context with the
    newly created local context as its left child and the old context as its
    right child. Statements in the sequence are then evaluated in the newly
    created context.

    <item>Evaluating an assignment changes the implementation of an existing
    binding if there is one in the current context, or otherwise creates a
    new binding in the local scope.
  </enumerate>

  <subsubsection|References>

  An expression that can be placed on the left of an assignment to identify a
  particular binding is called a <em|reference>. A reference can be any
  pattern that would go on the left of a rewrite. In addition, it can be an
  index operator:

  <\itemize>
    <item>If <verbatim|A> refers to a context, assigning to <verbatim|A.B>
    will affect the binding of <verbatim|B> in the context <verbatim|A>, and
    not the binding of <verbatim|A.B> in the current context.

    <item>If <verbatim|A> refers to a context, assigning to <verbatim|A[B]>
    (or, equivalently, to <verbatim|A{B}>, <verbatim|A(B)> or <verbatim|A B>)
    will affect the binding corresponding to <verbatim|B> in the context of
    <verbatim|A>, not the binding of <verbatim|A B> in the current context.
    The index <verbatim|B> will be evaluated in the current context if
    required to match patterns in <verbatim|A>, as explained in
    Section<nbsp><reference|evaluation>.
  </itemize>

  <subsection|<label|evaluation>Evaluation>

  Evaluation is the process through which a given tree is rewritten.

  <subsubsection|Standard evaluation><label|standard-evaluation>

  Except for special forms described later, the evaluation of XLR trees is
  performed as follows:

  <\enumerate>
    <item>The tree to evaluate, <verbatim|T>, is matched against the
    available data and rewrite pattern. <verbatim|3*4+5> will match
    <verbatim|A*B+C> as well as <verbatim|A+B> (since the outermost tree is
    an infix <verbatim|+> as in <verbatim|A+B>).

    <item>Possible matches are tested in <em|context order> (defined in
    Section<nbsp><reference|binding>) against the tree to evaluate. The first
    matching tree is selected. For example, in
    Figure<nbsp><reference|factorial>, <verbatim|(N-1)!> will be matched
    against the rules <verbatim|0!> and <verbatim|N!> in this order.

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
      evaluating both <verbatim|A> and the test value.

      <item>Type declarations in <verbatim|P> match if the result of
      evaluating the corresponding fragment in <verbatim|T> has the declared
      type, as defined in Section<nbsp><reference|types>. In that case, the
      variable being declared is bound to the evaluated value.

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
      <item>Integer, real and text terminals evaluates as themselves.

      <item>A block evaluates as the result of evaluating its child.

      <item>If the tree is a prefix with the left being a name containing
      <verbatim|error>, then the program immediatly aborts and shows the
      offending tree. This case corresponds to an unhandled error.

      <item>For a prefix node or postfix tree, the operator child (i.e. the
      left child for prefix and the right child for postfix) is evaluated,
      and if the result is different from the original operator child,
      evaluation is attempted again after replacing the original operator
      child with its evaluated version.

      <item>In any other case, the tree is prefixed with
      <verbatim|evaluation_error> and the result is evaluated. For example,
      <verbatim|<math|>$foo> will be transformed into
      <verbatim|evaluation_error $foo>. A prefix rewrite for
      <verbatim|evaluation_error> is supposed to handle such errors.
    </itemize>

    <item>If a match is found, variables in the first matching pattern
    (called <em|parameters>) are bound to the corresponding fragments of the
    tree to evaluate (called <em|arguments>).

    <\itemize>
      <item>If an argument was evaluated (including as required for
      comparison with an earlier pattern), then the corresponding parameter
      is bound with the evaluated version.

      <item>If the argument was not evaluated, the corresponding parameter is
      bound with the tree fragment in context, as explained in
      Section<nbsp><reference|binding>. In line with the terminology used in
      functional languages, this context-including binding is called a
      <em|closure>.
    </itemize>

    <item>Once all bindings have been performed, the implementation
    corresponding to the pattern in the previous step is itself evaluated.
    The result of the evaluation of the original form is the result of
    evaluating the implementation in the new context created by adding to the
    original context the bindings of parameters to their arguments. For a
    data form, the result of evaluation is the pattern after replacing
    parameters with the corresponding arguments.
  </enumerate>

  <subsubsection|Special forms>

  Some forms have a special meaning and are evaluated specially:

  <\enumerate>
    <item>A terminal node (integer, real, type, name) evaluates as itself,
    unless there is an explicit rewrite rule for it<\footnote>
      There several use cases for allowing rewrite rules for integer, real or
      text constants, notably to implement data maps such as
      <verbatim|(1-\<gtr\>0; 0-\<gtr\>1)>, also known as associative arrays.
    </footnote>.

    <item>A block evaluate as the result of evaluating its child.

    <item>A rewrite rule evaluates as itself.

    <item>A data declaration evaluates as itself

    <item>An assignment binds the variable and evaluates as the named
    variable after assignment

    <item>Evaluating a sequence creates a new local context with all
    declarations in the sequence, then evaluates all its statements in order
    in that new local context. The result of evaluation is the result of the
    last statement, if there is one, or the newly created context if the
    sequence only contains declarations.

    <item>A prefix with a sequence on the left evaluates as if the right
    child was added at the end of the sequence, and the sequence then
    evaluated. In other words, evaluating <verbatim|(A;B) C> is equivalent to
    evaluating <verbatim|(A;B;C)>. For example,
    <verbatim|(x-\<gtr\>3;y-\<gtr\>4)(x+y)> evaluates as <verbatim|7>, and
    <verbatim|(x:integer-\<gtr\>x+1) 3> evaluates as <verbatim|4>.
  </enumerate>

  <subsubsection|Lazy evaluation><label|lazy-evaluation>

  When an argument is bound to a parameter, it is associated to a context
  which allows correct evaluation at a later time, but the argument is in
  general not evaluated immediately. Instead, it is only evaluated when
  evaluation becomes necessary for the program to execute correctly. This
  technique is called <em|lazy evaluation>. It is intended to minimize
  unnecessary evaluations.

  Evaluation of an argument before binding it to its parameter occurs in the
  following cases, collectively called <em|demand-based evaluation>:

  <\enumerate>
    <item>The argument is compared to a constant value or bound name, see
    Section<nbsp><reference|standard-evaluation>, and the static value of the
    tree is not sufficient to perform the comparison. For example, in
    Figure<nbsp><reference|evaluation-for-comparison>, the expression
    <verbatim|4+X> requires evaluation of <verbatim|X> for comparison with
    <verbatim|4> to check if it matches <verbatim|A+A>; the expression
    <verbatim|B+B> can be statically bound to the form <verbatim|A+A> without
    requiring evaluation of <verbatim|B>; finally, in <verbatim|B+C>, both
    <verbatim|B> and <verbatim|C> need to be evaluated to compare if they are
    equal and if the form <verbatim|A+A> matches.

    <big-figure|<\verbatim>
      A+A -\<gtr\> 2*A

      4+X \ // X evaluated

      B+B \ // B not evaluated

      B+C \ // B and C evaluated
    </verbatim>|<label|evaluation-for-comparison>Evaluation for comparison>

    <item>The argument is tested against a parameter with a type declaration,
    and the static type of the tree is not sufficient to guarantee a match.
    For example, in Figure<nbsp><reference|evaluation-for-type-comparison>,
    the expression <verbatim|Z+1> can statically be found to match the form
    <verbatim|X+Y>, so <verbatim|Z> needn't be evaluated. On the other hand,
    in <verbatim|1+Z>, it is necessary to evaluate <verbatim|Z> to type-check
    it against <verbatim|integer>.

    <big-figure|<\verbatim>
      X:tree + Y:integer -\<gtr\> ...

      Z + 1 // Z not evaluated

      1 + Z // Z evaluated
    </verbatim>|<label|evaluation-for-type-comparison>Evaluation for type
    comparison>

    <item>A specific case of the above scenario is the left side of any index
    operator. In <verbatim|A.B> or <verbatim|A[B]>, the value <verbatim|A>
    needs to be evaluated to verify if it contains <verbatim|B>.
  </enumerate>

  Expressions are also evaluated in the following cases, collectively called
  <em|explicit evaluation>:

  <\enumerate>
    <item>An expression on the left or right of a sequence is evaluated. For
    example, in <verbatim|A;B>, the names <verbatim|A> and <verbatim|B> will
    be evaluated.

    <item>The prefix <verbatim|do> forces evaluation of its argument. For
    example, <verbatim|do X> will force the evaluation of <verbatim|X>.

    <item>The program itself is evaluated. Most useful programs are
    sequences.
  </enumerate>

  Whenever a parameter is evaluated, the evaluated result may be used for all
  subsequent demand-based evaluations, but not for explicit evaluations. This
  is illustrated with the example in Figure<nbsp><reference|explicit-vs-lazy-evaluation>:

  <big-figure|<\verbatim>
    foo X -\<gtr\>

    \ \ \ \ X

    \ \ \ \ if X then writeln "X is true"

    \ \ \ \ if do X then writeln "X is true"

    \ \ \ \ X

    bar -\<gtr\>

    \ \ \ \ writeln "bar evaluated"

    \ \ \ \ true

    foo bar
  </verbatim>|<label|explicit-vs-lazy-evaluation>Explicit vs. lazy
  evaluation>

  In Figure<nbsp><reference|explicit-vs-lazy-evaluation>, evaluation happens
  as follows:

  <\enumerate>
    <item>The expression <verbatim|foo bar> is evaluated explicitly, being
    part of a sequence. This matches the rewrite for <verbatim|foo X> on the
    first line.

    <item>The first reference to <verbatim|X> in the implementation is
    evaluated explicitly. This causes the message <samp|bar evaluated> to be
    written to the console.

    <item>The second reference to <verbatim|X> is demand-based, but since
    <verbatim|X> has already been evaluated, the result <verbatim|true> is
    used directly. The message <samp|X is true> is emitted on the console,
    but the message <samp|bar evaluated> is not.

    <item>The third reference to <verbatim|X> is an argument to
    <verbatim|do>, so it is evaluated again, which writes the message
    <samp|bar evaluated> on the console.

    <item>The last reference to <verbatim|X> is another explicit evaluation,
    so the message <samp|bar evaluated> is written on the console again.
  </enumerate>

  The purpose of these evaluation rules is to allow the programmer to pass
  code to be evaluated as an argument, while at the same time minimizing the
  number of repeated evaluations when a parameter is used for its value. In
  explicit evaluation, the value of the parameter is not used, making it
  clear that what matters is the effect of evaluation itself. In demand-based
  evaluation, it is on the contrary assumed that what matters is the result
  of the evaluation, not the process of evaluating it. It is always possible
  to force evaluation explicitly using <verbatim|do>.

  <subsection|<label|types>Types>

  Types are expressions that appear on the right of the colon operator
  <verbatim|:> in type declarations. In XLR, a type identifies the <em|shape>
  of a tree. A value is said to <em|belong> to a type if it matches the shape
  defined by the type. A value may belong to multiple types.

  <subsubsection|Predefined types>

  The following types are predefined:

  <\itemize>
    <item><verbatim|integer> matches integer constants

    <item><verbatim|real> matches real constants

    <item><verbatim|text> matches text constants

    <item><verbatim|symbol> matches names and operator symbols

    <item><verbatim|name> matches names only

    <item><verbatim|operator> matches operator symbols only

    <item><verbatim|infix> matches infix nodes

    <item><verbatim|prefix> matches prefix nodes

    <item><verbatim|postfix> matches postfix nodes

    <item><verbatim|block> matches block nodes

    <item><verbatim|tree> matches any tree

    <item><verbatim|boolean> matches the names <verbatim|true> and
    <verbatim|false>.
  </itemize>

  <subsubsection|Type definition><label|type-definition>

  A <em|type definition> for type <verbatim|T> is a special form of tree
  rewrite declaration where the pattern has the form <verbatim|type X>. A
  type definition declares a type name, and the pattern that the type must
  match. For example, Figure<nbsp><reference|simple-type> declares a type
  named <verbatim|complex> requiring two real numbers called <verbatim|re>
  and <verbatim|im>, and another type named <verbatim|ifte> that contains
  three arbitrary trees called <verbatim|Cond>, <verbatim|TrueC> and
  <verbatim|FalseC>.

  <big-figure|<\verbatim>
    complex -\<gtr\> type (re:real; im:real)

    ifte -\<gtr\> type {if Cond then TrueC else FalseC}

    block_type -\<gtr\> type[(BlockChild)]
  </verbatim>|<label|simple-type>Simple type declaration>

  The outermost block of a type pattern, if it exists, is not part of the
  type pattern. To create a type matching a specific block shape, two nested
  bocks are required, as illustrated with <verbatim|paren_block_type> in
  Figure<nbsp><reference|block-type-declaration>:

  <big-figure|<\verbatim>
    paren_block_type -\<gtr\> type((BlockChild))
  </verbatim>|<label|block-type-declaration>Simple type declaration>

  Note that type definitions and type declarations should not be confused. A
  type <em|definition> defines a type and has the form <verbatim|Name
  -\<gtr\> type TypePattern>, whereas a type <em|declaration> declares the
  type of an entity and has the form <verbatim|Name:Type>. The type defined
  by a type definition can be used on the right of a type declaration. For
  example, Figure<nbsp><reference|using-complex> shows how to use the
  <verbatim|complex> type defined in Figure<nbsp><reference|simple-type> in
  parameters.

  <big-figure|<\verbatim>
    Z1:complex+Z2:complex -\<gtr\> (Z1.re+Z2.re; Z1.im+Z2.im)
  </verbatim>|<label|using-complex>Using the <verbatim|complex> type>

  Parameters of types such as <verbatim|complex> are contexts with
  declarations for the individual variables of the pattern of the type. For
  example, a <verbatim|complex> like <verbatim|Z1> in
  Figure<nbsp><reference|using-complex> contains a rewrite for <verbatim|re>
  and a rewrite for <verbatim|im>. Figure<nbsp><reference|binding-for-complex-parameter>
  possible bindings when using the complex addition operator defined in
  Figure<nbsp><reference|using-complex>. The standard index notation
  described in Section<nbsp><reference|index-operators> applies, e.g. in
  <verbatim|Z1.re>, and these bindings can be assigned to.

  <big-figure|<\verbatim>
    // Expression being evaluated

    (3.4;5.2)+(0.4;2.22)

    \;

    // Possible resulting bindings

    // in the implementation of +

    Z1 -\<gtr\>

    \ \ \ \ re-\<gtr\>3.4

    \ \ \ \ im-\<gtr\>5.2

    \ \ \ \ (re; im)

    Z2 -\<gtr\>

    \ \ \ \ re-\<gtr\>0.4

    \ \ \ \ im-\<gtr\>2.22

    \ \ \ \ (re;im)
  </verbatim>|<label|binding-for-complex-parameter>Binding for a
  <verbatim|complex> parameter>

  Figure<nbsp><reference|making-two-types-equivalent> shows two ways to make
  type <verbatim|A> equivalent to type <verbatim|B>:

  <big-figure|<\verbatim>
    A -\<gtr\> B

    A -\<gtr\> type X:B
  </verbatim>|<label|making-two-types-equivalent>Making type <verbatim|A>
  equivalent to type <verbatim|B>>

  <subsubsection|Normal form for a type>

  By default, the name of a type is not part of the pattern being recognized.
  It is often recommended to make data types easier to identify by making the
  pattern more specific, for instance by including the type name in the
  pattern itself, as shown in Figure<nbsp><reference|more-specific-complex-types>:

  <big-figure|<\verbatim>
    complex -\<gtr\> type complex(re:real; im:real)
  </verbatim>|<label|more-specific-complex-types>Named patterns for
  <verbatim|complex>>

  In general, multiple notations for a same type can coexist. In that case,
  it is necessary to define a form for trees that the other possible forms
  will reduce to. This form is called the <em|normal form>. This is
  illustrated in Figure<nbsp><reference|complex-normal-form>, where the
  normal form is <verbatim|complex(re;im)> and the other notations are
  rewritten to this normal form for convenience.

  <big-figure|<\verbatim>
    // Normal form for the complex type

    complex -\<gtr\> type complex(re:real; im:real)

    \;

    // Other possible notations that reduce to the normal form

    i -\<gtr\> complex(0;1)

    A:real + i*B:real -\<gtr\> complex(A;B)

    A:real + B:real*i -\<gtr\> complex(A;B)
  </verbatim>|<label|complex-normal-form>Creating a normal form for the
  complex type>

  <subsubsection|Properties>

  A <em|properties definition> is a rewrite declaration like the one shown in
  Figure<nbsp><reference|properties-declaration> where:

  <\enumerate>
    <item>The implementation is a prefix with the name <verbatim|properties>
    followed by a block.

    <item>The block contains a sequence of type declarations
    <verbatim|Name:Type> or assignments to type declarations
    <verbatim|Name:Type:=DefaultValue>.
  </enumerate>

  <big-figure|<\verbatim>
    color -\<gtr\> properties

    \ \ \ \ red \ \ : real

    \ \ \ \ green : real

    \ \ \ \ blue \ : real

    \ \ \ \ alpha : real := 1.0
  </verbatim>|<label|properties-declaration>Properties declaration>

  Properties parameters match any block for which all the properties are
  defined. Properties are defined either if they exist in the argument's
  context, or if they are explicitly set in the block argument, or if a
  <em|default value> was assigned to the property in the properties
  declaration. An individual property can be set using an assignment or by
  using the property name as a prefix. For example,
  Figure<nbsp><reference|color-properties> shows how the <verbatim|color>
  type defined in Figure<nbsp><reference|properties-declaration> can be used
  in a parameter declaration, and how a <verbatim|color> argument can be
  passed.

  <big-figure|<\verbatim>
    write C:color -\<gtr\>

    \ \ \ \ write "R", C.red

    \ \ \ \ write "G", C.green

    \ \ \ \ write "B", C.blue

    \ \ \ \ write "A", C.alpha

    write_color { red 0.5; green 0.2; blue 0.6 }
  </verbatim>|<label|color-properties>Color properties>

  Properties parameters are contexts containing local declarations called
  <em|getters> and <em|setters> for each individual property:

  <\itemize>
    <item>The setter is a prefix taking an argument of the property's type,
    and setting the property's value to its argument.

    <item>The getter returns the value of the property in the argument's
    context (which may be actually set in the argument's enclosing contexts),
    or the default value if the property is not bound in the argument's
    context.
  </itemize>

  This makes it possible to set default value in the caller's context, which
  will be injected in the argument, as illustrated in
  Figure<nbsp><reference|setting-default-arguments>, where the expression
  <verbatim|C.red> in <verbatim|write_color> will evaluate to <verbatim|0.5>,
  and the argument <verbatim|C.alpha> will evaluate to <verbatim|1.0> as
  specified by the default value:

  <big-figure|<\verbatim>
    red := 0.5

    write_color (blue 0.6; green 0.2)
  </verbatim>|<label|setting-default-arguments>Setting default arguments from
  the current context>

  It is sufficient for the block argument to define all required properties.
  The block argument may also contain more code than just the references to
  the setters, as illustrated in Figure<nbsp><reference|extra-code-for-properties>:

  <big-figure|<\verbatim>
    write_color

    \ \ \ \ X:real := 0.444 * sin time

    \ \ \ \ if X \<less\> 0 then X := 1.0+X

    \ \ \ \ red X

    \ \ \ \ green X^2

    \ \ \ \ blue X^3
  </verbatim>|<label|extra-code-for-properties>Additional code in properties>

  \;

  <subsubsection|Data inheritance>

  Properties declarations may <em|inherit> data from one or more other types
  by using one or more <verbatim|inherit> prefixes in the properties
  declaration, as illustrated in Figure<nbsp><reference|data-inheritance>,
  where the type <verbatim|rgb> contains three properties called
  <verbatim|red>, <verbatim|green> and <verbatim|blue>, and the type
  <verbatim|rgba> additionally contains an <verbatim|alpha> property:

  <big-figure|<\verbatim>
    rgb -\<gtr\> properties

    \ \ \ \ red \ \ : real

    \ \ \ \ green : real

    \ \ \ \ blue \ : real

    rgba -\<gtr\> properties

    \ \ \ \ inherit rgb

    \ \ \ \ alpha : real
  </verbatim>|<label|data-inheritance>Data inheritance>

  Only declarations are inherited in this manner. The resulting types are not
  compatible, although they can be made compatible using automatic type
  conversions (see Section<nbsp><reference|type-conversions>).

  <subsubsection|Explicit type check>

  Internally, a type is any context where a <verbatim|contains> prefix can be
  evaluated. In such a context, the expression <verbatim|contains X> is
  called a <em|type check> for the type and for value <verbatim|X>. A type
  check must return a boolean value to indicate if the value <verbatim|X>
  belongs to the given type.

  Type checks can be declared explicitly to create types identifying
  arbitrary forms of trees that would be otherwise difficult to specify. This
  is illustrated in Figure<nbsp><reference|arbitrary-type> where we define an
  <verbatim|odd> type that contains only odd integers. We could similarly add
  a type check to the definition of <verbatim|rgb> in
  Figure<nbsp><reference|data-inheritance> to make sure that <verbatim|red>,
  <verbatim|green> and <verbatim|blue> are between <verbatim|0.0> and
  <verbatim|1.0>.

  <big-figure|<\verbatim>
    odd -\<gtr\>

    \ \ \ \ contains X:integer -\<gtr\> X mod 2 = 1

    \ \ \ \ contains X -\<gtr\> false
  </verbatim>|<label|arbitrary-type>Defining a type identifying an arbitrary
  AST shape>

  The type check for a type can be invoked explicitly using the infix
  <verbatim|contains> (with the type on the left) or <verbatim|is_a> (with
  the type on the right). \ This is shown in
  Figure<nbsp><reference|contains-tests>. The first type check <verbatim|odd
  contains 3> should return <verbatim|true>, since <verbatim|3> belongs to
  the <verbatim|odd> type. The second type check should return
  <verbatim|false> since <verbatim|rgb> expects the property <verbatim|blue>
  to be set.

  <big-figure|<\verbatim>
    if odd contains 3 then pass else fail

    if (red 1; green 1) is_a rgb then fail else pass
  </verbatim>|<label|contains-tests>Explicit type check>

  <subsubsection|Explicit and automatic type
  conversions><label|type-conversions>

  Prefix forms with the same name as a type can be provided to make it easy
  to convert values to type <verbatim|T>. Such forms are called <em|explicit
  type conversions>. This is illustrated in
  Figure<nbsp><reference|explicit-type-conversion>:

  <big-figure|<\verbatim>
    rgba C:rgb \ -\<gtr\> (red C.red; green C.green; blue C.blue; alpha 1.0)

    rgb \ C:rgba -\<gtr\> (red C.red; green C.green; blue C.blue)
  </verbatim>|<label|explicit-type-conversion>Explicit type conversion>

  An <em|automatic type conversion> is an infix <verbatim|as> form with a
  type on the right. If such a form exists, it can be invoked to
  automatically convert a value to the type on the right of <verbatim|as>.
  This is illustrated in Figure<nbsp><reference|automatic-type-conversion>.

  <big-figure|<\verbatim>
    X:integer as real -\<gtr\> real X

    1+1.5 \ \ // 1.0+1.5 using conversion above
  </verbatim>|<label|automatic-type-conversion>Automatic type conversion>

  <subsubsection|Parameterized types>

  Since type definitions are just regular rewrites, a type definition may
  contain a more complex pattern on the left of the rewrite. This is
  illustrated in Figure<nbsp><reference|parameterized-type>, where we define
  a <verbatim|one_modulo N> type that generalizes the <verbatim|odd> type.

  <big-figure|<\verbatim>
    one_modulo N:integer -\<gtr\>

    \ \ \ \ contains X:integer -\<gtr\> X mod N = 1

    \ \ \ \ contains X -\<gtr\> false

    show X:(one_modulo 1)
  </verbatim>|<label|parameterized-type>Parameterized type>

  It is also possible to define tree forms that are neither name nor prefix.
  Figure<nbsp><reference|infix-type> shows how we can use an infix form with
  the <verbatim|..> operator to declare a range type.

  <big-figure|<\verbatim>
    Low:integer..High:integer -\<gtr\>

    \ \ \ \ contains X:integer -\<gtr\> X\<gtr\>=Low and X\<less\>=High

    \ \ \ \ contains X -\<gtr\> false

    foo X:1..5 -\<gtr\> write X
  </verbatim>|<label|infix-type>Declaring a range type using an infix form>

  <subsubsection|Rewrite types>

  The infix <verbatim|-\<gtr\>> operator can be used in a type definition to
  identify specific forms of rewrites that perform a particular kind of tree
  transformation. Figure<nbsp><reference|rewrite-type> illustrates this usage
  to declare an <verbatim|adder> type that will only match rewrites declaring
  an infix <verbatim|+> node:

  <big-figure|<\verbatim>
    adder -\<gtr\> type {X+Y -\<gtr\> Z}
  </verbatim>|<label|rewrite-type>Declaration of a rewrite type>

  <section|Standard XL library>

  The XLR language is intentionally very simple, with a strong focus on how
  to extend it rather than on built-in features. Most features that would be
  considered fundamental in other languages are implemented in the library in
  XLR. Implementing basic amenities that way is an important proof point to
  validate the initial design objective, extensibility of the language.

  <subsection|<label|built-ins>Built-in operations>

  A number of operations are defined by the core run-time of the language,
  and appear in the context used to evaluate any XLR program.

  This section decsribes the minimum list of operations available in any XLR
  program. Operator priorities are defined by the <verbatim|xl.syntax> file
  in Figure<nbsp><reference|syntax-file>. All operations listed in this
  section may be implemented specially in the compiler, or using regular
  rewrite rules defined in a particular file called <verbatim|builtins.xl>
  that is loaded by XLR before evaluating any program, or a combination of
  both.

  <subsubsection|Arithmetic>

  Arithmetic operators for <verbatim|integer> and <verbatim|real> values are
  listed in Table<nbsp><reference|arithmetic>, where <verbatim|x> and
  <verbatim|y> denote integer or real values. Arithmetic operators take
  arguments of the same type and return an argument of the same type. In
  addition, the power operator <strong|^> can take a first <verbatim|real>
  argument and an <verbatim|integer> second argument.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x+y>>|<cell|Addition>>|<row|<cell|<verbatim|x-y>>|<cell|Subtraction>>|<row|<cell|<verbatim|x*y>>|<cell|Multiplication>>|<row|<cell|<verbatim|x/y>>|<cell|Division>>|<row|<cell|<verbatim|x
  rem y>>|<cell|Remainder>>|<row|<cell|<verbatim|x mod
  y>>|<cell|Modulo>>|<row|<cell|<verbatim|x^y>>|<cell|Power>>|<row|<cell|<verbatim|-x>>|<cell|Negation>>|<row|<cell|<verbatim|x%>>|<cell|Percentage
  (<verbatim|x/100.0)>>>|<row|<cell|<verbatim|x!>>|<cell|Factorial>>>>>|<label|arithmetic>Arithmetic
  operations>

  <subsubsection|Comparison>

  Comparison operators can take <verbatim|integer>, <verbatim|real> or
  <verbatim|text> argument, both arguments being of the same type, and return
  a <verbatim|boolean> argument, which can be either <verbatim|true> or
  <verbatim|false>. Text is compared using the lexicographic order<\footnote>
    There is currently no locale-dependent text comparison.
  </footnote>.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x=y>>|<cell|Equal>>|<row|<cell|<verbatim|x\<less\>\<gtr\>y>>|<cell|Not
  equal>>|<row|<cell|<verbatim|x\<less\>y>>|<cell|Less-than>>|<row|<cell|<verbatim|x\<gtr\>y>>|<cell|Greater
  than>>|<row|<cell|<verbatim|x\<less\>=y>>|<cell|Less or
  equal>>|<row|<cell|<verbatim|x\<gtr\>=y>>|<cell|Greater or
  equal>>>>>|<label|arithmetic>Comparisons>

  <subsubsection|Bitwise arithmetic>

  Bitwise operators operate on the binary representation of
  <verbatim|integer> values, treating each bit indivudally.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x
  shl y>>|<cell|Shift <verbatim|x> left by <verbatim|y>
  bits>>|<row|<cell|<verbatim|x shr y>>|<cell|Shift <verbatim|x> right by
  <verbatim|y> bits>>|<row|<cell|<verbatim|x and y>>|<cell|Bitwise
  and>>|<row|<cell|<verbatim|x or y>>|<cell|Bitwise
  or>>|<row|<cell|<verbatim|x xor y>>|<cell|Bitwise exclusive
  or>>|<row|<cell|<verbatim|not x>>|<cell|Bitwise
  complement>>>>>|<label|bitwise-arithmetic>Bitwise arithmetic operations>

  <subsubsection|Boolean operations>

  Boolean operators operate on the names <verbatim|true> and
  <verbatim|false>.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<verbatim|x=y>>|<cell|Equal>>|<row|<cell|<verbatim|x\<less\>\<gtr\>y>>|<cell|Not
  equal>>|<row|<cell|<verbatim|x and y>>|<cell|Logical
  and>>|<row|<cell|<verbatim|x or y>>|<cell|Logical
  or>>|<row|<cell|<verbatim|x xor y>>|<cell|Logical exclusive
  or>>|<row|<cell|<verbatim|not x>>|<cell|Logical
  not>>>>>|<label|boolean-operations>Boolean operations>

  <subsubsection|Mathematical functions>

  Mathematical functions operate on <verbatim|real> numbers. The
  <verbatim|random> function can also take two <verbatim|integer> arguments,
  in which case it returns an <verbatim|integer> value.

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

  <subsubsection|Text functions>

  Text functions operate on <verbatim|text> values.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|x&y>>>>>>|<cell|Concatenation>>|<row|<cell|<verbatim|text_length
  x>>|<cell|Length of the text>>|<row|<cell|<verbatim|x at
  y>>|<cell|Character at position <verbatim|y> in
  <verbatim|x>>>>>>|<label|text-operations>Text operations>

  <subsubsection|Conversions>

  Conversions operations transform data from one type to another.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|real
  x:integer>>>>>>|<cell|Convert integer to real>>|<row|<cell|<verbatim|real
  x:text>>|<cell|Convert text to real>>|<row|<cell|<verbatim|integer
  x:real>>|<cell|Convert real to integer>>|<row|<cell|<verbatim|integer
  x:text>>|<cell|Convert text to real>>|<row|<cell|<verbatim|text
  x:integer>>|<cell|Convert integer to text>>|<row|<cell|<verbatim|text
  x:real>>|<cell|Convert real to text>>>>>|<label|conversions>Conversions>

  \;

  <subsubsection|Date and time>

  Date and time functions manipulates time. Time is expressed with an integer
  representing a number of seconds since a time origin. Except for
  <verbatim|system_time> which never takes an argument, the functions can
  either take an explicit time represented as an <verbatim|integer> as
  returned by <verbatim|system_time>, or apply to the current time in the
  current time zone.

  <big-table|<block*|<tformat|<table|<row|<cell|<strong|Form>>|<cell|<strong|Description>>>|<row|<cell|<tformat|<table|<row|<cell|<verbatim|hours>>>>>>|<cell|Hours>>|<row|<cell|<verbatim|minutes>>|<cell|Minutes>>|<row|<cell|<verbatim|seconds>>|<cell|Seconds>>|<row|<cell|<verbatim|year>>|<cell|Year>>|<row|<cell|<verbatim|month>>|<cell|Month>>|<row|<cell|<verbatim|day>>|<cell|Day
  of the month>>|<row|<cell|<verbatim|week_day>>|<cell|Day of the
  week>>|<row|<cell|<verbatim|year_day>>|<cell|Day of the
  year>>|<row|<cell|<verbatim|system_time>>|<cell|Current time in
  seconds>>>>>|<label|time-operations>Date and time>

  <subsubsection|Tree operations>

  Tree operations are intended to manipulate trees.

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
    <item>Map: If <verbatim|x> is a name or an anonymous function, a list is
    built by mapping the name to each element of the list in turn. For
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

  <subsubsection|Data loading operations>

  Data loading operations read a data file and convert it to an XLR parse
  tree suitable for XLR data manipulation functions. The arguments are:

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

  <subsection|Library-defined types>

  A number of types are defined in the library.

  <subsubsection|Union types>

  The notation <verbatim|A\|B> in types is a <em|union type> for <verbatim|A>
  and <verbatim|B>, i.e. a type that can accept any element of types
  <verbatim|A> or <verbatim|B>. It is pre-defined in the standard library in
  a way subtantially equivalent to Figure<nbsp><reference|union-type-definition>:

  <big-figure|<\verbatim>
    type A\|B -\<gtr\>

    \ \ \ \ contains X:A -\<gtr\> true

    \ \ \ \ contains X:B -\<gtr\> true

    \ \ \ \ contains X -\<gtr\> false
  </verbatim>|<label|union-type-definition>Union type definition>

  Union types facilitate the definition of functions that work correctly on a
  multiplicity of data types, but not necessarily all of them, as shown in
  Figure<nbsp><reference|using-union-types>:

  <big-figure|<\verbatim>
    type number \ \ \ \ \ \ \ \ \ \ -\<gtr\> X:(integer\|real)

    succ X:number \ \ \ \ \ \ \ \ -\<gtr\> X + 1

    pred X:(integer\|real) -\<gtr\> X-1
  </verbatim>|<label|using-union-types>Using union types>

  <subsubsection|Enumeration types>

  An <em|enumeration type> acceps names in a predefined set. The notation
  <verbatim|enumeration(A, B, C)> corresponds to an enumeration accepting the
  names <verbatim|A>, <verbatim|B>, <verbatim|C>... This notation is
  pre-defined in the standard library in a way subtantially equivalent to
  Figure<nbsp><reference|enum-type-definition>:

  <big-figure|<\verbatim>
    type enumeration(A:name,Rest) -\<gtr\>

    \ \ \ \ contains X:name -\<gtr\> name.value = A.value or
    enumeration(Rest) has X

    \ \ \ \ contains X -\<gtr\> false
  </verbatim>|<label|enum-type-definition>Enumeration type definition>

  Unlike in other languages, enumeration types are not distinct from one
  another and can overlap. For example, the name <verbatim|do> belongs to
  <verbatim|enumeration(do,undo,redo)> as well as to
  <verbatim|enumeration(do,re,mi,fa,sol,la,si)>.

  <subsubsection|A type definition matching type declarations>

  Type declarations in a type definition are used to declare actual types, so
  a type that matches type declarations cannot be defined by a simple
  pattern. Figure<nbsp><reference|type-declaration-type> shows how the
  standard library defines a <verbatim|type_declaration> type using a type
  check.

  <big-figure|<\verbatim>
    type type_declaration -\<gtr\>

    \ \ \ contains X:infix -\<gtr\> X.name = ":"

    \ \ \ contains X -\<gtr\> false
  </verbatim>|<label|type-declaration-type>Type matching a type declaration>

  <subsection|Type inference>

  <subsection|Built-in operations>

  <subsection|<label|C-library>Importing symbols from C libraries>

  \;

  \;

  <section|Example code>

  <subsection|Minimum and maximum>

  \;

  <subsection|Complex numbers>

  \;

  <subsection|Vector and Matrix computations>

  <subsection|Linked lists with dynamic allocation>

  \;

  <subsection|Input / Output>

  \;

  <subsection|<label|object-oriented-programming>Object-Oriented Programming>

  <subsubsection|Classes>

  <subsubsection|Methods>

  <subsubsection|Dynamic dispatch>

  <subsubsection|Polymorphism>

  <subsubsection|Inheritance>

  <subsubsection|Multi-methods>

  <subsubsection|Object prototypes>

  <subsection|Functional-Programming>

  <subsubsection|Map>

  <subsubsection|Reduce>

  <subsubsection|Filter>

  <subsubsection|Functions as first-class objects>

  <subsubsection|Anonymous functions (Lambda)>

  <\with|font-series|bold>
    <subsubsection|Y-Combinator>
  </with>

  \;

  <section|Implementation notes>

  This section describes the implementation as published at
  <verbatim|http://xlr.sourceforge.net>.

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

  \;

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

  <subsubsection|Right side of a rewrite>

  If the tree is on the right of a rewrite (i.e. the right of an infix
  <verbatim|-\<gtr\>> operator), then <verbatim|code> will take additional
  input trees as arguments. Specifically, there will be one additional
  parameter in the code per variable in the rewrite rule pattern.

  For example, if a rewrite is <verbatim|X+Y-\<gtr\>foo X,Y>, then the
  <verbatim|code> field for <verbatim|foo X,Y> will have <verbatim|X> as its
  second argument and <verbatim|Y> as its third argument, as shown in
  Figure<nbsp><reference|rewrite-code>.

  \;

  \;

  In that case, the input tree for the actual expression being rewritten
  remains passed as the first argument, generally denoted as <verbatim|self>.

  <subsubsection|Closures>

  If a tree is passed as a <verbatim|tree> argument to a function, then it is
  encapsulated in a <em|closure>. The intent is to capture the environment
  that the passed tree depends on. Therefore, the associated <verbatim|code>
  will take additional arguments representing all the captured values. For
  instance, a closure for <verbatim|write X,Y> that captures variables
  <verbatim|X> and <verbatim|Y> will have the signature shown in
  Figure<nbsp><reference|closure-code>:

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

  <subsection|Machine Interface><label|machine-interface>

  <subsection|Machine Types and Normal Types>

  \;
</body>

<\initial>
  <\collection>
    <associate|par-hyphen|normal>
  </collection>
</initial>

<\references>
  <\collection>
    <associate|Binding|<tuple|3.5|?>>
    <associate|C-library|<tuple|4.5|26>>
    <associate|C-syntax-file|<tuple|12|9>>
    <associate|arbitrary-type|<tuple|41|21>>
    <associate|arithmetic|<tuple|1|22>>
    <associate|array-assign|<tuple|22|?>>
    <associate|assign-to-new-local|<tuple|21|?>>
    <associate|assignment|<tuple|3.1.4|?>>
    <associate|assignments-cant-override-patterns|<tuple|22|?>>
    <associate|auto-1|<tuple|1|1>>
    <associate|auto-10|<tuple|4|3>>
    <associate|auto-100|<tuple|4.1|23>>
    <associate|auto-101|<tuple|4.1.1|23>>
    <associate|auto-102|<tuple|1|23>>
    <associate|auto-103|<tuple|4.1.2|23>>
    <associate|auto-104|<tuple|2|23>>
    <associate|auto-105|<tuple|4.1.3|23>>
    <associate|auto-106|<tuple|3|23>>
    <associate|auto-107|<tuple|4.1.4|24>>
    <associate|auto-108|<tuple|4|24>>
    <associate|auto-109|<tuple|4.1.5|24>>
    <associate|auto-11|<tuple|2.2|3>>
    <associate|auto-110|<tuple|5|24>>
    <associate|auto-111|<tuple|4.1.6|24>>
    <associate|auto-112|<tuple|6|24>>
    <associate|auto-113|<tuple|4.1.7|24>>
    <associate|auto-114|<tuple|7|25>>
    <associate|auto-115|<tuple|4.1.8|25>>
    <associate|auto-116|<tuple|8|25>>
    <associate|auto-117|<tuple|4.1.9|25>>
    <associate|auto-118|<tuple|9|25>>
    <associate|auto-119|<tuple|4.1.10|26>>
    <associate|auto-12|<tuple|5|3>>
    <associate|auto-120|<tuple|10|26>>
    <associate|auto-121|<tuple|4.2|26>>
    <associate|auto-122|<tuple|4.2.1|26>>
    <associate|auto-123|<tuple|48|26>>
    <associate|auto-124|<tuple|49|26>>
    <associate|auto-125|<tuple|4.2.2|26>>
    <associate|auto-126|<tuple|50|26>>
    <associate|auto-127|<tuple|4.2.3|26>>
    <associate|auto-128|<tuple|51|26>>
    <associate|auto-129|<tuple|4.3|26>>
    <associate|auto-13|<tuple|2.3|3>>
    <associate|auto-130|<tuple|4.4|26>>
    <associate|auto-131|<tuple|4.5|26>>
    <associate|auto-132|<tuple|5|27>>
    <associate|auto-133|<tuple|5.1|27>>
    <associate|auto-134|<tuple|5.2|27>>
    <associate|auto-135|<tuple|5.3|27>>
    <associate|auto-136|<tuple|5.4|27>>
    <associate|auto-137|<tuple|5.5|27>>
    <associate|auto-138|<tuple|5.6|27>>
    <associate|auto-139|<tuple|5.6.1|27>>
    <associate|auto-14|<tuple|2.3.1|3>>
    <associate|auto-140|<tuple|5.6.2|27>>
    <associate|auto-141|<tuple|5.6.3|27>>
    <associate|auto-142|<tuple|5.6.4|27>>
    <associate|auto-143|<tuple|5.6.5|27>>
    <associate|auto-144|<tuple|5.6.6|27>>
    <associate|auto-145|<tuple|5.6.7|27>>
    <associate|auto-146|<tuple|5.7|27>>
    <associate|auto-147|<tuple|5.7.1|27>>
    <associate|auto-148|<tuple|5.7.2|27>>
    <associate|auto-149|<tuple|5.7.3|27>>
    <associate|auto-15|<tuple|6|4>>
    <associate|auto-150|<tuple|5.7.4|27>>
    <associate|auto-151|<tuple|5.7.5|27>>
    <associate|auto-152|<tuple|5.7.6|27>>
    <associate|auto-153|<tuple|6|28>>
    <associate|auto-154|<tuple|6.1|28>>
    <associate|auto-155|<tuple|52|28>>
    <associate|auto-156|<tuple|6.2|28>>
    <associate|auto-157|<tuple|53|28>>
    <associate|auto-158|<tuple|11|29>>
    <associate|auto-159|<tuple|6.3|29>>
    <associate|auto-16|<tuple|2.3.2|4>>
    <associate|auto-160|<tuple|6.4|29>>
    <associate|auto-161|<tuple|6.5|29>>
    <associate|auto-162|<tuple|6.6|29>>
    <associate|auto-163|<tuple|6.7|29>>
    <associate|auto-164|<tuple|6.8|29>>
    <associate|auto-165|<tuple|6.9|30>>
    <associate|auto-166|<tuple|54|30>>
    <associate|auto-167|<tuple|6.9.1|30>>
    <associate|auto-168|<tuple|6.9.2|30>>
    <associate|auto-169|<tuple|55|?>>
    <associate|auto-17|<tuple|7|4>>
    <associate|auto-170|<tuple|6.10|?>>
    <associate|auto-171|<tuple|6.11|?>>
    <associate|auto-172|<tuple|6.12|?>>
    <associate|auto-173|<tuple|6.13|?>>
    <associate|auto-174|<tuple|6.13|?>>
    <associate|auto-18|<tuple|2.3.3|4>>
    <associate|auto-19|<tuple|8|4>>
    <associate|auto-2|<tuple|1.1|1>>
    <associate|auto-20|<tuple|9|5>>
    <associate|auto-21|<tuple|2.3.4|5>>
    <associate|auto-22|<tuple|10|5>>
    <associate|auto-23|<tuple|2.4|5>>
    <associate|auto-24|<tuple|2.4.1|5>>
    <associate|auto-25|<tuple|2.4.2|6>>
    <associate|auto-26|<tuple|2.4.3|6>>
    <associate|auto-27|<tuple|2.5|6>>
    <associate|auto-28|<tuple|2.5.1|6>>
    <associate|auto-29|<tuple|2.5.2|6>>
    <associate|auto-3|<tuple|1.2|1>>
    <associate|auto-30|<tuple|2.5.3|7>>
    <associate|auto-31|<tuple|2.5.4|7>>
    <associate|auto-32|<tuple|2.6|7>>
    <associate|auto-33|<tuple|11|8>>
    <associate|auto-34|<tuple|12|9>>
    <associate|auto-35|<tuple|13|10>>
    <associate|auto-36|<tuple|3|10>>
    <associate|auto-37|<tuple|3.1|10>>
    <associate|auto-38|<tuple|3.1.1|10>>
    <associate|auto-39|<tuple|14|11>>
    <associate|auto-4|<tuple|1.3|2>>
    <associate|auto-40|<tuple|15|11>>
    <associate|auto-41|<tuple|16|11>>
    <associate|auto-42|<tuple|3.1.1.1|11>>
    <associate|auto-43|<tuple|3.1.2|11>>
    <associate|auto-44|<tuple|17|11>>
    <associate|auto-45|<tuple|18|12>>
    <associate|auto-46|<tuple|3.1.3|12>>
    <associate|auto-47|<tuple|19|12>>
    <associate|auto-48|<tuple|3.1.4|12>>
    <associate|auto-49|<tuple|20|12>>
    <associate|auto-5|<tuple|1|2>>
    <associate|auto-50|<tuple|21|12>>
    <associate|auto-51|<tuple|22|13>>
    <associate|auto-52|<tuple|3.1.5|13>>
    <associate|auto-53|<tuple|23|13>>
    <associate|auto-54|<tuple|3.1.6|13>>
    <associate|auto-55|<tuple|24|14>>
    <associate|auto-56|<tuple|3.1.7|14>>
    <associate|auto-57|<tuple|25|14>>
    <associate|auto-58|<tuple|3.2|14>>
    <associate|auto-59|<tuple|3.2.1|14>>
    <associate|auto-6|<tuple|2|2>>
    <associate|auto-60|<tuple|3.2.2|15>>
    <associate|auto-61|<tuple|3.2.3|15>>
    <associate|auto-62|<tuple|3.2.4|16>>
    <associate|auto-63|<tuple|3.3|16>>
    <associate|auto-64|<tuple|3.3.1|17>>
    <associate|auto-65|<tuple|3.3.2|17>>
    <associate|auto-66|<tuple|3.3.3|17>>
    <associate|auto-67|<tuple|26|18>>
    <associate|auto-68|<tuple|27|18>>
    <associate|auto-69|<tuple|28|18>>
    <associate|auto-7|<tuple|3|2>>
    <associate|auto-70|<tuple|3.4|18>>
    <associate|auto-71|<tuple|3.4.1|18>>
    <associate|auto-72|<tuple|3.4.2|19>>
    <associate|auto-73|<tuple|29|19>>
    <associate|auto-74|<tuple|30|19>>
    <associate|auto-75|<tuple|31|19>>
    <associate|auto-76|<tuple|32|19>>
    <associate|auto-77|<tuple|33|19>>
    <associate|auto-78|<tuple|3.4.3|20>>
    <associate|auto-79|<tuple|34|20>>
    <associate|auto-8|<tuple|2|2>>
    <associate|auto-80|<tuple|35|20>>
    <associate|auto-81|<tuple|3.4.4|20>>
    <associate|auto-82|<tuple|36|20>>
    <associate|auto-83|<tuple|37|20>>
    <associate|auto-84|<tuple|38|21>>
    <associate|auto-85|<tuple|39|21>>
    <associate|auto-86|<tuple|3.4.5|21>>
    <associate|auto-87|<tuple|40|21>>
    <associate|auto-88|<tuple|3.4.6|21>>
    <associate|auto-89|<tuple|41|21>>
    <associate|auto-9|<tuple|2.1|3>>
    <associate|auto-90|<tuple|42|21>>
    <associate|auto-91|<tuple|3.4.7|21>>
    <associate|auto-92|<tuple|43|21>>
    <associate|auto-93|<tuple|44|22>>
    <associate|auto-94|<tuple|3.4.8|22>>
    <associate|auto-95|<tuple|45|22>>
    <associate|auto-96|<tuple|46|22>>
    <associate|auto-97|<tuple|3.4.9|22>>
    <associate|auto-98|<tuple|47|22>>
    <associate|auto-99|<tuple|4|22>>
    <associate|automatic-type-conversion|<tuple|44|21>>
    <associate|binding|<tuple|3.2|14>>
    <associate|binding-for-complex-parameter|<tuple|32|19>>
    <associate|bitwise-arithmetic|<tuple|3|23>>
    <associate|block-type-declaration|<tuple|30|?>>
    <associate|boolean-operations|<tuple|4|23>>
    <associate|built-ins|<tuple|4.1|22>>
    <associate|class-like-data|<tuple|22|?>>
    <associate|closure-code|<tuple|55|29>>
    <associate|color-properties|<tuple|37|20>>
    <associate|comma-separated-list|<tuple|17|11>>
    <associate|comments|<tuple|5|3>>
    <associate|complex-normal-form|<tuple|35|19>>
    <associate|complex-type|<tuple|18|12>>
    <associate|contains-tests|<tuple|42|21>>
    <associate|conversions|<tuple|7|24>>
    <associate|data-inheritance|<tuple|40|20>>
    <associate|data-loading-operations|<tuple|10|25>>
    <associate|enum-type-definition|<tuple|50|26>>
    <associate|evaluation|<tuple|3.3|15>>
    <associate|evaluation-for-comparison|<tuple|26|17>>
    <associate|evaluation-for-type-comparison|<tuple|27|17>>
    <associate|explicit-and-automatic-type-conversions|<tuple|3.4.7|?>>
    <associate|explicit-type-conversion|<tuple|43|21>>
    <associate|explicit-vs-lazy-evaluation|<tuple|28|17>>
    <associate|extra-code-for-properties|<tuple|39|20>>
    <associate|factorial|<tuple|1|2>>
    <associate|footnote-1|<tuple|1|3>>
    <associate|footnote-2|<tuple|2|5>>
    <associate|footnote-3|<tuple|3|7>>
    <associate|footnote-4|<tuple|4|16>>
    <associate|footnote-5|<tuple|5|22>>
    <associate|footnote-6|<tuple|6|28>>
    <associate|footnote-7|<tuple|7|16>>
    <associate|footnr-1|<tuple|1|3>>
    <associate|footnr-2|<tuple|2|5>>
    <associate|footnr-3|<tuple|3|7>>
    <associate|footnr-4|<tuple|4|16>>
    <associate|footnr-5|<tuple|5|22>>
    <associate|footnr-6|<tuple|6|28>>
    <associate|footnr-7|<tuple|7|16>>
    <associate|guard|<tuple|23|12>>
    <associate|if-then|<tuple|3|2>>
    <associate|if-then-else|<tuple|14|11>>
    <associate|if-then-else-colorized|<tuple|15|11>>
    <associate|index-operators|<tuple|3.1.7|13>>
    <associate|infix-type|<tuple|46|21>>
    <associate|iterations|<tuple|3|?>>
    <associate|lazy-evaluation|<tuple|3.3.3|?>>
    <associate|literals|<tuple|2.3|3>>
    <associate|llvm-operations|<tuple|11|28>>
    <associate|local-and-nonlocal-assignment|<tuple|20|?>>
    <associate|long-text-indent|<tuple|9|5>>
    <associate|machine-interface|<tuple|6.12|30>>
    <associate|making-two-types-equivalent|<tuple|33|19>>
    <associate|map-reduce-filter|<tuple|2|2>>
    <associate|math-operations|<tuple|5|23>>
    <associate|mathematical-functions|<tuple|5|?>>
    <associate|more-specific-complex-types|<tuple|34|19>>
    <associate|object-oriented-programming|<tuple|5.6|27>>
    <associate|odd-type|<tuple|34|16>>
    <associate|off-side-rule|<tuple|4|3>>
    <associate|out-of-order-declarations|<tuple|16|11>>
    <associate|parameterized-type|<tuple|45|21>>
    <associate|person-properties|<tuple|26|?>>
    <associate|precedence|<tuple|2.6|7>>
    <associate|properties-declaration|<tuple|36|?>>
    <associate|references|<tuple|3.4|?>>
    <associate|return-type-declaration|<tuple|20|?>>
    <associate|rewrite-code|<tuple|54|29>>
    <associate|rewrite-type|<tuple|47|22>>
    <associate|sequence|<tuple|24|13>>
    <associate|setting-default-arguments|<tuple|38|20>>
    <associate|simple-type|<tuple|29|18>>
    <associate|simpleprog|<tuple|8|3>>
    <associate|simultaneously-type-and-data|<tuple|47|?>>
    <associate|source-syntax|<tuple|13|10>>
    <associate|standard-evaluation|<tuple|3.3.1|15>>
    <associate|structured-data|<tuple|25|13>>
    <associate|syntax-file|<tuple|11|8>>
    <associate|text-operations|<tuple|6|24>>
    <associate|time-operations|<tuple|8|24>>
    <associate|tree-operations|<tuple|9|24>>
    <associate|tree-rewrite-operators|<tuple|3.1|10>>
    <associate|type-conversion|<tuple|34|17>>
    <associate|type-conversions|<tuple|3.4.7|?>>
    <associate|type-declaration|<tuple|19|12>>
    <associate|type-declaration-type|<tuple|51|26>>
    <associate|type-definition|<tuple|3.4.2|?>>
    <associate|type-name|<tuple|34|17>>
    <associate|types|<tuple|3.4|18>>
    <associate|union-type-definition|<tuple|48|25>>
    <associate|using-automatic-type-conversion|<tuple|35|?>>
    <associate|using-complex|<tuple|31|18>>
    <associate|using-union-types|<tuple|49|26>>
    <associate|xlsyntax|<tuple|2.1|?>>
  </collection>
</references>

<\auxiliary>
  <\collection>
    <\associate|figure>
      <tuple|normal|<label|factorial>Declaration of the factorial
      function|<pageref|auto-5>>

      <tuple|normal|<label|map-reduce-filter>Map, reduce and
      filter|<pageref|auto-6>>

      <tuple|normal|<label|if-then>Declaration of
      if-then-else|<pageref|auto-7>>

      <tuple|normal|<label|off-side-rule>Off-side rule: Using indentation to
      mark program structure.|<pageref|auto-10>>

      <tuple|normal|<label|comments>Single-line and multi-line
      comments|<pageref|auto-12>>

      <tuple|normal|Valid integer constants|<pageref|auto-15>>

      <tuple|normal|Valid real constants|<pageref|auto-17>>

      <tuple|normal|Valid text constants|<pageref|auto-19>>

      <tuple|normal|<label|long-text-indent>Long text and
      indentation|<pageref|auto-20>>

      <tuple|normal|Examples of valid operator and name
      symbols|<pageref|auto-22>>

      <tuple|normal|<label|syntax-file>Default syntax configuration
      file|<pageref|auto-33>>

      <tuple|normal|<label|C-syntax-file>C syntax configuration
      file|<pageref|auto-34>>

      <tuple|normal|<label|source-syntax>Use of the
      <with|font-family|<quote|tt>|language|<quote|verbatim>|syntax>
      specification in a source file|<pageref|auto-35>>

      <tuple|normal|<label|if-then-else>Examples of tree
      rewrites|<pageref|auto-39>>

      <tuple|normal|<label|if-then-else-colorized>Examples of tree
      rewrites|<pageref|auto-40>>

      <tuple|normal|<label|out-of-order-declarations>Declarations are visible
      to the entire sequence containing them|<pageref|auto-41>>

      <tuple|normal|<label|comma-separated-list>Declaring a comma-separated
      list|<pageref|auto-44>>

      <tuple|normal|<label|complex-type>Declaring a
      <with|font-family|<quote|tt>|language|<quote|verbatim>|complex> data
      type|<pageref|auto-45>>

      <tuple|normal|<label|type-declaration>Simple type
      declarations|<pageref|auto-47>>

      <tuple|normal|<label|local-and-nonlocal-assignment>Local and non-local
      assignments|<pageref|auto-49>>

      <tuple|normal|<label|assign-to-new-local>Assigning to new local
      variable|<pageref|auto-50>>

      <tuple|normal|<label|assignments-cant-override-patterns>Assignments do
      not override patterns|<pageref|auto-51>>

      <tuple|normal|<label|guard>Guard limit the validity of
      operations|<pageref|auto-53>>

      <tuple|normal|<label|sequence>Code writing
      <with|font-family|<quote|tt>|language|<quote|verbatim>|A>, then
      <with|font-family|<quote|tt>|language|<quote|verbatim>|B>, then
      <with|font-family|<quote|tt>|language|<quote|verbatim>|f(100)+f(200)>|<pageref|auto-55>>

      <tuple|normal|<label|structured-data>Structured data|<pageref|auto-57>>

      <tuple|normal|<label|evaluation-for-comparison>Evaluation for
      comparison|<pageref|auto-67>>

      <tuple|normal|<label|evaluation-for-type-comparison>Evaluation for type
      comparison|<pageref|auto-68>>

      <tuple|normal|<label|explicit-vs-lazy-evaluation>Explicit vs. lazy
      evaluation|<pageref|auto-69>>

      <tuple|normal|<label|simple-type>Simple type
      declaration|<pageref|auto-73>>

      <tuple|normal|<label|block-type-declaration>Simple type
      declaration|<pageref|auto-74>>

      <tuple|normal|<label|using-complex>Using the
      <with|font-family|<quote|tt>|language|<quote|verbatim>|complex>
      type|<pageref|auto-75>>

      <tuple|normal|<label|binding-for-complex-parameter>Binding for a
      <with|font-family|<quote|tt>|language|<quote|verbatim>|complex>
      parameter|<pageref|auto-76>>

      <tuple|normal|<label|making-two-types-equivalent>Making type
      <with|font-family|<quote|tt>|language|<quote|verbatim>|A> equivalent to
      type <with|font-family|<quote|tt>|language|<quote|verbatim>|B>|<pageref|auto-77>>

      <tuple|normal|<label|more-specific-complex-types>Named patterns for
      <with|font-family|<quote|tt>|language|<quote|verbatim>|complex>|<pageref|auto-79>>

      <tuple|normal|<label|complex-normal-form>Creating a normal form for the
      complex type|<pageref|auto-80>>

      <tuple|normal|<label|properties-declaration>Properties
      declaration|<pageref|auto-82>>

      <tuple|normal|<label|color-properties>Color
      properties|<pageref|auto-83>>

      <tuple|normal|<label|setting-default-arguments>Setting default
      arguments from the current context|<pageref|auto-84>>

      <tuple|normal|<label|extra-code-for-properties>Additional code in
      properties|<pageref|auto-85>>

      <tuple|normal|<label|data-inheritance>Data
      inheritance|<pageref|auto-87>>

      <tuple|normal|<label|arbitrary-type>Defining a type identifying an
      arbitrary AST shape|<pageref|auto-89>>

      <tuple|normal|<label|contains-tests>Explicit type
      check|<pageref|auto-90>>

      <tuple|normal|<label|explicit-type-conversion>Explicit type
      conversion|<pageref|auto-92>>

      <tuple|normal|<label|automatic-type-conversion>Automatic type
      conversion|<pageref|auto-93>>

      <tuple|normal|<label|parameterized-type>Parameterized
      type|<pageref|auto-95>>

      <tuple|normal|<label|infix-type>Declaring a range type using an infix
      form|<pageref|auto-96>>

      <tuple|normal||<pageref|auto-97>>

      <tuple|normal|<label|rewrite-type>Declaration of a rewrite
      type|<pageref|auto-99>>

      <tuple|normal|<label|union-type-definition>Union type
      definition|<pageref|auto-124>>

      <tuple|normal|<label|using-union-types>Using union
      types|<pageref|auto-125>>

      <tuple|normal|<label|enum-type-definition>Enumeration type
      definition|<pageref|auto-127>>

      <tuple|normal|<label|type-declaration-type>Type matching a type
      declaration|<pageref|auto-129>>

      <tuple|normal|Lazy evaluation of an infinite list|<pageref|auto-156>>

      <tuple|normal|Controlled compilation|<pageref|auto-158>>

      <tuple|normal|<label|rewrite-code>Signature for rewrite code with two
      variables.|<pageref|auto-167>>

      <tuple|normal|<label|closure-code>Signature for rewrite code with two
      variables.|<pageref|auto-170>>
    </associate>
    <\associate|table>
      <tuple|normal|<label|arithmetic>Arithmetic
      operations|<pageref|auto-103>>

      <tuple|normal|<label|arithmetic>Comparisons|<pageref|auto-105>>

      <tuple|normal|<label|bitwise-arithmetic>Bitwise arithmetic
      operations|<pageref|auto-107>>

      <tuple|normal|<label|boolean-operations>Boolean
      operations|<pageref|auto-109>>

      <tuple|normal|<label|math-operations>Mathematical
      operations|<pageref|auto-111>>

      <tuple|normal|<label|text-operations>Text
      operations|<pageref|auto-113>>

      <tuple|normal|<label|conversions>Conversions|<pageref|auto-115>>

      <tuple|normal|<label|time-operations>Date and time|<pageref|auto-117>>

      <tuple|normal|<label|tree-operations>Tree
      operations|<pageref|auto-119>>

      <tuple|normal|<label|data-loading-operations>Data loading
      operations|<pageref|auto-121>>

      <tuple|normal|<label|llvm-operations>LLVM
      operations|<pageref|auto-159>>
    </associate>
    <\associate|toc>
      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|1<space|2spc>Introduction>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-1><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|1.1<space|2spc>Design objectives
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-2>>

      <with|par-left|<quote|1.5fn>|1.2<space|2spc>Keeping the syntax simple.
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-3>>

      <with|par-left|<quote|1.5fn>|1.3<space|2spc>Examples
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-4>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|2<space|2spc>Syntax>
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-8><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|2.1<space|2spc>Spaces and indentation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-9>>

      <with|par-left|<quote|1.5fn>|2.2<space|2spc>Comments and spaces
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-11>>

      <with|par-left|<quote|1.5fn>|2.3<space|2spc><label|literals>Literals
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-13>>

      <with|par-left|<quote|3fn>|2.3.1<space|2spc>Integer constants
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-14>>

      <with|par-left|<quote|3fn>|2.3.2<space|2spc>Real constants
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-16>>

      <with|par-left|<quote|3fn>|2.3.3<space|2spc>Text literals
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-18>>

      <with|par-left|<quote|3fn>|2.3.4<space|2spc>Name and operator symbols
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-21>>

      <with|par-left|<quote|1.5fn>|2.4<space|2spc>Structured nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-23>>

      <with|par-left|<quote|3fn>|2.4.1<space|2spc>Infix nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-24>>

      <with|par-left|<quote|3fn>|2.4.2<space|2spc>Prefix and postfix nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-25>>

      <with|par-left|<quote|3fn>|2.4.3<space|2spc>Block nodes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-26>>

      <with|par-left|<quote|1.5fn>|2.5<space|2spc>Parsing rules
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-27>>

      <with|par-left|<quote|3fn>|2.5.1<space|2spc>Precedence
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-28>>

      <with|par-left|<quote|3fn>|2.5.2<space|2spc>Associativity
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-29>>

      <with|par-left|<quote|3fn>|2.5.3<space|2spc>Infix versus Prefix versus
      Postfix <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-30>>

      <with|par-left|<quote|3fn>|2.5.4<space|2spc>Expression versus statement
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-31>>

      <with|par-left|<quote|1.5fn>|2.6<space|2spc><label|precedence>Syntax
      configuration <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-32>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|3<space|2spc>Language
      semantics> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-36><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|3.1<space|2spc><label|tree-rewrite-operators>Tree
      rewrite operators <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-37>>

      <with|par-left|<quote|3fn>|3.1.1<space|2spc>Rewrite declarations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-38>>

      <with|par-left|<quote|6fn>|Machine interface
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-42><vspace|0.15fn>>

      <with|par-left|<quote|3fn>|3.1.2<space|2spc>Data declaration
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-43>>

      <with|par-left|<quote|3fn>|3.1.3<space|2spc>Type declaration
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-46>>

      <with|par-left|<quote|3fn>|3.1.4<space|2spc>Assignment
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-48>>

      <with|par-left|<quote|3fn>|3.1.5<space|2spc>Guards
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-52>>

      <with|par-left|<quote|3fn>|3.1.6<space|2spc>Sequences
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-54>>

      <with|par-left|<quote|3fn>|3.1.7<space|2spc>Index operators
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-56>>

      <with|par-left|<quote|1.5fn>|3.2<space|2spc><label|binding>Binding
      References to Values <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-58>>

      <with|par-left|<quote|3fn>|3.2.1<space|2spc>Context Order
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-59>>

      <with|par-left|<quote|3fn>|3.2.2<space|2spc>Scoping
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-60>>

      <with|par-left|<quote|3fn>|3.2.3<space|2spc>Current context
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-61>>

      <with|par-left|<quote|3fn>|3.2.4<space|2spc>References
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-62>>

      <with|par-left|<quote|1.5fn>|3.3<space|2spc><label|evaluation>Evaluation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-63>>

      <with|par-left|<quote|3fn>|3.3.1<space|2spc>Standard evaluation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-64>>

      <with|par-left|<quote|3fn>|3.3.2<space|2spc>Special forms
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-65>>

      <with|par-left|<quote|3fn>|3.3.3<space|2spc>Lazy evaluation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-66>>

      <with|par-left|<quote|1.5fn>|3.4<space|2spc><label|types>Types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-70>>

      <with|par-left|<quote|3fn>|3.4.1<space|2spc>Predefined types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-71>>

      <with|par-left|<quote|3fn>|3.4.2<space|2spc>Type definition
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-72>>

      <with|par-left|<quote|3fn>|3.4.3<space|2spc>Normal form for a type
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-78>>

      <with|par-left|<quote|3fn>|3.4.4<space|2spc>Properties
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-81>>

      <with|par-left|<quote|3fn>|3.4.5<space|2spc>Data inheritance
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-86>>

      <with|par-left|<quote|3fn>|3.4.6<space|2spc>Explicit type check
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-88>>

      <with|par-left|<quote|3fn>|3.4.7<space|2spc>Explicit and automatic type
      conversions <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-91>>

      <with|par-left|<quote|3fn>|3.4.8<space|2spc>Parameterized types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-94>>

      <with|par-left|<quote|3fn>|3.4.9<space|2spc>Rewrite types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-98>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|4<space|2spc>Standard
      XL library> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-100><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|4.1<space|2spc><label|built-ins>Built-in
      operations <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-101>>

      <with|par-left|<quote|3fn>|4.1.1<space|2spc>Arithmetic
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-102>>

      <with|par-left|<quote|3fn>|4.1.2<space|2spc>Comparison
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-104>>

      <with|par-left|<quote|3fn>|4.1.3<space|2spc>Bitwise arithmetic
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-106>>

      <with|par-left|<quote|3fn>|4.1.4<space|2spc>Boolean operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-108>>

      <with|par-left|<quote|3fn>|4.1.5<space|2spc>Mathematical functions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-110>>

      <with|par-left|<quote|3fn>|4.1.6<space|2spc>Text functions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-112>>

      <with|par-left|<quote|3fn>|4.1.7<space|2spc>Conversions
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-114>>

      <with|par-left|<quote|3fn>|4.1.8<space|2spc>Date and time
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-116>>

      <with|par-left|<quote|3fn>|4.1.9<space|2spc>Tree operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-118>>

      <with|par-left|<quote|3fn>|4.1.10<space|2spc>Data loading operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-120>>

      <with|par-left|<quote|1.5fn>|4.2<space|2spc>Library-defined types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-122>>

      <with|par-left|<quote|3fn>|4.2.1<space|2spc>Union types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-123>>

      <with|par-left|<quote|3fn>|4.2.2<space|2spc>Enumeration types
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-126>>

      <with|par-left|<quote|3fn>|4.2.3<space|2spc>A type definition matching
      type declarations <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-128>>

      <with|par-left|<quote|1.5fn>|4.3<space|2spc>Type inference
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-130>>

      <with|par-left|<quote|1.5fn>|4.4<space|2spc>Built-in operations
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-131>>

      <with|par-left|<quote|1.5fn>|4.5<space|2spc><label|C-library>Importing
      symbols from C libraries <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-132>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|5<space|2spc>Example
      code> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-133><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|5.1<space|2spc>Minimum and maximum
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-134>>

      <with|par-left|<quote|1.5fn>|5.2<space|2spc>Complex numbers
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-135>>

      <with|par-left|<quote|1.5fn>|5.3<space|2spc>Vector and Matrix
      computations <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-136>>

      <with|par-left|<quote|1.5fn>|5.4<space|2spc>Linked lists with dynamic
      allocation <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-137>>

      <with|par-left|<quote|1.5fn>|5.5<space|2spc>Input / Output
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-138>>

      <with|par-left|<quote|1.5fn>|5.6<space|2spc><label|object-oriented-programming>Object-Oriented
      Programming <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-139>>

      <with|par-left|<quote|3fn>|5.6.1<space|2spc>Classes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-140>>

      <with|par-left|<quote|3fn>|5.6.2<space|2spc>Methods
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-141>>

      <with|par-left|<quote|3fn>|5.6.3<space|2spc>Dynamic dispatch
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-142>>

      <with|par-left|<quote|3fn>|5.6.4<space|2spc>Polymorphism
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-143>>

      <with|par-left|<quote|3fn>|5.6.5<space|2spc>Inheritance
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-144>>

      <with|par-left|<quote|3fn>|5.6.6<space|2spc>Multi-methods
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-145>>

      <with|par-left|<quote|3fn>|5.6.7<space|2spc>Object prototypes
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-146>>

      <with|par-left|<quote|1.5fn>|5.7<space|2spc>Functional-Programming
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-147>>

      <with|par-left|<quote|3fn>|5.7.1<space|2spc>Map
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-148>>

      <with|par-left|<quote|3fn>|5.7.2<space|2spc>Reduce
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-149>>

      <with|par-left|<quote|3fn>|5.7.3<space|2spc>Filter
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-150>>

      <with|par-left|<quote|3fn>|5.7.4<space|2spc>Functions as first-class
      objects <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-151>>

      <with|par-left|<quote|3fn>|5.7.5<space|2spc>Anonymous functions
      (Lambda) <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-152>>

      <with|par-left|<quote|3fn>|5.7.6<space|2spc>Y-Combinator
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-153>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|6<space|2spc>Implementation
      notes> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-154><vspace|0.5fn>

      <with|par-left|<quote|1.5fn>|6.1<space|2spc>Lazy evaluation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-155>>

      <with|par-left|<quote|1.5fn>|6.2<space|2spc>Controlled compilation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-157>>

      <with|par-left|<quote|1.5fn>|6.3<space|2spc>Tree representation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-160>>

      <with|par-left|<quote|1.5fn>|6.4<space|2spc>Evaluation of trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-161>>

      <with|par-left|<quote|1.5fn>|6.5<space|2spc>Tree position
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-162>>

      <with|par-left|<quote|1.5fn>|6.6<space|2spc>Actions on trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-163>>

      <with|par-left|<quote|1.5fn>|6.7<space|2spc>Symbols
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-164>>

      <with|par-left|<quote|1.5fn>|6.8<space|2spc>Evaluating trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-165>>

      <with|par-left|<quote|1.5fn>|6.9<space|2spc>Code generation for trees
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-166>>

      <with|par-left|<quote|3fn>|6.9.1<space|2spc>Right side of a rewrite
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-168>>

      <with|par-left|<quote|3fn>|6.9.2<space|2spc>Closures
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-169>>

      <with|par-left|<quote|1.5fn>|6.10<space|2spc>Tail recursion
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-171>>

      <with|par-left|<quote|1.5fn>|6.11<space|2spc>Partial recompilation
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-172>>

      <with|par-left|<quote|1.5fn>|6.12<space|2spc>Machine Interface
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-173>>

      <with|par-left|<quote|1.5fn>|6.13<space|2spc>Machine Types and Normal
      Types <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-174>>
    </associate>
  </collection>
</auxiliary>