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

// enum declarations
typedef enum action_enum {
	HELP, RET_VERSION, NEW, READ, SEARCH, ALL, INSERT, RENAME
	, DELETE
} e_action;

typedef enum action_type_enum {
	CATEGORY, TAG, TAGFILE, FILENAME
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
sql_callback(void *not_used, int argc, char **argv, char **az_colname)
{
	for (int i=0; i < argc; ++i) {
		printf("%s = %s\n", az_colname[i], argv[i] ? argv[i] : "NULL");
	}
	putchar('\n');

	return 0;
}

/* SQL sqlite3 SELECT callback function
 *
 */
static int
sql_select(void *data, int argc, char **argv, char **az_colname)
{
	fprintf(stderr, "%s: ", (const char *) data);
	for (int i=0; i < argc; ++i) {
		printf("%s = %s\n", az_colname[i], argv[i] ? argv[i] : "NULL");
	}
	putchar('\n');

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
		fprintf(stdout, "Databased opened\n");
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
		fprintf(stderr, "SQL error: %s\n", *z_errmsg);
		sqlite3_free(*z_errmsg);
		return (OPERATION_FAILURE);
	} else {
		fprintf(stdout, "SQL executed\n");
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
		", tags_id	INTEGER"
		", FOREIGN KEY(tags_id)	REFERENCES tags(id)"
		");"	// categories tables
		"CREATE TABLE categories("
		"  id		INTEGER PRIMARY KEY	NOT NULL"
		", name		TEXT			NOT NULL"
		", tags_id	INTEGER"
		", FOREIGN KEY(tags_id)	REFERENCES tags(id)"
		");"
		;
}

int
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
		"  -r,\tread the tag file\n"
		"  -s,\tsearch tag(s)\n"
		"  -a,\treturn all files\n"
		"  -x,\tprogram to execute\n"
		"  -i,\tinsert {t,c,r,f}\n"
		"  -e,\trename {t,c}\n"
		"  -d,\tdelete {t,c,r,f}\n"
		"\n"
		"  -c,\tcategory\n"
		"  -t,\ttag\n"
		"  -z,\ttag-filename\n"
		"  -f,\tfilename\n"
		"\n"
		"  -h,\tdisplay this help\n"
		"  -v,\tdisplay version\n"
		"\n"
		"For more details see sftagger(1).\n"
		);

	return (EXIT_SUCCESS);
}

int
version(void)
{
	// Prints version
	fprintf(stdout, "Version: %s\n", VERSION);
	return (EXIT_SUCCESS);
}

int
sql_action(	s_args args,
		const char *sql,
		int (*callback)(void *, int, char **, char **)
		)
{
	sqlite3		*db;
	char		*z_errmsg = 0;

	// Open SQL database
	if (sql_open(args.db_filename, &db) == OPERATION_FAILURE) {
		return (OPERATION_FAILURE);
	}

	// Execute SQL statement
	sql_exec(db, sql, callback, 0, &z_errmsg);

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
	return sql_action(args, sql_create_tables(), sql_callback);
}

int
read_file(s_args args)
{
	switch (args.type) {
	case CATEGORY:
		return sql_action(args, "SELECT * from categories",
				sql_select);
	case TAG:
		return sql_action(args, "SELECT * from tags", sql_select);
	case FILENAME:
		return sql_action(args, "SELECT * from files", sql_select);
	case TAGFILE:
		fprintf(stderr, "Tag-file not a printable type\n");
	default:
		return sql_action(args,
				"SELECT * from categories, tags, files",
				sql_select);
	}
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

int
action_insert(s_args args)
{
	char	*sql = NULL;
	//args.arg;	// TODO

	for (int i=0; i < 0; ++i) {
		switch (args.type) {
		case CATEGORY:
			sql = 	"INSERT INTO categories(id, name)"
				"VALUES (1, %s);";
		case TAG:
			sql =	"INSERT INTO tags(id, name)"
				"VALUES (1, %s);";
		case FILENAME:
			sql =	"INSERT INTO files(id, name)"
				"VALUES (1, %s);";
		case TAGFILE:
			fprintf(stderr, "Tag-file not a printable type\n");
			return help_msg();
		default:
			fprintf(stderr, "No insert type defined");
			return help_msg();
		}

		if (sql_action(args, sql, sql_select) == OPERATION_FAILURE) {
			return OPERATION_FAILURE;
		}
	}

	fprintf(stdout, "Operation completed\n");
	return OPERATION_SUCCESS;
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
	opts.args.search_execute = 0;		// Search execute flag: 0
	opts.action = HELP;			// Action: Help message
	opts.args.db_filename = FILETARGET;	// Filename: FILETARGET
	opts.func = &dummy;			// Function: dummy function

	// Parse CLI arguments
	while ((opt = getopt(argc, argv, "hvnrsaiedx:c::t::z:f::")) != -1) {
		switch (opt) {
		case 'h': opts.action = HELP;		break;
		case 'v': opts.action = RET_VERSION;	break;
		case 'n': opts.action = NEW;		break;
		case 'r': opts.action = READ;		break;
		case 's': opts.action = SEARCH;		break;
		case 'a': opts.action = ALL;		break;
		case 'i': opts.action = INSERT;		break;
		case 'e': opts.action = RENAME;		break;
		case 'd': opts.action = DELETE;		break;
		// xctzf options to fall through to set optarg
		case 'x':
			opts.args.search_execute = 1;
			opts.args.arg	= optarg;	break;
		case 'c':
			opts.args.type	= CATEGORY;
			opts.args.arg	= optarg;	break;
		case 't':
			opts.args.type	= TAG;
			opts.args.arg	= optarg;	break;
		case 'z':
			opts.args.type	= TAGFILE;
			opts.args.arg	= optarg;	break;
		case 'f':
			opts.args.type	= FILENAME;
			opts.args.arg	= optarg;	break;
		default:	// Error message
			fprintf(stderr, "Error: Argument '%c' not found\n",
					opt);
			return help_msg();
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
	opts.args.outer = (char **) calloc(BUF_M, sizeof(char) * NAME_MAX);
	for (int i=0; optind < argc; ++i, ++optind) {
		strcpy(opts.args.outer[i], argv[optind]);
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
	
	// Use operation's function
	if (opts.func(opts.args) == OPERATION_FAILURE) {
		free(opts.args.outer);
		return (EXIT_FAILURE);
	}

	free(opts.args.outer);
	return (EXIT_SUCCESS);
}

