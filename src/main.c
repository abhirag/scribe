#include "indexer.h"
#include "repl.h"

int main(int argc, char** argv) {
  index_files(".");
  indexer_terminate();
  return launch_repl(argc, argv);
}
