project('nur', 'c',
  version : '1.2.1',
  license : 'MIT',
  default_options : ['c_std=c11', 'warning_level=3'])

nur_inc = include_directories('src')

nur_dep = declare_dependency(
  include_directories : nur_inc,
  version : meson.project_version()
)

install_headers('src/nur.h')

pkg = import('pkgconfig')
pkg.generate(
  name : 'nur',
  description : 'Rust-style Result and Option error handling for ISO C',
  version : meson.project_version()
)

if get_option('tests')
  test_exe = executable('test_nur', 'test/test_nur.c',
    dependencies : nur_dep,
    c_args : ['-Wall', '-Wextra', '-Wpedantic']
  )
  test('unit_tests', test_exe)
endif
