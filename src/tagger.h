#ifndef TAGGER_H
#define TAGGER_H

#include "hmap.h"
typedef struct {
	FILE 	*fp;
	char	path[256];
	hmap	files_hm;
} tagger;

tagger tagger_init(void);
void tagger_deinit(tagger *t);

int tagger_add(tagger *t, char *arg);
int tagger_updateFile(tagger *t);
#endif /* TAGGER_H */
