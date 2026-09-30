test_conv = executable('test_conv', 'test_conv.c',
  c_args : ['-DHAVE_CONFIG_H'],
  include_directories : include_directories('..'),
  link_with : conv_lib,
  dependencies : deps)

test('conv', test_conv,
  env : ['UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1'])
