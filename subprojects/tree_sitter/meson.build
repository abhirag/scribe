project('tree_sitter', 'c')
i = include_directories('lib/src', 'lib/include')
l = static_library('tree_sitter', 'lib/src/lib.c', include_directories : i, install : false)

tree_sitter_dep = declare_dependency(include_directories : i,
  link_with : l)