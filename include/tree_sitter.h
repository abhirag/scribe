#ifndef SCRIBE_TREE_SITTER_H
#define SCRIBE_TREE_SITTER_H

#include <sds.h>
#include <tree_sitter/api.h>

TSParser* create_parser(TSLanguage* lang);
TSTree* parse_string(TSParser* parser, sds src);
sds* query_tree(sds src, TSLanguage* lang, TSTree* tree, sds query_string);
sds query_filter_tree(sds src, TSLanguage* lang, TSTree* tree, sds query_string,
                      char const* filter_string, int filter_index);

#endif  // SCRIBE_TREE_SITTER_H
