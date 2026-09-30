#ifndef L10N_CJK_UNI_TABLE_H
#define L10N_CJK_UNI_TABLE_H

#include <stddef.h>
#include <stdint.h>

struct cjk_width_range {
    uint32_t first;
    uint32_t last;
};

/* sorted, non-overlapping ranges of two-column code points */
extern const struct cjk_width_range cjk_wide_ranges[];
extern const size_t cjk_wide_ranges_count;

#endif /* L10N_CJK_UNI_TABLE_H */
