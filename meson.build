project('cocot', 'c',
  version : '1.2-20171118',
  license : 'BSD-3-Clause',
  meson_version : '>= 0.60.0',
  default_options : ['warning_level=1'])

cc = meson.get_compiler('c')
cdata = configuration_data()

cdata.set_quoted('PACKAGE', meson.project_name())
cdata.set_quoted('PACKAGE_NAME', meson.project_name())
cdata.set_quoted('PACKAGE_TARNAME', meson.project_name())
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('PACKAGE_STRING', '@0@ @1@'.format(meson.project_name(), meson.project_version()))
cdata.set_quoted('VERSION', meson.project_version())

cdata.set_quoted('DEFAULT_TERM_CODE', get_option('default_term_code'))
cdata.set_quoted('DEFAULT_PROC_CODE', get_option('default_proc_code'))

deps = []

# iconv (libc built-in or libiconv)
iconv_dep = dependency('iconv')
deps += iconv_dep
if not cc.has_header('iconv.h', dependencies : iconv_dep)
  error('iconv.h is not available')
endif
cdata.set('HAVE_ICONV_H', 1)

# openpty() from libutil, or fall back to /dev/ptmx
util_dep = cc.find_library('util', required : false)
if util_dep.found() and cc.has_function('openpty', dependencies : util_dep)
  cdata.set('HAVE_LIBUTIL', 1)
  deps += util_dep
elif import('fs').exists('/dev/ptmx') and cc.has_header('stropts.h')
  cdata.set('HAVE_STROPTS_H', 1)
endif

foreach h : ['stdlib.h', 'string.h', 'sys/ioctl.h', 'sys/time.h', 'sys/types.h',
             'sys/wait.h', 'termios.h', 'unistd.h', 'utmp.h',
             'libutil.h', 'util.h', 'pty.h', 'signal.h']
  if cc.has_header(h)
    cdata.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

foreach f : ['memcpy', 'memmove', 'memset', 'select', 'strerror',
             'max', 'cfmakeraw', 'login_tty']
  if cc.has_function(f, dependencies : deps)
    cdata.set('HAVE_' + f.to_upper(), 1)
  endif
endforeach

# Assume a GNU-compatible malloc (malloc(0) returns a valid pointer)
cdata.set('HAVE_MALLOC', 1)

configure_file(output : 'config.h', configuration : cdata)

sources = files(
  'ccio.c',
  'cocot.c',
  'init.c',
  'l10n_cjk_uni.c',
  'l10n_cjk_uni_table.c',
  'l10n_ja.c',
  'loop.c',
  'malloc.c',
  'sigwinch.c',
  'suspend.c',
)

executable('cocot', sources,
  c_args : ['-DHAVE_CONFIG_H'],
  dependencies : deps,
  install : true)
