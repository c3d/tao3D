#!/usr/bin/perl
#
# Read a .xl source and extract all text strings that need
# to be translated.
#
# Whenever this script encounters code like:
#   tr "some text"
# it will generate a translation template:
#   tr "fr", "some text" -> "some text"
# ...unless the string appears in the reference file.
#
# Usage:
# ./extrace_tr.pl [-lang <code>] [-ref <file>] file...
#
#  -lang <code>   A 2-letter language code (fr, en...)
#  -ref <file>    The reference file. Only string NOT present
#                 in <file> are ouput.
#
# Example:
# ./extract_tr.pl -lang fr -ref tao_fr.xl tao.xl
#   Will show all the definitions that need to be added to tao_fr.xl
#   and translated.

use Getopt::Long;

my $lang="fr";
GetOptions('lang=s', => \$lang,
           'ref=s', => \$ref);

my %strings;
while (<>) {
    while (/tr (\"[^"]+\")/g) {
        $strings{$1} = 1;
    }
}

my %ref_strings = ();
if ($ref) {
    open REF, "<", $ref or die $!;
    while (<REF>) {
        while(/tr \"[^"]+\", (\"[^"]+\")/g) {
            $ref_strings{$1} = 1
        }
    }
}

foreach (keys(%strings)) {
    print "tr \"$lang\", $_ -> $_\n" unless $ref_strings{$_};
}
