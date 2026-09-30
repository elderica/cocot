/*
 * Localization for CJK Language with Unicode
 *
 * Copyright (c) 2004, 2005  IWAMURO Motonori
 * All rights reserved.
 */

#include <config.h>
#include <sys/types.h>
#include <iconv.h>
#include <errno.h>
#include "l10n_cjk_uni.h"
#include "l10n_cjk_uni_table.h"

static const unsigned char char_bytes[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1,
};

#define IS_NOT_TRAILING_BYTE(ch) (((ch) & 0xc0) != 0x80)

/* binary search in the table of two-column code points */
static int
is_wide(size_t ch)
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

int
l10n_cjk_uni_skip(const char **spp, size_t *scp)
{
    const unsigned char *sp;
    size_t cb;
    size_t ch;
    int w;

    sp = (const unsigned char *) *spp;
    cb = char_bytes[sp[0]];
    if (cb > *scp)
	goto error;
    switch (cb) {
    case 1:
	ch = sp[0];
	break;
    case 2:
	if (IS_NOT_TRAILING_BYTE(sp[1]))
	    goto error;
	ch  = ((sp[0] & 0x1f) <<  6)
	    |  (sp[1] & 0x3f);
	break;
    case 3:
	if (IS_NOT_TRAILING_BYTE(sp[1])
	    || IS_NOT_TRAILING_BYTE(sp[2]))
	    goto error;
	ch  = ((sp[0] & 0x0f) << 12)
	    | ((sp[1] & 0x3f) <<  6)
	    |  (sp[2] & 0x3f);
	break;
    case 4:
	if (IS_NOT_TRAILING_BYTE(sp[1])
	    || IS_NOT_TRAILING_BYTE(sp[2])
	    || IS_NOT_TRAILING_BYTE(sp[3]))
	    goto error;
	ch  = ((sp[0] & 0x07) << 18)
	    | ((sp[1] & 0x3f) << 12)
	    | ((sp[2] & 0x3f) <<  6)
	    |  (sp[3] & 0x3f);
	break;
    case 5:
	if (IS_NOT_TRAILING_BYTE(sp[1])
	    || IS_NOT_TRAILING_BYTE(sp[2])
	    || IS_NOT_TRAILING_BYTE(sp[3])
	    || IS_NOT_TRAILING_BYTE(sp[4]))
	    goto error;
	ch  = ((sp[0] & 0x03) << 24)
	    | ((sp[1] & 0x3f) << 18)
	    | ((sp[2] & 0x3f) << 12)
	    | ((sp[3] & 0x3f) <<  6)
	    |  (sp[4] & 0x3f);
	break;
    case 6:
	if (IS_NOT_TRAILING_BYTE(sp[1])
	    || IS_NOT_TRAILING_BYTE(sp[2])
	    || IS_NOT_TRAILING_BYTE(sp[3])
	    || IS_NOT_TRAILING_BYTE(sp[4])
	    || IS_NOT_TRAILING_BYTE(sp[5]))
	    goto error;
	ch  = ((sp[0] & 0x01) << 30)
	    | ((sp[1] & 0x3f) << 24)
	    | ((sp[2] & 0x3f) << 18)
	    | ((sp[3] & 0x3f) << 12)
	    | ((sp[4] & 0x3f) <<  6)
	    |  (sp[5] & 0x3f);
	break;
    default:
	goto error;
    }
    w = is_wide(ch) ? 2 : 1;
    *spp += cb;
    *scp -= cb;
    return w;

error:
    (*spp)++;
    (*scp)--;
    return 1;
}
