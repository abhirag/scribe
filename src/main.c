#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "indexer.h"
#include "repl.h"
#include "substitute.h"

int main(int argc, char** argv) {
  persist_project_details(".");
  index_files(".");
  indexer_terminate();
  char* contents = read_file_to_str("./doc_in.md", (void*)0);
  md_substitute_data d = {.code_text = sdsempty(),
                          .output = sdsempty(),
                          .is_ordered_list = false,
                          .current_index = 0};
  md_substitute(contents, strlen(contents), &d);
  free(contents);
  FILE* fp = fopen("doc_out.md", "w");
  fprintf(fp, d.output);
  fclose(fp);
  return launch_repl(argc, argv);
}
