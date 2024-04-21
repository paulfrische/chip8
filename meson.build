project('chip-8', 'c', 
  default_options : ['c_std=c2x'])

c = meson.get_compiler('c')
libdir = meson.current_source_dir() + './lib/'

incdir = include_directories('include')
raylib = c.find_library('raylib', dirs : libdir)
math = c.find_library('m', required : false)

executable('chip8', 'src/main.c', 'src/chip8.c', 'src/chip8_internal.c', 'src/util.c', 'src/data.c', 'src/raygui_impl.c', include_directories : incdir, dependencies : [raylib, math])
