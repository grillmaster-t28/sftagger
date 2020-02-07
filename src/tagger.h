#ifndef TAGGER_H
#define TAGGER_H
typedef struct {
	FILE 	*fp;
	char	path[256];
} tagger;

tagger *tagger_init(void);
void tagger_deinit(tagger *t);

int tagger_add(tagger *t, char *arg);
#endif /* TAGGER_H */
