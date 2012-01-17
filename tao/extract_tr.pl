#!/usr/bin/perl
#
# Read a .xl source and extract all text strings that need
# to be translated.
#
# Whenever this script encounters code like:
#   tr "some text"
# it will generate a translation template:
#   tr "some text", "some text"
# ...unless the string appears in the reference file.
#
# Usage:
# ./extrace_tr.pl [-ref <file>] file...
#
#  -ref <file>    The reference file. Only string NOT present
#                 in <file> are ouput.
#
# Example:
# ./extract_tr.pl -ref tao_fr.xl tao.xl
#   Will show all the definitions that need to be added to tao_fr.xl
#   and translated.

use Getopt::Long;

GetOptions('ref=s', => \$ref);

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
        while(/tr (\"[^"]+\"), \"[^"]+\"/g) {
            $ref_strings{$1} = 1
        }
    }
}

foreach (keys(%strings)) {
    print "    tr $_, $_\n" unless $ref_strings{$_};
}
