#include <stdio.h>

#include "impl.h"
#include "tagger.h"

#define BUFFER_SIZE (2560+256)

/* Compiler option */
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

static int 
tagger_fileRead(tagger *t)
{
	char	file[256] = {0};
	char	tags[2560] = {0};
	char	buffer[BUFFER_SIZE] = {0};
	unsigned int rc = BUFFER_SIZE;
	unsigned int isFile = 1;
	unsigned int si = 0;
	unsigned int i;

	while (rc == BUFFER_SIZE)
	{
		rc = fread(buffer, 1, BUFFER_SIZE, t->fp);

		for (i = 0; i < rc; ++i)
		{
			switch (buffer[i])
			{
			case 0:	/* End of file */
				rc = 0;
			case 1:	/* Mode switcher */
				isFile = !isFile;

				/* Both file and tags complete */
				if (isFile)
				{
					tags[si] = '\0';
					hmap_set(&(t->files_hm), file, tags);
				}
				else
				{
					file[si] = '\0';
				}

				si = 0;
				break;
			default:
				if (isFile)
				{
					file[si++] = buffer[i];
				}
				else
				{
					tags[si++] = buffer[i];
				}
			}
		}
	}

	return 0;
}

static int
tagger_fileInit(tagger *t)
{
	if (t->fp == NULL)
	{
		/* First time init fp */
		t->fp = fopen(t->path, "rb+");

		if (t->fp == NULL)
		{
			/* File does not exists, create it */
			if (create())
			{
				fprintf(stderr, "ERROR: Cannot initialize file \"%s\"!\n", t->path);
				return 1;
			}
		}
	}

	return tagger_fileRead(t);
}

static inline char *
tagger_isFileStr(int isFile)
{
	switch (isFile)
	{
	case 0:
		return "Tags";
	default:
		return "File";
	}
}

inline tagger
tagger_init(void)
{
	tagger t = (const tagger){0};

	setpath(t.path);

	return t;
}

inline void
tagger_deinit(tagger *t)
{
	if (t->fp != NULL)
		fclose(t->fp);
}

int
tagger_add(tagger *t, char *arg)
{
	static int	isfile = 1;
	static char	file[256];

	if (tagger_fileInit(t))
		return 1;

	if (isfile)
	{
		strucpy(file, arg);
	}
	else
	{
		hmap_setcat(&(t->files_hm), file, arg);
	}
	isfile = !isfile;

	return 0;
}

int
tagger_updateFile(tagger *t)
{
	hmap_print(&(t->files_hm));
	return 0;
}

