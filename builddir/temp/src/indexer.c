#include <janet.h>
#include <sds.h>
#include <stdbool.h>
#define CUTE_FILES_IMPLEMENTATION
#include <deps/cute_files.h>
#define CUTE_PATH_IMPLEMENTATION
#include <deps/cute_path.h>
#define STB_DS_IMPLEMENTATION
#include <deps/stb_ds.h>

#include "lisp.h"
#include "trace.h"

INIT_TRACE;

typedef struct file_info file_info;
struct file_info {
  sds name;
  sds path;
  char* contents;
  unsigned int length;
  unsigned int num_lines;
};

static char* read_file_to_str(const char* path, unsigned int* file_len_out);
static void get_file_info(cf_file_t* file, void* udata);
static void collect_file_info(char const* path);
static char* read_scribe_file(char const* path);
static int set_language(char const* lang);
static Janet cfun_set_language(int32_t argc, Janet* argv);
static int execute_scribe_file(char const* path);

static char** exts = (void*)0;
static file_info* finfos = (void*)0;

static char* read_file_to_str(const char* path, unsigned int* file_len_out) {
  START_ZONE;
  FILE* file;
  int e;

  file = fopen(path, "rb");
  if (!file) {
    log_error("indexer::read_file_to_str unable to open file %s", path);
    return (void*)0;
  }

  e = fseek(file, 0, SEEK_END);
  if (-1 == e) {
    log_error("indexer::read_file_to_str unable to seek file %s", path);
    fclose(file);
    return (void*)0;
  }

  long file_len = ftell(file);
  if (-1 == file_len) {
    log_error("indexer::read_file_to_str unable to ftell() file %s", path);
    fclose(file);
    return (void*)0;
  }

  e = fseek(file, 0, SEEK_SET);
  if (-1 == e) {
    log_error("indexer::read_file_to_str unable to seek file %s", path);
    fclose(file);
    return (void*)0;
  }

  char* contents = malloc(file_len + 1);
  if (!contents) {
    log_error("indexer::read_file_to_str memory error!");
    fclose(file);
    return (void*)0;
  }

  unsigned long bytes_read = fread(contents, file_len, 1, file);
  if (bytes_read == 0 && ferror(file)) {
    log_error("indexer::read_file_to_str read error");
    free(contents);
    fclose(file);
    return (void*)0;
  }
  fclose(file);

  contents[file_len] = '\0';

  if (file_len_out) *file_len_out = file_len + 1;

  END_ZONE;
  return contents;
}

static void get_file_info(cf_file_t* file, void* udata) {
  START_ZONE;
  bool file_filter = false;
  for (int i = 0; i < arrlen(exts); i += 1) {
    file_filter = file_filter || cf_match_ext(file, exts[i]);
  }
  if (file_filter) {
    char out[1024] = "";
    path_pop(file->path, out, (void*)0);
    sds name = sdsnew(file->name);
    sds path = sdsnew(out);
    unsigned int length = 0;
    unsigned int num_lines = 0;
    char* contents = read_file_to_str(file->path, &length);
    for (unsigned int i = 0; i < length; i += 1) {
      if (contents[i] == '\n') {
        num_lines += 1;
      }
    }
    file_info finfo = {.name = name,
                       .path = path,
                       .contents = contents,
                       .length = length,
                       .num_lines = num_lines};
    arrput(finfos, finfo);
  }
  END_ZONE;
}

static void collect_file_info(char const* path) {
  START_ZONE;
  cf_traverse(path, get_file_info, (void*)0);
  END_ZONE;
}

static char* read_scribe_file(char const* path) {
  START_ZONE;
  char scribe_file_path[1024] = "";
  path_concat(path, ".scribe", scribe_file_path, 1024);
  char* contents = read_file_to_str(scribe_file_path, (void*)0);
  END_ZONE;
  return contents;
}

static int set_language(char const* lang) {
  START_ZONE;
  if (strcmp(lang, "c") == 0) {
    arrput(exts, ".c");
    arrput(exts, ".h");
    END_ZONE;
    return 0;
  } else {
    message_fatal("indexer::set_language language not found!");
    END_ZONE;
    return -1;
  }
}

static Janet cfun_set_language(int32_t argc, Janet* argv) {
  START_ZONE;
  janet_fixarity(argc, 1);
  JanetString lang = janet_getstring(argv, 0);
  int rc = set_language(lang);
  if (rc != 0) {
    END_ZONE;
    janet_panicf("failed in setting language!");
  }
  END_ZONE;
  return janet_wrap_nil();
}

static const JanetReg config_cfuns[] = {
    {"set-language", cfun_set_language,
     "(config/set-language)\n\nSet the language."},
};

static int execute_scribe_file(char const* path) {
  START_ZONE;
  char* contents = read_scribe_file(path);
  if (!contents) {
    message_fatal(
        "indexer::execute_scribe_file failed in reading the scribe file");
    END_ZONE;
    return -1;
  }
  JanetTable* env = lisp_init_env();
  lisp_register_module(env, "config", config_cfuns);
  int rc = lisp_execute_script(env, contents);
  END_ZONE;
  return rc;
}

void index_files(char const* path) {
  START_ZONE;
  int rc = execute_scribe_file(path);
  if (rc != 0) {
    message_fatal("indexer::index_files failed in executing the scribe file");
  }
  collect_file_info(path);
  if (!finfos) {
    message_error(
        "indexer::index_files no files with the specified extensions were "
        "found");
    END_ZONE;
    return;
  }
  END_ZONE;
}
