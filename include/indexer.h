#ifndef SCRIBE_INDEXER_H
#define SCRIBE_INDEXER_H

int persist_project_details(char const* path);
int index_files(char const* path);
void indexer_terminate(void);

#endif  // SCRIBE_INDEXER_H
