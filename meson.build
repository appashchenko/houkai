project(
  'Houkai Explorer',
  'c',
  version: '0.1.0',
  license: 'MIT',
  meson_version: '>=0.54.0',
  default_options: [
    'buildtype=release',
    'c_std=c11',
    'warning_level=1',
    'werror=true',
    'b_ndebug=if-release',
    'strip=true'
  ]
)

subdir('src')
subdir('tests')

executable(
  'houkai',
  sources,
  dependencies: [
    dependency('gtk4')
  ],
  install: false
)
