#ifndef IMPL_H
#define IMPL_H

#define FILE_BUFFER_SIZE 	(256)
#define READ_BUFFER_SIZE	(1256)

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
void strucat(char *dest, const char *src);
void strucpy(char *dest, const char *src);

/* file.c */
const char *homedir(void);
void setpath(char *path);
int create(void);
#endif /* IMPL_H */
