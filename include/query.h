#ifndef SCRIBE_QUERIER_H
#define SCRIBE_QUERIER_H

#include <janet.h>
#include <sds.h>
#include <stdbool.h>

bool db_exists(char const* path);
void register_modules(JanetTable* env);
sds get_language(void);

#endif  // SCRIBE_QUERIER_H
