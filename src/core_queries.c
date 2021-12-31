#include <deps/cute_files.h>
#include <deps/cute_path.h>
#include <janet.h>
#include <lmdb.h>
#include <stdbool.h>
#include <stdlib.h>

#include "db.h"
#include "lisp.h"
#include "query.h"
#include "trace.h"

INIT_TRACE;

static sds list_files(JanetString path);
static sds list_paths(void);
static sds get_file_src(JanetString path, JanetString name);
static Janet cfun_list_files(int32_t argc, Janet* argv);
static Janet cfun_list_paths(int32_t argc, Janet* argv);
static Janet cfun_file_src(int32_t argc, Janet* argv);
static long int get_file_num_lines(JanetString path, JanetString name);
static sds get_file_src_slice(JanetString path, JanetString name,
                              int64_t start_line, int64_t end_line);
static Janet cfun_file_src_slice(int32_t argc, Janet* argv);

static sds list_files(JanetString path) {
  START_ZONE;
  MDB_env* env = db_env_init("./scribe_db", false, 100);
  if (!env) {
    message_fatal("core_queries::list_files failed in creating db environment");
    goto error_end;
  }
  MDB_txn* txn = db_txn_init(env, false);
  if (!txn) {
    message_fatal("core_queries::list_files failed in creating transaction");
    goto error_end;
  }
  MDB_dbi db_handle = db_get_handle(txn, path, true);
  if (db_handle == 0) {
    log_fatal(
        "core_queries::list_files failed in creating db handle for name: %s",
        path);
    goto error_end;
  }
  sds listing = db_list_keys(txn, db_handle, true);
  if (!listing) {
    message_fatal("core_queries::list_files failed in listing keys");
    goto error_end;
  }
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return listing;
error_end:
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return (void*)0;
}

static sds list_paths(void) {
  START_ZONE;
  MDB_env* env = db_env_init("./scribe_db", false, 100);
  if (!env) {
    message_fatal("core_queries::list_paths failed in creating db environment");
    goto error_end;
  }
  MDB_txn* txn = db_txn_init(env, false);
  if (!txn) {
    message_fatal("core_queries::list_paths failed in creating transaction");
    goto error_end;
  }
  MDB_dbi db_handle = db_get_handle(txn, "paths", true);
  if (db_handle == 0) {
    log_fatal(
        "core_queries::list_paths failed in creating db handle for name: "
        "paths");
    goto error_end;
  }
  sds listing = db_list_keys(txn, db_handle, false);
  if (!listing) {
    message_fatal("core_queries::list_paths failed in listing keys");
    goto error_end;
  }
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return listing;
error_end:
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return (void*)0;
}

static sds get_file_src(JanetString path, JanetString name) {
  START_ZONE;
  MDB_env* env = db_env_init("./scribe_db", false, 100);
  if (!env) {
    message_fatal(
        "core_queries::get_file_src failed in creating db environment");
    goto error_end;
  }
  MDB_txn* txn = db_txn_init(env, false);
  if (!txn) {
    message_fatal("core_queries::get_file_src failed in creating transaction");
    goto error_end;
  }
  MDB_dbi db_handle = db_get_handle(txn, path, true);
  if (db_handle == 0) {
    log_fatal(
        "core_queries::get_file_src failed in creating db handle for name: %s",
        path);
    goto error_end;
  }
  sds src = db_get(txn, db_handle, name);
  if (!src) {
    log_fatal("core_queries::get_file_src failed in getting key: %s", name);
    goto error_end;
  }
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return src;
error_end:
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return (void*)0;
}

static long int get_file_num_lines(JanetString path, JanetString name) {
  START_ZONE;
  sds num_lines_key = sdsempty();
  sds num_lines = sdsempty();
  MDB_env* env = db_env_init("./scribe_db", false, 100);
  if (!env) {
    message_fatal(
        "core_queries::get_file_num_lines failed in creating db environment");
    goto error_end;
  }
  MDB_txn* txn = db_txn_init(env, false);
  if (!txn) {
    message_fatal(
        "core_queries::get_file_num_lines failed in creating transaction");
    goto error_end;
  }
  MDB_dbi db_handle = db_get_handle(txn, path, true);
  if (db_handle == 0) {
    log_fatal(
        "core_queries::get_file_num_lines failed in creating db handle for "
        "name: %s",
        path);
    goto error_end;
  }
  num_lines_key = sdscatfmt(num_lines_key, "%s::%s", name, "num_lines");
  num_lines = db_get(txn, db_handle, num_lines_key);
  if (!num_lines) {
    log_fatal("core_queries::get_file_num_lines failed in getting key: %s",
              num_lines_key);
    goto error_end;
  }
  char* e;
  long int num_lines_int = strtol(num_lines, &e, 10);
  if (*e != '\0') {
    log_fatal(
        "core_queries::get_file_num_lines failed in converting string: %s to "
        "integer",
        num_lines);
  }
  sdsfree(num_lines_key);
  sdsfree(num_lines);
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return num_lines_int;
error_end:
  sdsfree(num_lines_key);
  sdsfree(num_lines);
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return -1;
}

