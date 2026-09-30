bin_PROGRAMS = cocot
cocot_SOURCES = cocot.c init.c init.h loop.c loop.h sigwinch.c sigwinch.h suspend.c suspend.h debug.h
cocot_LDADD = libcocotconv.a

# code conversion, shared with the tests
noinst_LIBRARIES = libcocotconv.a
libcocotconv_a_SOURCES = ccio.c ccio.h l10n_ja.c l10n_ja.h l10n_cjk_uni.c l10n_cjk_uni.h l10n_cjk_uni_table.c l10n_cjk_uni_table.h

check_PROGRAMS = tests/test_conv
tests_test_conv_SOURCES = tests/test_conv.c
tests_test_conv_LDADD = libcocotconv.a
TESTS = $(check_PROGRAMS)
# make UBSan reports fatal when built with -fsanitize=undefined
AM_TESTS_ENVIRONMENT = UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1; export UBSAN_OPTIONS;

EXTRA_DIST = ChangeLog.ja README.ja UNICODE_MEMO make_l10n_cjk_uni_table.sh make_l10n_cjk_uni_table.pl cygwin-build.sh cygwin-install.sh

# include a prebuilt binary (for Cygwin releases) when there is one
dist-hook:
	if test -f cocot.exe; then cp -p cocot.exe $(distdir)/; fi

mydist::
	make
	strip cocot.exe
	make dist-bzip2
	mv cocot-*.tar.bz2 ..
