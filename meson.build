project('cocot', 'c',
  version : '1.2-20171118',
  license : 'BSD-3-Clause',
  meson_version : '>= 0.60.0',
  default_options : ['warning_level=1'])

cc = meson.get_compiler('c')
cdata = configuration_data()

cdata.set_quoted('PACKAGE_NAME', meson.project_name())
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())

# Enable platform extensions (like AC_USE_SYSTEM_EXTENSIONS)
foreach m : ['_GNU_SOURCE', '_DARWIN_C_SOURCE', '_NETBSD_SOURCE',
             '_OPENBSD_SOURCE', '_ALL_SOURCE', '__EXTENSIONS__']
  cdata.set(m, 1)
endforeach

cdata.set_quoted('DEFAULT_TERM_CODE', get_option('default_term_code'))
cdata.set_quoted('DEFAULT_PROC_CODE', get_option('default_proc_code'))

# iconv (in libc or libiconv)
iconv_dep = dependency('iconv')

# openpty() and login_tty() are in libc or libutil depending on the platform
util_dep = cc.find_library('util', required : false)
pty_deps = []
foreach f : ['openpty', 'login_tty']
  if cc.has_function(f)
    cdata.set('HAVE_' + f.to_upper(), 1)
  elif util_dep.found() and cc.has_function(f, dependencies : util_dep)
    cdata.set('HAVE_' + f.to_upper(), 1)
    pty_deps = [util_dep]
  endif
endforeach

# Headers declaring openpty()/login_tty() vary between platforms
foreach h : ['utmp.h', 'libutil.h', 'util.h', 'pty.h']
  if cc.has_header(h)
    cdata.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# STREAMS modules must be pushed on the slave pty (e.g. Solaris)
if not cdata.has('HAVE_OPENPTY') and cc.has_header('stropts.h')
  cdata.set('HAVE_STROPTS_H', 1)
endif

if cc.has_function('cfmakeraw', prefix : '#include <termios.h>')
  cdata.set('HAVE_CFMAKERAW', 1)
endif

configure_file(output : 'config.h', configuration : cdata)

conv_lib = static_library('cocotconv',
  'ccio.c',
  'l10n_cjk_uni.c',
  'l10n_cjk_uni_table.c',
  'l10n_ja.c',
  # link iconv only once, through conv_dep
  dependencies : iconv_dep.partial_dependency(compile_args : true, includes : true))

conv_dep = declare_dependency(link_with : conv_lib, dependencies : iconv_dep)

executable('cocot',
  'cocot.c',
  'init.c',
  'loop.c',
  'sigwinch.c',
  'suspend.c',
  dependencies : [conv_dep, pty_deps],
  install : true)

subdir('tests')
