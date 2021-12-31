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

mkdirp_sp = subproject('mkdirp')
mkdirp = mkdirp_sp.get_variable('mkdirp_dep')

md4c_sp = subproject('md4c')
md4c = md4c_sp.get_variable('md4c_dep')

inc = include_directories('include', 'dev_deps/tracy')
scribe_src = files('src/scribe.c')
db_src = files('src/db.c')
tracy_src = files('dev_deps/tracy/TracyClient.cpp')
c_parser_src = files('src/parsers/c_parser.c')
repl_src = files('src/repl.c')
lisp_src = files('src/lisp.c')
indexer_src = files('src/indexer.c')
core_queries_src = files('src/core_queries.c')
query_src = files('src/query.c')
tree_sitter_src = files('src/tree_sitter.c')
c_queries_src = files('src/c_queries.c')
substitute_src = files('src/substitute.c')

subdir('tests')

scribe = executable('scribe', 
                    [indexer_src, scribe_src, db_src, tracy_src, c_parser_src, repl_src, lisp_src,
                    core_queries_src, query_src, tree_sitter_src, c_queries_src, substitute_src, 'src/main.c'],
                    include_directories: inc,
                    dependencies: [sds, log, tree_sitter, lmdb, mkdirp, janet, md4c])

test_indexer = executable('test_indexer',
                          [indexer_src, tracy_src, lisp_src, db_src],
                          include_directories: inc,
                          dependencies: [sds, log, mkdirp, janet, lmdb],
                          c_args: ['-D UNIT_TEST_INDEXER'])

test_core_queries = executable('test_core_queries',
                               [core_queries_src, indexer_src, tracy_src, lisp_src, db_src, query_src, c_queries_src, tree_sitter_src, c_parser_src],
                               include_directories: inc,
                               dependencies: [sds, log, mkdirp, janet, lmdb, tree_sitter],
                               c_args: ['-D UNIT_TEST_CORE_QUERIES'])

test_tree_sitter = executable('test_tree_sitter',
                              [indexer_src, tracy_src, lisp_src, db_src, tree_sitter_src, c_parser_src, c_queries_src, query_src, core_queries_src],
                              include_directories: inc,
                              dependencies: [sds, log, mkdirp, janet, lmdb, tree_sitter],
                              c_args: ['-D UNIT_TEST_TREE_SITTER'])

test_substitute = executable('test_substitute',
                              [substitute_src, tracy_src, query_src, lisp_src, db_src, indexer_src, c_queries_src, core_queries_src, tree_sitter_src, c_parser_src],
                              include_directories: inc,
                              dependencies: [sds, log, md4c, janet, lmdb, mkdirp, tree_sitter],
                              c_args: ['-D UNIT_TEST_SUBSTITUTE'])








