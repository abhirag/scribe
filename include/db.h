#ifndef SCRIBE_DB_H
#define SCRIBE_DB_H

#include <lmdb.h>
#include <sds.h>
#include <stdbool.h>

MDB_env* db_env_init(char const* path, bool read_only, MDB_dbi max_dbs);
void db_env_terminate(MDB_env* env);
MDB_txn* db_txn_init(MDB_env* env, bool read_only);
int db_txn_terminate(MDB_txn* txn, bool commit);
MDB_dbi db_get_handle(MDB_txn* txn, char const* name, bool create_if_not_exist);
int db_put(MDB_txn* txn, MDB_dbi db_handle, char* key, char* value);
sds db_get(MDB_txn* txn, MDB_dbi db_handle, char* key);

#endif  // SCRIBE_DB_H
