![Scribe](logo.png)
# 

## Abstract
According to a recent study[^1] the average percentage of time programmers spend on comprehending a codebase is 57.62%, which is on top of an average 23.96% time spent navigating the codebase. So, all-in-all an average of 81.58% time spent just trying to figure out the codebase.

Reading and comprehending a codebase is such a time taking endeavor because even in well commented codebases all we have at our disposal is a grab bag of facts without a narrative tying them. The structure of our programs is still based on the whims of the compiler/interpreter and exploring codebases is still a very rigid and manual process.

Scribe is a tool based on the premise that _code is data_ which aims to alleviate some of the problems mentioned above by exposing that data to the programmers for documentation and exploration.

## Introduction
Scribe is a tool for writing better documentation of programs. It borrows ideas from literate programming and augments them with static analysis.\
The basic idea is simple:
- Scribe first creates a database of all the code, indexed by a composite key of path and filename
- Entities such as functions, structs etc. can then be referred in documentation by a query over the created database
- During final export these queries are replaced with the code fragments they refer to. Each query is mapped to an underlying [tree-sitter query](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries) and the corresponding file is parsed on demand to evaluate it
- Query results are also persisted in the database, so that drift in between the code and documentation can be identified

An example will make things clearer, let’s say you have to document the following function:

```c
int editorRowHasOpenComment(erow *row) {
    if (row->hl && row->rsize && row->hl[row->rsize-1] == HL_MLCOMMENT &&
        (row->rsize < 2 || (row->render[row->rsize-2] != '*' ||
                            row->render[row->rsize-1] != '/'))) return 1;
    return 0;
}
```
You normally do that by writing a comment in the source file itself, such as:
```c
/* Return true if the specified row last char is part of a multi line comment
 * that starts at this row or at one before, and does not end at the end
 * of the row but spawns to the next row. */
int editorRowHasOpenComment(erow *row) {
    if (row->hl && row->rsize && row->hl[row->rsize-1] == HL_MLCOMMENT &&
        (row->rsize < 2 || (row->render[row->rsize-2] != '*' ||
                            row->render[row->rsize-1] != '/'))) return 1;
    return 0;
}
```
This is the status quo of documentation but it has a few issues:
- You have to make sure that code and the corresponding comment stay in sync
- This comment documents the function but doesn’t tell us anything about the broader narrative of how it fits in the system
- The narrative is lost because these facts need to be presented in a sequence such that they build upon each other. This narrative is what makes code readable, but I as a programmer don’t have the last word on how code is organized, compiler/interpreter does

The solution is simple, document your program in a separate document where you control the narrative. But how do you refer to code fragments in that document? Copy pasting code wouldn’t work because then documentation and code would drift apart with time.

Scribe solves this problem by letting you refer to entities in your codebase using a query written in lisp such as:
```clojure
(let [db-c (core/file-src "./src" "db.c")
      db_get (c/function-definition "db_get" db-c)]
  (core/src-slice db_get 3 15))
```
During the final export stage, this query is replaced with:
```c
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
```
i.e. lines 3 - 15 from the function named `db_get` in the `./src/db.c` file. Checksums of query results are maintained to detect drift between code and documentation.

You can also launch a lisp REPL over the codebase which makes writing these queries and exploring the codebase an interactive process.

## Architecture
- Code is indexed by a composite key of path and filename, then persisted in a [LMDB](http://www.lmdb.tech/doc/) database
- An embedded lisp ([janet](https://janet-lang.org/)) is used for configuration, and for querying the database
- Query written in `janet` is mapped to an underlying [tree-sitter query](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries) and the corresponding file is parsed on demand to evaluate it
- Markdown file is parsed to replace scribe queries with corresponding results, leaving the rest unchanged
- Query results are stored inside the database to make sure that code and documentation stay in sync

## Dependencies
All dependencies are vendored under `subprojects/`. No external dependencies 

## Build
Scribe is written in `C11`, so you will need a `C` compiler that supports that standard.
We are using [meson](https://mesonbuild.com/) with the [ninja](https://ninja-build.org/) backend:

| `meson`       | `ninja`       |
| ------------- | ------------- |
| 0.59.2        | 1.10.2        |

1. Get the `ninja` binary from [here](https://github.com/ninja-build/ninja/releases)
2. Add the location of your `ninja` binary to your PATH environment variable
3. [Install meson](https://mesonbuild.com/Quick-guide.html)
4. Navigate to the project directory
5. `$ meson builddir`
6. `$ cd builddir`
7. `$ meson compile`

## Status
Scribe is now capable of documenting itself. Once we finish documenting scribe using itself we will release an alpha version.
For now, it should be considered an early prototype capable of documenting `C` codebases

## Roadmap
Scribe can be extended to support over 40 programming languages because we use `tree-sitter` to parse and query the code. Moreover a common unified syntax can be used to query code from different languages. Keeping that in mind for the future, the immediate plan is:
- Support for querying more code entities
- Rust support
- Go support

[^1]: [Measuring Program Comprehension: A Large-Scale Field Study with Professionals](https://ieeexplore.ieee.org/document/7997917) _by Xia, Bao, Lo, Xing, Hassan, & Li_