static sds get_file_src_slice(JanetString path, JanetString name,
                              int64_t start_line, int64_t end_line) {
  START_ZONE;
  sds* lines = (void*)0;
  sds file_src_slice = sdsempty();
  int count = 0;
  sds file_src = get_file_src(path, name);
  if (!file_src) {
    message_fatal(
        "core_queries::get_file_src_slice failed in getting file source");
    goto error_end;
  }
  long int num_lines = get_file_num_lines(path, name);
  if (num_lines == -1) {
    message_fatal(
        "core_queries::get_file_src_slice failed in getting number of lines");
    goto error_end;
  }
  if (end_line > num_lines) {
    log_fatal(
        "core_queries::get_file_src_slice invalid argument: end_line=%d is "
        "greater than num_lines=%d",
        (int)end_line, (int)num_lines);
    goto error_end;
  }
  lines = sdssplitlen(file_src, sdslen(file_src), "\n", 1, &count);
  for (int i = (start_line - 1); i < end_line; i += 1) {
    file_src_slice = sdscatfmt(file_src_slice, "%S\n", lines[i]);
  }
  file_src_slice = sdscatfmt(file_src_slice, "%S", lines[end_line]);
  sdsfree(file_src);
  sdsfreesplitres(lines, count);
  END_ZONE;
  return file_src_slice;
error_end:
  sdsfree(file_src);
  sdsfreesplitres(lines, count);
  END_ZONE;
  return (void*)0;
}

static Janet cfun_list_files(int32_t argc, Janet* argv) {
  janet_fixarity(argc, 1);
  if (!db_exists(".")) {
    janet_panicf("scribe db not found in the current directory");
  }
  JanetString path = janet_getstring(argv, 0);
  sds listing = list_files(path);
  if (!listing) {
    janet_panicf("failed in listing keys");
  }
  const uint8_t* jstr = janet_string(listing, sdslen(listing));
  sdsfree(listing);
  return janet_wrap_string(jstr);
}

static Janet cfun_list_paths(int32_t argc, Janet* argv) {
  janet_fixarity(argc, 0);
  if (!db_exists(".")) {
    janet_panicf("scribe db not found in the current directory");
  }
  sds listing = list_paths();
  if (!listing) {
    janet_panicf("failed in listing keys");
  }
  const uint8_t* jstr = janet_string(listing, sdslen(listing));
  sdsfree(listing);
  return janet_wrap_string(jstr);
}

static Janet cfun_file_src(int32_t argc, Janet* argv) {
  janet_fixarity(argc, 2);
  if (!db_exists(".")) {
    janet_panicf("scribe db not found in the current directory");
  }
  JanetString path = janet_getstring(argv, 0);
  JanetString name = janet_getstring(argv, 1);
  sds src = get_file_src(path, name);
  if (!src) {
    janet_panicf("failed in getting value for the specified key");
  }
  const uint8_t* jstr = janet_string(src, sdslen(src));
  sdsfree(src);
  return janet_wrap_string(jstr);
}

static Janet cfun_file_src_slice(int32_t argc, Janet* argv) {
  janet_fixarity(argc, 4);
  if (!db_exists(".")) {
    janet_panicf("scribe db not found in the current directory");
  }
  JanetString path = janet_getstring(argv, 0);
  JanetString name = janet_getstring(argv, 1);
  int64_t start_line = janet_getinteger64(argv, 2);
  int64_t end_line = janet_getinteger64(argv, 3);
  if (start_line <= 0) {
    janet_panicf("start-line needs to be >= 1");
  }
  if (end_line <= 0) {
    janet_panicf("end-line needs to be >= 1");
  }
  if (end_line < start_line) {
    janet_panicf("end-line needs to be >= start-line");
  }
  sds src_slice = get_file_src_slice(path, name, start_line, end_line);
  if (!src_slice) {
    janet_panicf("failed in getting the slice");
  }
  const uint8_t* jstr = janet_string(src_slice, sdslen(src_slice));
  sdsfree(src_slice);
  return janet_wrap_string(jstr);
}

static const JanetReg core_cfuns[] = {
    {"file-src", cfun_file_src, "(core/file-src)\n\nGet the file source."},
    {"list-paths", cfun_list_paths,
     "(core/list-paths)\n\nList the indexed paths."},
    {"list-files", cfun_list_files,
     "(core/list-files)\n\nList the files in the directory."},
    {"file-src-slice", cfun_file_src_slice,
     "(core/file-src-slice)\n\nGet the file source sliced by line nums."},
};

void register_core_module(JanetTable* env) {
  lisp_register_module(env, "core", core_cfuns);
}

#ifdef UNIT_TEST_CORE_QUERIES

#include "test_deps/utest.h"

UTEST(core_queries, sample_test) { ASSERT_TRUE(true); }

UTEST_MAIN();

#endif
