#include <deps/cute_files.h>
#include <deps/cute_path.h>
#include <janet.h>
#include <lmdb.h>
#include <sds.h>
#include <stdbool.h>

#include "c_queries.h"
#include "core_queries.h"
#include "db.h"
#include "trace.h"

INIT_TRACE;

bool db_exists(char const* path) {
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

static sds get_language(void) {
  START_ZONE;
  if (!db_exists(".")) {
    message_fatal(
        "querier::get_language failed because scribe db not found in the "
        "current directory");
    goto error_end;
  }
  MDB_env* env = db_env_init("./scribe_db", false, 100);
  if (!env) {
    message_fatal("querier::get_language failed in creating db environment");
    goto error_end;
  }
  MDB_txn* txn = db_txn_init(env, false);
  if (!txn) {
    message_fatal("querier::get_language failed in creating transaction");
    goto error_end;
  }
  MDB_dbi db_handle = db_get_handle(txn, "project", true);
  if (db_handle == 0) {
    message_fatal(
        "querier::get_language failed in creating db handle for name: project");
    goto error_end;
  }
  sds lang = db_get(txn, db_handle, "language");
  if (!lang) {
    message_fatal("querier::get_language failed in getting key: language");
    goto error_end;
  }
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return lang;
error_end:
  db_txn_terminate(txn, false);
  db_env_terminate(env);
  END_ZONE;
  return (void*)0;
}

void register_modules(JanetTable* env) {
  sds lang = get_language();
  sds c_lang = sdsnew("c");
  if (!lang) {
    message_fatal("querier::register_modules failed in getting language");
    return;
  }
  if (sdscmp(lang, c_lang) == 0) {
    register_core_module(env);
    register_c_module(env);
  }
  sdsfree(lang);
  sdsfree(c_lang);
}
