project('chip-8', 'c')

c = meson.get_compiler('c')
libdir = meson.current_source_dir() + './lib/'

incdir = include_directories('include')
# raylib = declare_dependency(link_with : 'lib/libraylib.a')
raylib = c.find_library('raylib', dirs : libdir)

executable('chip8', 'src/main.c', include_directories : incdir, dependencies : raylib)
