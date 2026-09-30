/*
 * Tests for the code conversion layer
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ccio.h"
#include "l10n_ja.h"
#include "l10n_cjk_uni.h"

static int failures;

#define CHECK(cond) \
    do { \
	if (!(cond)) { \
	    fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
	    failures++; \
	} \
    } while (0)

#define MEMEQ(p, lit) (memcmp((p), (lit), sizeof(lit) - 1) == 0)

/* Put IN into the CCIO buffer, run ccio_write() and capture its output. */
static size_t
run_ccio_write(CCIO *c, const char *in, size_t len, char *out, size_t outlen)
{
    int p[2];
    ssize_t n;

    if (pipe(p) < 0) {
	perror("pipe");
	failures++;
	return 0;
    }
    memcpy(c->buf, in, len);
    c->len = len;
    CHECK(ccio_write(c, p[1]) == CCIO_SUCCESS);
    close(p[1]);
    n = read(p[0], out, outlen);
    close(p[0]);
    return n < 0 ? 0 : (size_t) n;
}

static void
test_ccio_iconv(void)
{
    CCIO c;
    char out[256];
    size_t n;

    CHECK(ccio_init(&c, "EUC-JP", "UTF-8", 0) == CCIO_SUCCESS);

    n = run_ccio_write(&c, "\xe3\x81\x82", 3, out, sizeof(out));
    CHECK(n == 2 && MEMEQ(out, "\xa4\xa2"));
    CHECK(c.len == 0);

    /* an incomplete sequence is kept for the next read */
    n = run_ccio_write(&c, "a\xe3\x81", 3, out, sizeof(out));
    CHECK(n == 1 && out[0] == 'a');
    CHECK(c.len == 2 && MEMEQ(c.buf, "\xe3\x81"));

    /* an invalid byte at the end is replaced and consumed */
    n = run_ccio_write(&c, "abc\xff", 4, out, sizeof(out));
    CHECK(n == 4 && MEMEQ(out, "abc#"));
    CHECK(c.len == 0);

    ccio_done(&c);
}

static void
test_ccio_noconv(void)
{
    CCIO c;
    char out[256];
    size_t n;

    CHECK(ccio_init(&c, NULL, NULL, 0) == CCIO_SUCCESS);
    n = run_ccio_write(&c, "\xff\xfe", 2, out, sizeof(out));
    CHECK(n == 2 && MEMEQ(out, "\xff\xfe"));
    CHECK(c.len == 0);
    ccio_done(&c);
}

typedef size_t (*ja_conv)(void *, const char **, size_t *, char **, size_t *);

/* Convert IN with FN; returns the converter result and stores the output. */
static size_t
run_ja(ja_conv fn, int dec_jis, const char *in, size_t len,
       char *out, size_t *outlen, size_t *rest)
{
    void *ja = l10n_ja_open(dec_jis);
    const char *sp = in;
    char *dp = out;
    size_t dc = *outlen;
    size_t rv;

    rv = fn(ja, &sp, &len, &dp, &dc);
    *outlen = (size_t) (dp - out);
    *rest = len;
    l10n_ja_close(ja);
    return rv;
}

static void
test_l10n_ja(void)
{
    char out[256];
    size_t n, rest, rv;

    /* EUC-JP -> Shift_JIS */
    n = sizeof(out);
    rv = run_ja(l10n_ja_eucj2sjis, 0, "a\xa4\xa2", 3, out, &n, &rest);
    CHECK(rv == 0 && n == 3 && MEMEQ(out, "a\x82\xa0"));

    /* JIS X 0212 is replaced and all three bytes are consumed */
    n = sizeof(out);
    rv = run_ja(l10n_ja_eucj2sjis, 0, "\x8f\xb0\xa1" "A", 4, out, &n, &rest);
    CHECK(rv == 1 && n == 3 && MEMEQ(out, "\x81\xa0" "A"));

    /* half-width katakana */
    n = sizeof(out);
    rv = run_ja(l10n_ja_eucj2sjis, 0, "\x8e\xb1", 2, out, &n, &rest);
    CHECK(rv == 0 && n == 1 && MEMEQ(out, "\xb1"));

    /* incomplete input */
    n = sizeof(out);
    rv = run_ja(l10n_ja_eucj2sjis, 0, "a\xa4", 2, out, &n, &rest);
    CHECK(rv == (size_t) -1 && n == 1 && rest == 1);

    /* ISO-2022-JP escape sequences are decoded when enabled */
    n = sizeof(out);
    rv = run_ja(l10n_ja_eucj2sjis, 1, "\x1b$B$\"\x1b(B", 8, out, &n, &rest);
    CHECK(rv == 0 && n == 2 && MEMEQ(out, "\x82\xa0"));

    /* Shift_JIS -> EUC-JP */
    n = sizeof(out);
    rv = run_ja(l10n_ja_sjis2eucj, 0, "a\x82\xa0\xb1", 4, out, &n, &rest);
    CHECK(rv == 0 && n == 5 && MEMEQ(out, "a\xa4\xa2\x8e\xb1"));

    /* user-defined characters are replaced */
    n = sizeof(out);
    rv = run_ja(l10n_ja_sjis2eucj, 0, "\xf0\x40", 2, out, &n, &rest);
    CHECK(rv == 1 && n == 2 && MEMEQ(out, "\xa2\xa2"));
}

static int
skip_width(const char *s, size_t len, size_t *consumed)
{
    const char *p = s;
    size_t n = len;
    int w = l10n_cjk_uni_skip(&p, &n);
    *consumed = len - n;
    return w;
}

static void
test_l10n_cjk_uni(void)
{
    size_t c;

    CHECK(skip_width("A", 1, &c) == 1 && c == 1);
    CHECK(skip_width("\xef\xbc\x81", 3, &c) == 2 && c == 3);	/* U+FF01 */
    CHECK(skip_width("\xe3\x81\x82", 3, &c) == 2 && c == 3);	/* U+3042 */
    CHECK(skip_width("\xe3\x80\x9f", 3, &c) == 2 && c == 3);	/* U+301F (bit 31 of a table word) */
    CHECK(skip_width("\xf0\xa0\x80\x80", 4, &c) == 2 && c == 4);	/* U+20000 */
    CHECK(skip_width("\xe3\x81", 2, &c) == 1 && c == 1);	/* truncated */
}

int
main(void)
{
    test_ccio_iconv();
    test_ccio_noconv();
    test_l10n_ja();
    test_l10n_cjk_uni();
    if (failures)
	fprintf(stderr, "%d check(s) failed\n", failures);
    return failures ? 1 : 0;
}
