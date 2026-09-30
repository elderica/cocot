test_conv = executable('test_conv', 'test_conv.c',
  include_directories : include_directories('..'),
  dependencies : conv_dep)

test('conv', test_conv,
  env : ['UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1'])
