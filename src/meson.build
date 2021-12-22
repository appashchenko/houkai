sources = files(
  'main-gtk.c',
  'akpk/read.c',
  'akpk/bkhd.c',
  'akpk/didx.c',
  'akpk/riff.c'
)

gtk3 = dependency('gtk+-3.0')

executable(
  'houkai',
  sources,
  dependencies: [ gtk3 ],
  install: false
)
