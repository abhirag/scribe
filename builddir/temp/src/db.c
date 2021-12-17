#include "db.h"

#include <errno.h>
#include <lmdb.h>
#include <sds.h>
#include <stdbool.h>
#include <string.h>

#include "trace.h"

INIT_TRACE;

MDB_env* db_env_init(char const* path, bool read_only, MDB_dbi max_dbs) {
  START_ZONE;
  int rc = 0;
  unsigned int flags = 0;
  if (read_only) {
    flags = MDB_RDONLY;
  }
  MDB_env* env = (void*)0;
  rc = mdb_env_create(&env);
  if (rc != 0) {
    message_fatal("db::db_env_init failed in creating LMDB environment handle");
    goto error_end;
  }
  rc = mdb_env_set_maxdbs(env, max_dbs);
  if (rc != 0) {
    message_fatal(
        "db::db_env_init failed in setting the maximum number of named "
        "databases "
        "for the environment");
    goto error_end;
  }
  rc = mdb_env_open(env, path, flags, 0664);
  if (rc != 0) {
    message_fatal("db::db_env_init failed in opening the environment handle");
    goto error_end;
  }
  END_ZONE;
  return env;
error_end:
  mdb_env_close(env);
  END_ZONE;
  return (void*)0;
}

void db_env_terminate(MDB_env* env) { mdb_env_close(env); }

MDB_txn* db_txn_init(MDB_env* env, bool read_only) {
  START_ZONE;
  MDB_txn* txn = (void*)0;
  int rc = 0;
  unsigned int flags = 0;
  if (read_only) {
    flags = MDB_RDONLY;
  }
  rc = mdb_txn_begin(env, (void*)0, flags, &txn);
  if (rc != 0) {
    switch (rc) {
      case MDB_PANIC:
        message_fatal(
            "db::db_txn_init a fatal error occurred earlier and the "
            "environment must be shut down");
        goto error_end;
      case MDB_MAP_RESIZED:
        message_fatal(
            "db::db_txn_init another process wrote data beyond this MDB_env's "
            "mapsize and this "
            "environment's map must be resized as well. See "
            "mdb_env_set_mapsize()");
        goto error_end;
      case MDB_READERS_FULL:
        message_fatal(
            "db::db_txn_init a read-only transaction was requested and the "
            "reader lock table "
            "is full. See mdb_env_set_maxreaders()");
        goto error_end;
      case ENOMEM:
        message_fatal("db::db_txn_init out of memory");
        goto error_end;
      default:
        message_fatal("db::db_txn_init failed in creating a transaction");
        goto error_end;
    }
  }
  END_ZONE;
  return txn;
error_end:
  mdb_txn_abort(txn);
  mdb_env_close(env);
  END_ZONE;
  return (void*)0;
}

int db_txn_terminate(MDB_txn* txn, bool commit) {
  START_ZONE;
  int rc = 0;
  if (commit) {
    rc = mdb_txn_commit(txn);
    if (rc != 0) {
      message_fatal(
          "db::db_txn_terminate failed in committing the transaction");
    }
    END_ZONE;
    return rc;
  }
  mdb_txn_abort(txn);
  END_ZONE;
  return rc;
}

MDB_dbi db_get_handle(MDB_txn* txn, char const* name,
                      bool create_if_not_exist) {
  START_ZONE;
  int rc = 0;
  MDB_env* txn_env = mdb_txn_env(txn);
  MDB_dbi db_handle = 0;
  unsigned int flags = 0;
  if (create_if_not_exist) {
    flags = MDB_CREATE;
  }
  rc = mdb_dbi_open(txn, name, flags, &db_handle);
  if (rc != 0) {
    switch (rc) {
      case MDB_NOTFOUND:
        message_fatal(
            "db::db_get_handle the specified database doesn't exist in the "
            "environment and MDB_CREATE was not specified");
        goto error_end;
      case MDB_DBS_FULL:
        message_fatal(
            "db::db_get_handle too many databases have been opened. See "
            "mdb_env_set_maxdbs()");
        goto error_end;
      default:
        message_fatal("db::db_get_handle failed in opening a database");
        goto error_end;
    }
  }
  END_ZONE;
  return db_handle;
error_end:
  mdb_txn_abort(txn);
  mdb_env_close(txn_env);
  END_ZONE;
  return 0;
}

int db_put(MDB_txn* txn, MDB_dbi db_handle, char* key, char* value) {
  START_ZONE;
  int rc = 0;
  unsigned int flags = MDB_NOOVERWRITE;
  MDB_val key_val = {.mv_size = strlen(key) + 1, .mv_data = (void*)key};
  MDB_val data_val = {.mv_size = strlen(value) + 1, .mv_data = (void*)value};
  rc = mdb_put(txn, db_handle, &key_val, &data_val, flags);
  if (rc != 0) {
    if (rc == MDB_KEYEXIST) {
      message_error("db::db_put key already exists");
    } else {
      message_error("db::db_put put failed");
    }
  }
  END_ZONE;
  return rc;
}

sds db_get(MDB_txn* txn, MDB_dbi db_handle, char* key) {
  START_ZONE;
  int rc = 0;
  MDB_val key_val = {.mv_size = strlen(key) + 1, .mv_data = (void*)key};
  MDB_val data = {0};
  rc = mdb_get(txn, db_handle, &key_val, &data);
  if (rc != 0) {
    if (rc == MDB_NOTFOUND) {
      message_error("db::db_get the key was not in the database");
      goto error_end;
    } else {
      message_error("db::db_get get failed");
      goto error_end;
    }
  }
  sds value = sdsnew((char*)data.mv_data);
  END_ZONE;
  return value;
error_end:
  END_ZONE;
  return (void*)0;
}
