#ifndef IMPL_H
#define IMPL_H

#define FILE_BUFFER_SIZE 	(256)
#define MAX_TAGS_LEN		(1256)

#define SEP_ITEM	1
#define SEP_SEC		2
#define END		3

typedef struct {
	char *cmd;
	int (*func)(int, char **);
	void *ptr;
} cmds_t;

typedef struct {
	int argc;
	char **argv;
} args;

/* str.c */
int strsize(const char *s);
int strsame(const char *a, const char *b);
void strucat(unsigned char *dest, const unsigned char *src);

/* file.c */
const char *homedir(void);
void setpath(char *path);
int create(void);
#endif /* IMPL_H */
