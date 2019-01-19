/* See LICENSE file for copyright and license details.
 *
 * sftagger - Simple File Tagging tool is a simple but effective file tagger 
 * that just use one file to manage your files, usually in image situations.
 */

/* 4.0 - sftagger rework
 *
 * - Change from using flat file to sqlite3
 * - Options parameters completely changed, now as characters instead
 * - Everything rewritten again
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <unistd.h>
#include <limits.h>

#define OPERATION_FAILURE	(-1)
#define OPERATION_SUCCESS	(+0)

#define BUF_S		(  64)
#define BUF_M		( 256)
#define BUF_L		(2560)

#define VERSION		"4.0-a01 - 2019-01-18"
#define FILETARGET	".tags.db"
#define FILETARGETTEMP	".temp" FILETARGET

#define RENAME(x, y) \
	if (rename(x, y)) \
		printf("Error: Unable to rename file\n");

// Typenames
enum typename_e {
	TYPENAME_OTHER,
	TYPENAME_VOID_PTR,
	TYPENAME_CHAR_PTR,
	TYPENAME_CHAR,
	TYPENAME_INT,
	TYPENAME_UINT,
	TYPENAME_LINT,
	TYPENAME_FLOAT,
	TYPENAME_DOUBLE
};

// C11 Generics
#define typename(x) _Generic((x),\
	void *:		TYPENAME_VOID_PTR,	\
	char *:		TYPENAME_CHAR_PTR,	\
	char:		TYPENAME_CHAR,		\
	int:		TYPENAME_INT,		\
	unsigned int:	TYPENAME_UINT,		\
	long int:	TYPENAME_LINT,		\
	float:		TYPENAME_FLOAT,		\
	double:		TYPENAME_DOUBLE,	\
	default:	TYPENAME_OTHER		)

// enum declarations
typedef enum action_enum {
	HELP, RET_VERSION, NEW, READ, SEARCH, ALL, INSERT, RENAME
	, DELETE
} e_action;

typedef enum action_type_enum {
	NONE, CATEGORY, TAG, TAGFILE, FILENAME
} e_action_type;

// structs declarations
typedef struct args_struct {
	char		*db_filename;
	char		*arg;
	char		**outer;
	int		search_execute;
	e_action_type	type;
} s_args;

typedef struct options_struct {
	e_action	action;
	s_args		args;
	int		(*func)(s_args);
} s_opt;

/* SQL sqlite3 callback function
 *
 */
static int
sql_callback(void *dataset, int argc, char **argv, char **az_colname)
{
	for (int i=0; i < argc; ++i) {
		if (!strcmp(az_colname[i], "name")) {
			if (typename(dataset) == TYPENAME_CHAR_PTR) {
				sprintf(dataset, "%s%s "
						, strdup(dataset)
						, argv[i] ? argv[i] : "NULL");
			} else {
				fprintf(stdout, "%s "
						, argv[i] ? argv[i] : "NULL");
			}
		}
	}
	if (typename(dataset) != TYPENAME_CHAR_PTR) {
		putchar('\n');
	}

	return 0;
}

/* SQL sqlite3 file open function
 *
 */
static int
sql_open(const char *filename, sqlite3 **db)
{
	int	rc;

	// Open SQL database
	rc = sqlite3_open(filename, db);

	// Open status message
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n",
				sqlite3_errmsg(*db));
		sqlite3_close(*db);
		return (OPERATION_FAILURE);
	} else {
		return (OPERATION_SUCCESS);
	}
}

/* SQL sqlite3 execution function
 *
 */
static int
sql_exec(	sqlite3 *db,
		const char *sql,
		int (*sql_callback)(void *, int, char **, char **),
		void *arg,
		char **z_errmsg
		)
{
	int	rc;

	// Execute SQL statement
	rc = sqlite3_exec(db, sql, sql_callback, arg, z_errmsg);

	// Execute status message
	if (rc != SQLITE_OK) {
		fprintf(stderr, "\nSQL error: %s\n", *z_errmsg);
		sqlite3_free(*z_errmsg);
		return (OPERATION_FAILURE);
	} else {
		fprintf(stdout, "\n");
		return (OPERATION_SUCCESS);
	}
}

/* SQL table creation
 *
 * Creates the tags, files, and categories tables
 */
