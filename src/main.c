#include "indexer.h"
#include "repl.h"

int main(int argc, char** argv) {
  persist_project_details(".");
  index_files(".");
  indexer_terminate();
  return launch_repl(argc, argv);
}
