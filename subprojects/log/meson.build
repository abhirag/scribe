project('log', 'c')
i = include_directories('.')
l = static_library('log', 'log.c', include_directories : i, install : false)

log_dep = declare_dependency(include_directories : i,
  link_with : l)