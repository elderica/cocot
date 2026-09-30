#ifndef L10N_CJK_UNI_TABLE_H
#define L10N_CJK_UNI_TABLE_H

#include <stdint.h>

#define CJK_WIDTH_LENGTH (0x10000 / 32)

extern const uint32_t cjk_width[CJK_WIDTH_LENGTH];

#endif /* L10N_CJK_UNI_TABLE_H */
