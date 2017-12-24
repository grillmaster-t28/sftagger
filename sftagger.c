/* See LICENSE file for copyright and license details.
 *
 * sftagger - Simple File Tagging tool is a simple but effective file tagger 
 * that just use one file to manage your files, usually in image situations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define BUFFER 256
#define B_BUFFER 1280
#define VERSION "PRE-RELEASE 2017/12/23"

#define CATEGORY 0
#define FILES 1

int str_sort(char string[][BUFFER], int size);
void multi_strtolower(char string[][BUFFER], int size);
void multi_strcpy(char dest[][BUFFER], char src[][BUFFER], int size);
void multi_strclr(char string[][BUFFER], int size);
void multi_strprint(char string[][BUFFER], int size);
void tilldd_strcpy(char *dest, char *src);
int afterdd_strcpytomulti(char dest[][BUFFER], char *src);
int checkifexist(char *file);
void strcpy_wonl(char *dest, char *src);
void strcat_as(char *dest, char *add);

void readfile(void);
void createfile(void);
void add(int argc, char *argv[]);
void addtag(int argc, char *argv[]);
void addcategory(int argc, char *argv[]);
void addtagstofile(int argc, char *argv[]);
void usage(void);

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp(argv[1], "create") == 0)
			createfile();
		else if (strcmp(argv[1], "read") == 0)
			readfile();
		else if (strcmp(argv[1], "version") == 0)
			printf("%s\n", VERSION);
		else if (strcmp(argv[1], "help") == 0)
			usage();
		else if (strcmp(argv[1], "add") == 0)
			add(argc, argv);
	} else
		usage();
	return 0;
}

void readfile(void)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;

	if (fp = fopen("tags", "r")) {
		while (getline(&line, &len, fp) != -1)
			printf("%s", line);
		free(line);
		fclose(fp);
	} else 
		printf("File \"tags\" doesn't exit in current directory\n");
}

void add(int argc, char *argv[])
{
	if (argc > 2) {
		if (strcmp(argv[2], "tag") == 0)
			addtag(argc, argv);
		else if (strcmp(argv[2], "category") == 0)
			addcategory(argc, argv);
		else
			addtagstofile(argc, argv);
	} else
		printf("Error: You must give at least one parameter\n");
}

/* Add tags */
void addtag(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i = 0, j, k, total_tags;
	char *line = NULL;
	size_t len = 0;
	int max;	/* File writing variable */

	int mode = CATEGORY;
	char strcompare[B_BUFFER];
	char strtoformat[B_BUFFER];
	char tagsofcategory[B_BUFFER][BUFFER];

	/* Duplicate checking */
	int duplicate = 0;
	int validcategory = 0;

	if (argc <= 4) {
		printf("Error: You must state at least one tag and a category\n");
		return;
	}
	if (!(checkifexist("tags"))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}
	fp = fopen("tags", "r");
	while (getline(&line, &len, fp) != -1) {
		/* newline changes to file mode */
		if (strcmp(line,"\n") == 0)
			mode = FILES;
		if (mode == CATEGORY) {
			tilldd_strcpy(strcompare, line);
			/* accept if category not there */
			if (strcmp(strcompare, argv[argc-1]) == 0)  {
				validcategory = 1;
				strcpy_wonl(strtoformat, line);
				/* Gets tags (after the double dots) */
				total_tags = afterdd_strcpytomulti(tagsofcategory, line);
				/* Skips duplicate checking if 0 */
				if (total_tags == 0) {
					for (j=3; j<argc-1; j++) 
						strcat_as(strtoformat, argv[j]);
				} else {
					for (j=3; j<argc-1; j++) {
						/* Duplicate checking */
						for (k=0; k<total_tags; k++) {
							if (strcmp(tagsofcategory[k], argv[j]) == 0) {
								printf("Duplicate \"%s\" won't be put in\n", tagsofcategory[k]);
								duplicate = 1;
								break;
							}
						}
						if (duplicate == 0)
							strcat_as(strtoformat, argv[j]);
						duplicate = 0;
					}
				}
				strcpy(flines[i++], strtoformat);
			} else
				strcpy(flines[i++], line);
		} else	/* if FILES */
			strcpy(flines[i++], line);
		/* add in rest */
	}
	free(line);
	fclose(fp);
	max = i;
	/* Write new modification to file */
	if (validcategory == 1) {
		fp = fopen("tags", "w");
		for (i = 0; i < max; i++)
			fprintf(fp, "%s", flines[i]); 
		fclose(fp);
	} else
		printf("The category provided isn't valid\n");
}

int afterdd_strcpytomulti(char dest[][BUFFER], char *src)
{
	int i=0;
	/* Ignoring till colon */
	while (*src++ != ':')
		;
	*src++;
	for (int j=0; (dest[i][j] = *src) != '\0'; j++, *src++) {
		if (*src == ' ' || *src == '\n') {
			dest[i][j] = '\0';
			i++;	/* value reset */
			j = -1;
		}
	}
	return i;	/* Number of tags */
}

/* Concatenate string plus space */
void strcat_as(char *dest, char *add)
{
	while (*dest++ != '\0')
		;
	*--dest = ' ';
	*++dest = ' ';
	while ((*dest++ = *add++) != '\0')
		;
	*--dest = '\0';
}

/* Copy string without newline */
void strcpy_wonl(char *dest, char *src)
{
	while ((*dest++ = *src++) != '\n')
		;
	*--dest = '\0';
}

/* Check if the file exists */
int checkifexist(char *file)
{
	FILE *fp;

	if (fp = fopen(file, "r")) {
		fclose(fp);
		return 1;
	} else
		return 0;
}

