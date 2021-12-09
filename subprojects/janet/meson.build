project('janet', 'c')
i = include_directories('.')
l = static_library('janet', 'janet.c', include_directories : i, install : false)

janet_dep = declare_dependency(include_directories : i,
  link_with : l)