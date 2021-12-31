#include <janet.h>

#include "trace.h"

INIT_TRACE;

JanetTable* lisp_init_env(void) {
  START_ZONE;
  janet_init();
  JanetTable* env = janet_core_env(NULL);
  END_ZONE;
  return env;
}

void lisp_terminate(void) {
  START_ZONE;
  janet_deinit();
  END_ZONE;
}

void lisp_register_module(JanetTable* env, char const* module_name,
                          JanetReg* cfuns) {
  START_ZONE;
  janet_cfuns_prefix(env, module_name, cfuns);
  END_ZONE;
}

int lisp_execute_script(JanetTable* env, char const* src, Janet* out) {
  START_ZONE;
  int rc = janet_dostring(env, src, "main", out);
  END_ZONE;
  return rc;
}