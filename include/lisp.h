#ifndef SCRIBE_LISP_H
#define SCRIBE_LISP_H

#include <janet.h>

JanetTable* lisp_init_env(void);
void lisp_register_module(JanetTable* env, char const* module_name,
                          JanetReg* cfuns);
void lisp_terminate(void);
int lisp_execute_script(JanetTable* env, char const* src);

#endif  // SCRIBE_LISP_H
