#!/usr/bin/perl
#
# Generate l10n_cjk_uni_table.c from EastAsianWidth.txt.
#
# Code points whose East_Asian_Width is W (Wide), F (Fullwidth) or
# A (Ambiguous) occupy two columns on a CJK terminal. They are emitted
# as a sorted list of merged ranges; everything else is one column.

use strict;
use warnings;

my %wide = map { $_ => 1 } qw(W F A);

my $file = shift // 'EastAsianWidth.txt';
open(my $in, '<', $file) or die "$file: $!\n";

my $version;
my @ranges;
while (<$in>) {
    $version //= $1 if /^#\s*(EastAsianWidth-[\d.]+)\.txt/;
    next if /^\s*(#|$)/;
    my ($from, $to, $type) = /^([0-9A-F]+)(?:\.\.([0-9A-F]+))?\s*;\s*(\w+)/i
	or die "$file:$.: unexpected line: $_";
    next unless $wide{$type};
    push @ranges, [hex($from), hex($to // $from)];
}
close($in);
die "$file: no version header\n" unless defined $version;

my @merged;
for my $r (sort { $a->[0] <=> $b->[0] } @ranges) {
    if (@merged && $r->[0] <= $merged[-1][1] + 1) {
	$merged[-1][1] = $r->[1] if $r->[1] > $merged[-1][1];
    } else {
	push @merged, [@$r];
    }
}

print <<EOF;
/*
 * Two-column code points (East_Asian_Width W, F and A)
 *
 * Generated from $version.txt by make_l10n_cjk_uni_table.pl.
 * Do not edit.
 */

#include "l10n_cjk_uni_table.h"

const struct cjk_width_range cjk_wide_ranges[] = {
EOF
printf("    { 0x%06x, 0x%06x },\n", @$_) for @merged;
print <<EOF;
};

const size_t cjk_wide_ranges_count =
    sizeof(cjk_wide_ranges) / sizeof(cjk_wide_ranges[0]);
EOF