/* Add category */
void addcategory(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i = 0, max, j, k = 0, m;
	char *line = NULL;
	size_t len = 0;

	char newcategory[B_BUFFER];

	/* Duplication checking variables */
	int skipdupverif = 0;
	int duplicate[BUFFER];
	char strcompare[B_BUFFER];
	int skip = 0;

	if (argc <= 3) {
		printf("Error: You must state the category\n");
		return;		/* Terminates if no category stated */
	}
	if (!(checkifexist("tags"))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}
	fp = fopen("tags", "r");
	/* Reading each line */
	while (getline(&line, &len, fp) != -1) {
		if (strcmp("\n", line) == 0) {
			/* Adding new categories */
			for (j=3; j<argc; j++) {
				/* Checking for duplicates */
				for (m=0; m<k; m++) 	
					if (duplicate[m] == j)  {
						skip = 1;
						printf("Duplicate: \"%s\""
							" won't be added in\n",
							argv[j]);
						break;
					}
				/* Non-duplicate/Accepted */
				if (skip == 0) {
					strcpy(newcategory, argv[j]);
					strcat(newcategory, ":\n");
					strcpy(flines[i++], newcategory);
					printf("New Category: %s\n", argv[j]);
				}
				skip = 0;
			}
			strcpy(flines[i++], "\n");
			skipdupverif = 1;
		} else {	/* Copying over per line */
			if (skipdupverif == 0) {	/* Categories lines */
				tilldd_strcpy(strcompare, line);
				for (j=3; j<argc; j++) {
					if (strcmp(strcompare, argv[j]) == 0) {
						duplicate[k++] = j;
						break;
					}
				}
			}
			strcpy(flines[i++], line);	/* All lines */
		}
	}	
	free(line);
	fclose(fp);
	max = i;
	/* Write new modification to file */
	fp = fopen("tags", "w");
	for (i = 0; i < max; i++)
		fprintf(fp, "%s", flines[i]); 
	fclose(fp);
}

void tilldd_strcpy(char *dest, char *src)
{
	while ((*dest++ = *src++) != ':')
		;
	*--dest = '\0';
}

void addtagstofile(int argc, char *argv[])
{
	/* Add tag(s) to a file (or more) */
	return;
}

/* Creates a new tags file */
void createfile(void)
{
	/* File writing */
	FILE *fp;

	/* Directory reading */
	DIR *dp;
	struct dirent *ep;

	/* Checking */
	int matches;
	int choice, answer;

	/* sorting */
	char ep_dname_ls[B_BUFFER][BUFFER];
	int itr, upto;

	/* Check if file exists */
	if (fp = fopen("tags", "r")) {
		fclose(fp);
		fputs("File already exist, overwrite? [y/n] ", stdout);
		choice = tolower(fgetc(stdin));
		answer = choice;
		/* Discard rest of input line */
		while (choice != '\n')
			choice = fgetc(stdin);
		if (answer != 'y') 
			return;		// Terminate file creation 
		printf("Overwrite\n");
	} 

	fp = fopen("tags", "w+");	/* Make file "tags" */
	dp = opendir("./");
	if (dp != NULL) {
		itr = 0;
		while (ep = readdir(dp)) 
			strcpy(ep_dname_ls[itr++], ep->d_name);
		upto = str_sort(ep_dname_ls, itr);
		/* Category placeholder */
		fprintf(fp, "category: placeholder\n\n");
		for (itr = 0; itr < upto; itr++) 
			fprintf(fp, "%s\n", ep_dname_ls[itr]);
		(void) closedir(dp);
	} else
		perror("Couldn't open current directory");
	
	fclose(fp);
}

int str_sort(char string[][BUFFER], int or_size)
{
	int i, j, matches;
	char temp[BUFFER];
	char lowerstr[B_BUFFER][BUFFER], newstring[B_BUFFER][BUFFER];
	for (i=0,j=0; i<or_size; i++) {
		/* file not . or itself */
		matches = strcmp(string[i], "tags");
		if (string[i][0] != '.' && matches != 0) 
			strcpy(newstring[j++], string[i]);
	}
	int size = j;
	multi_strclr(string, size);
	multi_strcpy(string, newstring, size);
	multi_strcpy(lowerstr, string, size);
	multi_strtolower(lowerstr, size);
	/* Multi string sorting */
	for (i=0; i<size; i++) 
		for (j=0; j<size-1; j++) 
			/* Swap */
			if (strcmp(lowerstr[j], lowerstr[j+1]) > 0) {
				strcpy(temp, string[j]);
				strcpy(string[j], string[j+1]);
				strcpy(string[j+1], temp);
				strcpy(temp, lowerstr[j]);
				strcpy(lowerstr[j], lowerstr[j+1]);
				strcpy(lowerstr[j+1], temp);
			}
	return size;
}

void multi_strtolower(char string[][BUFFER], int size)
{
	int i, j;
	for (i=0; i<size; i++)
		for (j=0; j<sizeof(string[i]); j++)
			string[i][j] = tolower(string[i][j]);
}

void multi_strcpy(char dest[][BUFFER], char src[][BUFFER], int size)
{
	for (int i=0; i<size; i++)
		strcpy(dest[i], src[i]);
}

void multi_strclr(char string[][BUFFER], int size)
{
	for (int i=0; i<size; i++)
		string[i][0] = '\0';
}

/* Prints multiple of string */
void multi_strprint(char string[][BUFFER], int size)
{
	putchar('\n');
	for (int i=0; i<size; i++)
		printf("%s\n", string[i]);
}

void usage(void)
{
	printf("usage: {create | read | version | help | add {category | tag}}\n");
}

