#ifndef SCRIBE_INDEXER_H
#define SCRIBE_INDEXER_H

int persist_project_details(char const* path);
int index_files(char const* path);
void indexer_terminate(void);
char* read_file_to_str(const char* path, unsigned int* file_len_out);

#endif  // SCRIBE_INDEXER_H