static char *
sql_create_tables(void)
{
	// Create SQL statement
			// tags table
	return  "CREATE TABLE tags("
		"  id		INTEGER PRIMARY KEY	NOT NULL"
		", name		TEXT			NOT NULL"
		");"	// files tables
		"CREATE TABLE files("
		"  id		INTEGER PRIMARY KEY	NOT NULL"
		", filename	TEXT			NOT NULL"
		");"	// categories tables
		"CREATE TABLE categories("
		"  id		INTEGER PRIMARY KEY	NOT NULL"
		", name		TEXT			NOT NULL"
		");"	// tags-files tables
		"CREATE TABLE tags_files("
		"  tags_id	INTEGER			NOT NULL"
		", files_id	INTEGER			NOT NULL"
		", FOREIGN KEY(tags_id)		REFERENCES tags(id)"
		", FOREIGN KEY(files_id)	REFERENCES files(id)"
		");"	// tags-categories tables
		"CREATE TABLE tags_categories("
		"  tags_id		INTEGER		NOT NULL"
		", categories_id	INTEGER		NOT NULL"
		", FOREIGN KEY(tags_id)		REFERENCES tags(id)"
		", FOREIGN KEY(categories_id)	REFERENCES categories(id)"
		");"
		;
}

static int
help_msg(void)
{
	// Prints help message
	fprintf(stdout,
		"Usage:\n"
		"  sftagger [options] <files>\n"
		"\n"
		"Simple file tagging utility.\n"
		"\n"
		"Options:\n"
		"  -n,\tmake a new tag file\n"
		"  -r,\tread the tag file {T,C,F}\n"
		"  -s,\tsearch tag(s)\n"
		"  -a,\treturn all files\n"
		"  -x,\tprogram to execute\n"
		"  -i,\tinsert {t,c,z,f}\n"
		"  -e,\trename {t,c}\n"
		"  -d,\tdelete {t,c,z,f}\n"
		"\n  Requires argument:\n"
		"    -c,\tcategory\t(with insert/rename/delete option)\n"
		"    -t,\ttag\t\t(with insert/rename/delete option)\n"
		"    -z,\ttag-filename\t(with insert/rename/delete option)\n"
		"    -f,\tfilename\t(with insert/rename/delete option)\n"
		"\n  Doesn't require argument:\n"
		"    -C,\tcategory\t(with read option)\n"
		"    -T,\ttag\t\t(with read option)\n"
		"    -F,\tfilename\t(with read option)\n"
		"\n"
		"  -h,\tdisplay this help\n"
		"  -v,\tdisplay version\n"
		"\n"
		"For more details see sftagger(1).\n"
		);

	return (OPERATION_SUCCESS);
}

static int
version(void)
{
	// Prints version
	fprintf(stdout, "Version: %s\n", VERSION);
	return (EXIT_SUCCESS);
}

int
sql_action(	s_args args,
		const char *sql,
		int (*callback)(void *, int, char **, char **),
		void *data
		)
{
	sqlite3		*db;
	char		*z_errmsg = 0;

	// Open SQL database
	if (sql_open(args.db_filename, &db) == OPERATION_FAILURE) {
		return (OPERATION_FAILURE);
	}

	// Execute SQL statement
	sql_exec(db, sql, callback, data, &z_errmsg);

	// Close SQL database
	sqlite3_close(db);

	return (OPERATION_SUCCESS);
}

/* Create file
 *
 */
static int
create_file(s_args args)
{
	return sql_action(args, sql_create_tables(), sql_callback, NULL);
}

int
read_file(s_args args)
{
	char	type[BUF_S];
	char	sql[BUF_M];
	char	*output = (char *) malloc(sizeof(char) * BUF_L);

	switch (args.type) {
	case CATEGORY:	strcpy(type, "categories");	break;
	case TAG:	strcpy(type, "tags");		break;
	case FILENAME:	strcpy(type, "files");		break;
	case TAGFILE:
		fprintf(stderr, "Tag-file not a printable type\n");
		return (OPERATION_FAILURE);
	default:	strcpy(type, "categories, tags, files"); break;
	}

	// Execute SQL SELECT statement
	sprintf(sql, "SELECT * from %s", type);
	sql_action(args, sql, sql_callback, output);
	printf("%s\n", output);

	return (OPERATION_SUCCESS);
}

int
search(s_args args)
{
	return (OPERATION_SUCCESS);
}

int
all(s_args args)
{
	return (OPERATION_SUCCESS);
}

