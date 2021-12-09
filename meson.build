project('scribe', 'c', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 
                     'c_std=c11'])

add_project_arguments(['-DTRACY_ENABLE=1', '-DTRACY_NO_EXIT=1'], language : ['c', 'cpp'])
add_project_link_arguments(['-lm', '-lstdc++', '-ldl', '-lrt', '-lpthread'], language : ['c', 'cpp'])

sds_sp = subproject('sds')
sds = sds_sp.get_variable('sds_dep')

log_sp = subproject('log')
log = log_sp.get_variable('log_dep')

tree_sitter_sp = subproject('tree_sitter')
tree_sitter = tree_sitter_sp.get_variable('tree_sitter_dep')

janet_sp = subproject('janet')
janet = janet_sp.get_variable('janet_dep')

lmdb_sp = subproject('lmdb')
lmdb = lmdb_sp.get_variable('lmdb_dep')

inc = include_directories('include', 'dev_deps/tracy')
scribe_src = files('src/scribe.c')
tracy_src = files('dev_deps/tracy/TracyClient.cpp')

subdir('tests')

scribe = executable('scribe', 
                    [scribe_src, tracy_src, 'src/main.c'],
                    include_directories: inc,
                    dependencies: [sds, log, tree_sitter, lmdb])








