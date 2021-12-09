project('lmdb', 'c')
i = include_directories('.')
l = static_library('lmdb', ['mdb.c', 'midl.c'], include_directories : i, install : false)

lmdb_dep = declare_dependency(include_directories : i,
  link_with : l)