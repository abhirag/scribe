project('sds', 'c')
i = include_directories('.')
l = static_library('sds', 'sds.c', include_directories : i, install : false)

sds_dep = declare_dependency(include_directories : i,
  link_with : l)