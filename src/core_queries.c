#include <deps/cute_files.h>
#include <deps/cute_path.h>
#include <janet.h>
#include <lmdb.h>
#include <stdbool.h>

#include "db.h"
#include "lisp.h"
#include "trace.h"

INIT_TRACE;

static bool db_exists(char const* path) {
  START_ZONE;
  char db_dir_path[1024];
  path_concat(path, "scribe_db", db_dir_path, 1024);
  int rc = cf_file_exists(db_dir_path);
  if (rc == 1) {
    END_ZONE;
    return true;
  }
  END_ZONE;
  return false;
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

static const JanetReg core_cfuns[] = {
    {"file-src", cfun_file_src, "(core/file-src)\n\nGet the file source."},
};

void register_core_module(JanetTable* env) {
  lisp_register_module(env, "core", core_cfuns);
}

#ifdef UNIT_TEST_CORE_QUERIES

#include "test_deps/utest.h"

UTEST(core_queries, sample_test) { ASSERT_TRUE(true); }

UTEST_MAIN();

#endif
