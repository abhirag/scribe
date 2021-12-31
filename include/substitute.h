#ifndef SCRIBE_SUBSTITUTE_H
#define SCRIBE_SUBSTITUTE_H

#include <md4c.h>
#include <sds.h>
#include <stdbool.h>

typedef struct md_substitute_data md_substitute_data;
struct md_substitute_data {
  sds output;
  sds code_text;
  bool is_ordered_list;
  unsigned int current_index;
};

int md_substitute(const MD_CHAR* input, MD_SIZE input_size,
                  md_substitute_data* data);

#endif  // SCRIBE_SUBSTITUTE_H
