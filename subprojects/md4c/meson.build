project('md4c', 'c')
i = include_directories('.')
l = static_library('md4c', 'md4c.c', include_directories : i, install : false)

md4c_dep = declare_dependency(include_directories : i,
  link_with : l)