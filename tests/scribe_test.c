#include <mkdirp.h>

#include "db.h"
#include "test_deps/utest.h"
#include "trace.h"

#define EQ0(x) ASSERT_EQ((x), 0)
#define NEQ0(x) ASSERT_NE((x), 0)
#define NNULL(p) ASSERT_TRUE((p) != (void*)0)

UTEST(db, put_and_get) {
  MARK_FRAME("db_put_and_get");
  EQ0(mkdirp("./temp_db", 0777));
  MDB_env* env = db_env_init("./temp_db", false, 100);
  NNULL(env);
  MDB_txn* txn = db_txn_init(env, false);
  NNULL(txn);
  MDB_dbi db_handle = db_get_handle(txn, "test", true);
  NEQ0(db_handle);
  sds key = sdsnew("this is a key");
  NNULL(key);
  sds value = sdsnew("this is a value");
  NNULL(value);
  EQ0(db_put(txn, db_handle, key, value));
  EQ0(db_txn_terminate(txn, true));
  txn = db_txn_init(env, false);
  NNULL(txn);
  sds want = sdsnew("this is a value");
  sds got = db_get(txn, db_handle, key);
  EQ0(sdscmp(got, want));
  EQ0(db_txn_terminate(txn, false));
  db_env_terminate(env);
  sdsfree(want);
  sdsfree(got);
  sdsfree(key);
  sdsfree(value);
}

UTEST_MAIN();
