#include <stdio.h>

#include "impl.h"
#include "tagger.h"

#define VERSION "v4.0"

/* Options */
#define STATE_NONE 		0
#define STATE_ADD		'a'
#define STATE_REMOVE_FILES	'r'
#define STATE_DELETE_TAGS	'd'
#define STATE_SEARCH		's'
#define STATE_TAGS		't'
#define STATE_HELP		'h'
#define STATE_VERSION		'v'

static inline int
help(int retval)
{
	printf( "sftagger (" VERSION ")\n"
		"\n"
		"Help:\n"
		"\n"
		"-a file tag0,tag1,tagx  To add tag(s) to the file specified\n"
		"-r file                 To remove the file\n"
		"-d file tag0,tag1,tagx  To delete tag(s) from the file specified\n"
		"-s tag0 tag1 tagx       To search for the file(s) with the tag(s)\n"
		"-t file                 To get the tags of the file\n"
		"-h                      Displays this help\n"
		"-v                      Displays the version\n"
		"\n");
	return retval;
}

static inline void
version(void)
{
	puts(VERSION);
}

int
main(int argc, char **argv)
{
	char 	state = STATE_NONE;
	int 	i;
	int	skip;
	int	retval = 0;
	tagger	t;

	if (argc < 2)
	{
		return help(1);
	}

	t = tagger_init();

	for (i = 0; i < argc; ++i)
	{
		/* Set the state if an option found */
		if (argv[i][0] == '-')
		{
			if (argv[i][1] == '\0')
			{
				fprintf(stderr, "ERROR: \"-\" must have an option following it.\n");
			}
			else
			{
				state = argv[i][1];
			}

			/*
			 * Skip state usage if it is NOT a direct option
			 * usage (no additional parameters and not possible
			 * error)
			 */
			switch (state)
			{
			case STATE_ADD:
			case STATE_REMOVE_FILES:
			case STATE_DELETE_TAGS:
			case STATE_SEARCH:
			case STATE_TAGS:
				skip = 1;
				break;
			default:
				skip = 0;
			}

			if (skip)
			{
				continue;
			}
		}

		switch (state)
		{
		case STATE_NONE:
			break;

		/* Additional parameters puts in */
		case STATE_ADD:
			if (tagger_add(&t, argv[i]))
				retval = help(1);
			break;
		case STATE_REMOVE_FILES:
			break;
		case STATE_DELETE_TAGS:
			break;
		case STATE_SEARCH:
			break;
		case STATE_TAGS:
			break;

		/* Returns early */
		case STATE_HELP:
			retval = help(0);
			break;
		case STATE_VERSION:
			version();
			break;
		default:
			fprintf(stderr, "ERROR: Option '%c' not found!\n", state);
			retval = help(1);
		}

		if (retval)
			break;
	}

	tagger_updateFile(&t);
	tagger_deinit(&t);

	return retval;
}
