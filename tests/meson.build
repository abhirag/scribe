scribe_test = executable('scribe_test', 
                         [scribe_src, db_src, tracy_src, 'scribe_test.c'],
                         include_directories: inc,
                         dependencies: [sds, log, tree_sitter, lmdb, mkdirp])