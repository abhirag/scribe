#include "tree_sitter.h"

#include <deps/stb_ds.h>
#include <sds.h>
#include <stdbool.h>
#include <tree_sitter/api.h>

#include "trace.h"

INIT_TRACE;

TSParser* create_parser(TSLanguage* lang) {
  START_ZONE;
  TSParser* parser = ts_parser_new();
  bool rc = ts_parser_set_language(parser, lang);
  if (!rc) {
    message_fatal("tree_sitter::create_parser failed in setting language");
    goto error_end;
  }
  END_ZONE;
  return parser;
error_end:
  ts_parser_delete(parser);
  END_ZONE;
  return (void*)0;
}

TSTree* parse_string(TSParser* parser, sds src) {
  START_ZONE;
  TSTree* tree = ts_parser_parse_string(parser, (void*)0, src, sdslen(src));
  if (!tree) {
    log_fatal("tree_sitter::parse_string failed in parsing src: %s", src);
    goto error_end;
  }
  END_ZONE;
  return tree;
error_end:
  ts_tree_delete(tree);
  END_ZONE;
  return (void*)0;
}

sds* query_tree(sds src, TSLanguage* lang, TSTree* tree, sds query_string) {
  START_ZONE;
  sds* s_arr = (void*)0;
  TSQueryCursor* cursor = (void*)0;
  TSQuery* query = (void*)0;
  TSNode root_node = ts_tree_root_node(tree);
  TSQueryError err = {0};
  uint32_t err_offset = 0;
  query =
      ts_query_new(lang, query_string, sdslen(query_string), &err_offset, &err);
  if (!query) {
    switch (err) {
      case TSQueryErrorSyntax:
        log_fatal(
            "tree_sitter::query_tree syntax error in query: %s at byte offset: "
            "%u",
            query_string, err_offset);
        break;
      default:
        message_fatal("tree_sitter::query_tree failed in creating query");
        break;
    }
    goto error_end;
  }
  cursor = ts_query_cursor_new();
  if (!cursor) {
    message_fatal("tree_sitter::query_tree failed in creating cursor");
    goto error_end;
  }
  ts_query_cursor_exec(cursor, query, root_node);
  TSQueryMatch match = {0};
  bool matches_remain = false;
  do {
    matches_remain = ts_query_cursor_next_match(cursor, &match);
    if (matches_remain && (match.capture_count != 0)) {
      sds captured_src = sdscatsds(sdsempty(), src);
      TSNode captured_node = match.captures->node;
      uint32_t start_byte = ts_node_start_byte(captured_node);
      uint32_t end_byte = ts_node_end_byte(captured_node);
      sdsrange(captured_src, start_byte, end_byte - 1);
      arrput(s_arr, captured_src);
    }
  } while (matches_remain);
  ts_query_delete(query);
  ts_query_cursor_delete(cursor);
  END_ZONE;
  return s_arr;
error_end:
  ts_query_delete(query);
  END_ZONE;
  return (void*)0;
}

#ifdef UNIT_TEST_TREE_SITTER

#include "test_deps/utest.h"

UTEST(tree_sitter, sample_test) { ASSERT_TRUE(true); }

UTEST_MAIN();

#endif
