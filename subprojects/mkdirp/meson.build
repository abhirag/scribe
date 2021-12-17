project('mkdirp', 'c')
i = include_directories('.')
l = static_library('mkdirp', 'mkdirp.c', include_directories : i, install : false)

mkdirp_dep = declare_dependency(include_directories : i,
  link_with : l)