/*
 * Localization for CJK Language with Unicode
 *
 * Copyright (c) 2004, 2005  IWAMURO Motonori
 * All rights reserved.
 */

#include <config.h>
#include <stdint.h>
#include "l10n_cjk_uni.h"
#include "l10n_cjk_uni_table.h"

#define IS_NOT_TRAILING_BYTE(ch) (((ch) & 0xc0) != 0x80)

/* binary search in the table of two-column code points */
static int
is_wide(uint32_t ch)
{
    size_t lo = 0, hi = cjk_wide_ranges_count;

    while (lo < hi) {
	size_t mid = lo + (hi - lo) / 2;
	if (ch < cjk_wide_ranges[mid].first)
	    hi = mid;
	else if (ch > cjk_wide_ranges[mid].last)
	    lo = mid + 1;
	else
	    return 1;
    }
    return 0;
}

/*
 * Skip one character that could not be converted and return its width.
 * Invalid UTF-8 (RFC 3629) is skipped one byte at a time.
 */
int
l10n_cjk_uni_skip(const char **spp, size_t *scp)
{
    const unsigned char *sp = (const unsigned char *) *spp;
    size_t cb, i;
    uint32_t ch, min;

    if (sp[0] < 0x80) {
	cb = 1, ch = sp[0], min = 0;
    } else if (0xc2 <= sp[0] && sp[0] <= 0xdf) {
	cb = 2, ch = sp[0] & 0x1f, min = 0x80;
    } else if (0xe0 <= sp[0] && sp[0] <= 0xef) {
	cb = 3, ch = sp[0] & 0x0f, min = 0x800;
    } else if (0xf0 <= sp[0] && sp[0] <= 0xf4) {
	cb = 4, ch = sp[0] & 0x07, min = 0x10000;
    } else {
	goto error;
    }
    if (cb > *scp)
	goto error;
    for (i = 1; i < cb; i++) {
	if (IS_NOT_TRAILING_BYTE(sp[i]))
	    goto error;
	ch = (ch << 6) | (sp[i] & 0x3f);
    }
    /* overlong forms, surrogates and code points beyond U+10FFFF */
    if (ch < min || ch > 0x10ffff || (0xd800 <= ch && ch <= 0xdfff))
	goto error;
    *spp += cb;
    *scp -= cb;
    return is_wide(ch) ? 2 : 1;

error:
    (*spp)++;
    (*scp)--;
    return 1;
}