// TODO
int
action_insert(s_args args)
{
	char		*sql;
	char		*str = NULL;
	const char	*delim = " ";
	char		type[BUF_S];
	int		strflag = 0;

	if (!strcmp(args.arg, "")) {
		// Error occurred: No arguments given
		fprintf(stderr, "Error: No arguments given\n");
		return (OPERATION_FAILURE);
	} else {	
		// No error occurred
		sql = (char *) malloc(sizeof(char) * 
				(BUF_L + strlen(args.arg)));
	}

#ifdef DEBUG
	if (args.arg != NULL) {
		printf("args.arg = '%s'\n", args.arg);
	}
#endif

	// Go through the arguments given
	while ((str = strsep(&args.arg, delim)) != NULL) {
		switch (args.type) {
		case CATEGORY:	strcpy(type, "categories");	break;
		case TAG:	strcpy(type, "tags");		break;
		case FILENAME:	strcpy(type, "files");		break;
		case TAGFILE:	// Error: Type not for inserting
			fprintf(stderr, "Tag-file not a printable type\n");
			free(sql);
			return help_msg();
		case NONE:
		default:	// Error: Unknown type
			fprintf(stderr, "No insert type defined\n");
			free(sql);
			return help_msg();
		}

		sprintf(sql, "%sINSERT INTO %s(name) VALUES (\"%s\");"
				, strdup(sql), type, str);
		++strflag;
	}

	if (strflag > 0) {
#ifdef DEBUG
		printf("sql: %s\n", sql);
#endif

		if (sql_action(args, sql, sql_callback, NULL) == OPERATION_FAILURE) {
			free(sql);
			return (OPERATION_FAILURE);
		}

		free(sql);
		return (OPERATION_SUCCESS);
	} else {
		fprintf(stderr, "Error: No insert type defined\n");
		free(sql);
		return (OPERATION_FAILURE);
	}
}

int
action_rename(s_args args)
{
	return (OPERATION_SUCCESS);
}

int
action_delete(s_args args)
{
	return (OPERATION_SUCCESS);
}

// Dummy function
int
dummy(s_args args)
{
	return (OPERATION_FAILURE);
}

int
main(int argc, char **argv)
{
	// Variables
	int		opt;			// Option parsed
	s_opt		opts;			// Option used

	// Default options
	opts.args.type = NONE;			// Type: None
	opts.args.search_execute = 0;		// Search execute flag: 0
	opts.action = HELP;			// Action: Help message
	opts.args.db_filename = FILETARGET;	// Filename: FILETARGET
	opts.func = &dummy;			// Function: dummy function

	// Parse CLI arguments
	while ((opt = getopt(argc, argv, "hvnrsaiedx:Cc:Tt:z:Ff:")) != -1) {
		printf("opt: '%c'\toptopt: '%c'\n", opt, optopt);
		switch (opt) {
		// Action cases
		case 'h': opts.action = HELP;		break;
		case 'v': opts.action = RET_VERSION;	break;
		case 'n': opts.action = NEW;		break;
		case 'r': opts.action = READ;		break;
		case 's': opts.action = SEARCH;		break;
		case 'a': opts.action = ALL;		break;
		case 'i': opts.action = INSERT;		break;
		case 'e': opts.action = RENAME;		break;
		case 'd': opts.action = DELETE;		break;
		// Non action cases
		case 'x':		opts.args.search_execute = 1; 	break;
		case 'c': case 'C': 	opts.args.type = CATEGORY;	break;
		case 't': case 'T': 	opts.args.type = TAG;		break;
		case 'z': 	    	opts.args.type = TAGFILE;	break;
		case 'f': case 'F': 	opts.args.type = FILENAME;	break;
		case '?':	// Error message
			fprintf(stderr, "Error: Argument '%c' not"
					" found\n",
					optopt);
			return help_msg();
		}

		// xctzf to set optarg
		switch (opt) {
		case 'x': case 'c': case 't': case 'z': case 'f':
#ifdef DEBUG
			printf("optarg: %s\n", optarg);
#endif
			opts.args.arg	= optarg;	break;
		case 'C': case 'T': case 'F':
			opts.args.arg	= "";
		}

		// Return early if just a message required
		if (opts.action == HELP) {
			return help_msg();
		} else if (opts.action == RET_VERSION) {
			return version();
		}
	}

	// Default action if no arguments
	if (opts.action == HELP) {
		return help_msg();
	}

	// Non-parsed/extra arguments
	opts.args.outer = (char **) malloc(sizeof(char *) * BUF_M);
	for (int i=0; optind < argc; ++i, ++optind) {
		opts.args.outer[i] = (char *) malloc(sizeof(char) * NAME_MAX);
		strcpy(opts.args.outer[i], argv[optind]);
#ifdef DEBUG
		printf("opts.args.outer[%d]: '%s'\n", i, opts.args.outer[i]);
#endif
	}

	// Set function to be used
	switch (opts.action) {
	case NEW:	opts.func = &create_file;	break;
	case READ:	opts.func = &read_file;		break;
	case SEARCH:	opts.func = &search;		break;
	case ALL:	opts.func = &all;		break;
	case INSERT:	opts.func = &action_insert;	break;
	case RENAME:	opts.func = &action_rename;	break;
	case DELETE:	opts.func = &action_delete;	break;
	default:	// Error message
		fprintf(stderr, "Error: Action not found\n");
	}

	if (opts.args.arg == NULL) {
		opts.args.arg = "";
	}

	// Use operation's function
	if (opts.func(opts.args) == OPERATION_FAILURE) {
		free(opts.args.outer);
		return (EXIT_FAILURE);
	}

	free(opts.args.outer);
	return (EXIT_SUCCESS);
}

