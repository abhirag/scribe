#include <deps/stb_ds.h>
#include <janet.h>
#include <tree_sitter/api.h>

#include "lisp.h"
#include "query.h"
#include "trace.h"
#include "tree_sitter.h"

INIT_TRACE;

TSLanguage* tree_sitter_c();

sds c_function_definition(JanetString name, JanetString src) {
  sds src_sds = sdsnew(src);
  sds query_sds = sdsnew(
      "(function_definition (function_declarator (identifier) @func_name)) "
      "@func_def");
  TSParser* parser = (void*)0;
  TSTree* tree = (void*)0;
  parser = create_parser(tree_sitter_c());
  if (!parser) {
    message_fatal("c_queries::c_function_definition failed in creating parser");
    goto end;
  }
  tree = parse_string(parser, src_sds);
  if (!tree) {
    message_fatal("c_queries::c_function_definition failed in parsing");
    goto end;
  }
  sds func_def =
      query_filter_tree(src_sds, tree_sitter_c(), tree, query_sds, name, 1);
end:
  sdsfree(src_sds);
  sdsfree(query_sds);
  ts_parser_delete(parser);
  ts_tree_delete(tree);
  END_ZONE;
  return func_def;
}

static Janet cfun_c_function_definition(int32_t argc, Janet* argv) {
  janet_fixarity(argc, 2);
  if (!db_exists(".")) {
    janet_panicf("scribe db not found in the current directory");
  }
  JanetString name = janet_getstring(argv, 0);
  JanetString src = janet_getstring(argv, 1);
  sds func_def = c_function_definition(name, src);
  if (!func_def) {
    janet_panicf("no result to display");
  }
  const uint8_t* jstr = janet_string(func_def, sdslen(func_def));
  sdsfree(func_def);
  return janet_wrap_string(jstr);
}

sds* c_tree_sitter_query(JanetString query, JanetString src) {
  START_ZONE;
  sds src_sds = sdsnew(src);
  sds query_sds = sdsnew(query);
  sds* strs = (void*)0;
  TSParser* parser = (void*)0;
  TSTree* tree = (void*)0;
  parser = create_parser(tree_sitter_c());
  if (!parser) {
    message_fatal("c_queries::c_tree_sitter_query failed in creating parser");
    goto end;
  }
  tree = parse_string(parser, src_sds);
  if (!tree) {
    message_fatal("c_queries::c_tree_sitter_query failed in parsing");
    goto end;
  }
  strs = query_tree(src_sds, tree_sitter_c(), tree, query_sds);
end:
  sdsfree(src_sds);
  sdsfree(query_sds);
  ts_parser_delete(parser);
  ts_tree_delete(tree);
  END_ZONE;
  return strs;
}

static Janet cfun_c_tree_sitter_query(int32_t argc, Janet* argv) {
  janet_fixarity(argc, 2);
  if (!db_exists(".")) {
    janet_panicf("scribe db not found in the current directory");
  }
  JanetString query = janet_getstring(argv, 0);
  JanetString src = janet_getstring(argv, 1);
  sds* strs = c_tree_sitter_query(query, src);
  if (!strs) {
    janet_panicf("no results to display");
  }
  int num_strs = arrlen(strs);
  JanetArray* jarr = janet_array(num_strs);
  jarr->count = num_strs;
  for (int i = 0; i < num_strs; i += 1) {
    const uint8_t* jstr = janet_string(strs[i], sdslen(strs[i]));
    jarr->data[i] = janet_wrap_string(jstr);
  }
  for (int i = 0; i < num_strs; i += 1) {
    sdsfree(strs[i]);
  }
  arrfree(strs);
  return janet_wrap_array(jarr);
}

static const JanetReg c_cfuns[] = {
    {"tree-sitter-query", cfun_c_tree_sitter_query,
     "(c/tree-sitter-query)\n\nExecute a tree-sitter query which captures one "
     "node"},
    {"function-definition", cfun_c_function_definition,
     "(c/function-definition)\n\nReturn function defined by name"
     "node"},
};

void register_c_module(JanetTable* env) {
  lisp_register_module(env, "c", c_cfuns);
}
