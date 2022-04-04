```config
(set-title "Documenting Scribe using itself") 
(set-author "Abhirag <hey@abhirag.com>")
(set-date "22 February 2022") 
(set-pwidth 500) 
(set-pheight 200)
```
```fe
(abstract "Reading and comprehending a codebase is such a time taking endeavor because even in well 
commented codebases all we have at our disposal is a grab bag of facts without a narrative tying them. 
The structure of our programs is still based on the whims of the compiler/interpreter and exploring 
codebases is still a very rigid and manual process.

Scribe is a tool based on the premise that code is data which aims to alleviate some of the problems 
mentioned above by exposing that data to the programmers for documentation and exploration.")
```
```c
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
```