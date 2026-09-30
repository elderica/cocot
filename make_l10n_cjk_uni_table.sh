#!/bin/sh -x

UNICODE_VERSION=18.0.0

if [ ! -f EastAsianWidth.txt ] ||
   ! head -1 EastAsianWidth.txt | grep -q "EastAsianWidth-$UNICODE_VERSION.txt"; then
    curl -fsSL -o EastAsianWidth.txt \
	https://www.unicode.org/Public/$UNICODE_VERSION/ucd/EastAsianWidth.txt
fi

perl make_l10n_cjk_uni_table.pl EastAsianWidth.txt > l10n_cjk_uni_table.c
