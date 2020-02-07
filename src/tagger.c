#include <stdio.h>
#include <stdlib.h>

#include "impl.h"
#include "tagger.h"

tagger *
tagger_init(void)
{
	tagger *t = malloc(sizeof(tagger));

	t->fp = NULL;
	setpath(t->path);

	return t;
}

void
tagger_deinit(tagger *t)
{
	if (t->fp != NULL)
		fclose(t->fp);
}

int
tagger_add(tagger *t, char *arg)
{
	static int		isfile = 1;

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

	/*
	 * strucat(tags, (unsigned char *) arg);
	 * TODO
	 */
	printf("Added (%d): %s\n", isfile, arg);
	isfile = !isfile;

	return 0;
}
