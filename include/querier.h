#ifndef SCRIBE_QUERIER_H
#define SCRIBE_QUERIER_H

#include <janet.h>
#include <stdbool.h>

bool db_exists(char const* path);
void register_modules(JanetTable* env);

#endif  // SCRIBE_QUERIER_H
