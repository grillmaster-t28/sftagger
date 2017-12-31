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

#define S_BUFFER 64
#define BUFFER 256
#define B_BUFFER 1280

#define VERSION "1.0 Release - 2017/12/31"
#define FILETARGET "tags"

enum {
	CATEGORY,
	FILES,
	FALSE = 0,
	TRUE,
	CATS = 0,
	TAGS,
	CATS_TAGS
};

int str_sort(char string[][BUFFER], int size, int type);
void multi_strtolower(char string[][BUFFER], int size);
void multi_strcpy(char dest[][BUFFER], char src[][BUFFER], int size);
void multi_strclr(char string[][BUFFER], int size);
void multi_strprint(char string[][BUFFER], int size);
void tilldd_strcpy(char *dest, char *src);
int afterddff_strcpytomulti(char dest[][BUFFER], char *src, int mode);
int checkifexist(char *filename);
void strcpy_wonl(char *dest, char *src);
void strcat_as(char *dest, char *add);
void getfname(char *dest, char *src);
int gettags_wfilt(int numtags[BUFFER], char alltags[][BUFFER], int ctagstotal);
int * intcpy(int *src, size_t len);
int hasspace(char *src);
int dirtomstr(char *dirname, char dest[][BUFFER]);
void writefile(char flines[][BUFFER], int max);
void rmdubspaces(char *dest, char *src);
int checkdup(char src_a[][BUFFER], char *src_b, int a_limit);
void getchange(int *min, int *addby, int max, char *argv[]);
void rm_rejects(char *src, int limit, char *src_b, char reject[][BUFFER]);

void readfile(void);
void createfile(void);
void updatefile(void);
void add(int argc, char *argv[]);
void addtags(int argc, char *argv[]);
void addcategory(int argc, char *argv[]);
void addtagstofile(int argc, char *argv[]);
void searchtags(int argc, char *argv[]);
void list(int argc, char *argv[]);
void listcattags(int argc, char *argv[], int type);
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
		else if (strcmp(argv[1], "update") == 0)
			updatefile();
		else if (strcmp(argv[1], "search") == 0)
			searchtags(argc, argv);
		else if (strcmp(argv[1], "list") == 0)
			list(argc, argv);
		else
			usage();
	} else {
		usage();
	}
	return 0;
}

void add(int argc, char *argv[])
{
	if (argc > 2) {
		if (strcmp(argv[2], "tags") == 0)
			addtags(argc, argv);
		else if (strcmp(argv[2], "category") == 0)
			addcategory(argc, argv);
		else if (strcmp(argv[2], "tags-to") == 0)
			addtagstofile(argc, argv);
		else
			printf("Error: You must give: {tags | category |"
					" tags-to}\n");
	} else {
		printf("Error: You must give at least one parameter: {tags"
				" | category | tags-to}\n");
	}
}

void list(int argc, char *argv[])
{
	if (argc > 2) {
		if (strcmp(argv[2], "categories") == 0)
			listcattags(argc, argv, CATS);
		else if (strcmp(argv[2], "tags-of") == 0)
			listcattags(argc, argv, TAGS);
		else if (strcmp(argv[2], "all") == 0)
			listcattags(argc, argv, CATS_TAGS);
		else {
			printf("Error: You must give: {categories | tags-of |"
					" all}\n");
		}
	} else {
		printf("Error: You must give at least one parameter:"
				" {categories | tags-of | all}\n");
	}
}

void listcattags(int argc, char *argv[], int type)
{
	FILE *fp;
	char line[B_BUFFER];

	char category[B_BUFFER];
	int i;

	fp = fopen(FILETARGET, "r");
	if (fp == NULL) {
		printf("File \"tags\" doesn't exit in current directory\n");
		return;
	}
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			break;
		tilldd_strcpy(category, line);
		switch (type) {
		case CATS:
			printf("%s ", category);
			break;
		case TAGS:
			for (i=3; i<argc; i++)
				if (strcmp(category, argv[i]) == 0) {
					printf("%s", line);
					break;
				}
			break;
		case CATS_TAGS:
			printf("%s", line);
			break;
		default:
			printf("Debug: listcattags: Wrong type used\n");
			return;
		}
		memset(line, 0, sizeof line);
	}
	if (type == CATS)
		putchar('\n');
	fclose(fp);
}

void readfile(void)
{
	FILE *fp;
	char line[B_BUFFER];

	fp = fopen(FILETARGET, "r");
	if (fp == NULL) {
		printf("File \"tags\" doesn't exit in current directory\n");
		return;
	}
	while (fgets(line, B_BUFFER, fp) != NULL) {
		printf("%s", line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);
}

void searchtags(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char slines[B_BUFFER][BUFFER];
	int i = 0, j, k = 0;
	char line[B_BUFFER];
	int mode = CATEGORY;

	/* Tags filtering */
	int maxtags;
	int ftagsnums[BUFFER];
	char ftagsnames[B_BUFFER][BUFFER];

	/* In-line variables */
	int ltagstotal = 0;
	char linetags[B_BUFFER][BUFFER];
	char str_num[BUFFER];
	int filematches = 0;

	if (argc <= 2) {
		printf("Error: You must state at least one tag\n");
		return;
	}
	if (!(checkifexist(FILETARGET))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	for (i=0; i<argc-2; i++)
		strcpy(ftagsnames[i], argv[i+2]);
	/* Get and exclude tags in file as well as assign to number */
	maxtags = gettags_wfilt(ftagsnums, ftagsnames, argc-2);
	if (maxtags == 0) {
		printf("Error: No valid tag given\n");
		return;
	}

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
			goto reset_line;
		} else if (mode == CATEGORY) {
			goto reset_line;
		}
		ltagstotal = afterddff_strcpytomulti(linetags, line, 1);
		if (ltagstotal == 0)
			goto reset_line;

		filematches = 0;
		for (i=0; i<ltagstotal; i++) {
			for (j=0; j<maxtags; j++) {
				sprintf(str_num, "%d", ftagsnums[j]);
				if (strcmp(linetags[i], str_num) == 0) {
					filematches++;
					break;
				}
			}
		}
		if (filematches == maxtags)
			getfname(slines[k++], line);
reset_line:
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	if (k == 0) {
		printf("Notice: No files found with the tags given\n");
		return;
	}

	/* Outputs out result of the search */
	for (i=0; i<k; i++) {
		if (hasspace(slines[i])) {
			printf("\"%s\" ", slines[i]);
		} else
			printf("%s ", slines[i]);
	}
	putchar('\n');
}

void getchange(int *min, int *addby, int max, char *argv[])
{
	FILE *fp;
	char line[B_BUFFER];

	int j, k, l=0, m=0;
	int needmod = 0;	/* If it need modification */
	char strcompare[B_BUFFER];
	char linetags[B_BUFFER][BUFFER];
	int maxtags;

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			break;
		tilldd_strcpy(strcompare, line);
		maxtags = afterddff_strcpytomulti(linetags, line, 0);
		if (needmod == 1) {
			for (j=0; j<maxtags; m++, l++, j++) {
				for (k=3; k<max; k++) {
					if (strcmp(argv[k], linetags[j]) == 0)
						break;
				}
				if (k == max) {	/* Not a duplicate */
					*min = m;
					*addby = l-m;
					return;		/* End variable */
				} else
					l--;
			}
		} else {
			for (j=0; j<maxtags; j++) {
				for (k=3; k<max; k++) {
					if (strcmp(argv[k], linetags[j]) == 0)
						break;
				}	/* Ignore non edited categories */
				if (k == max) {
					m++;
					l++;
				} else
					l--;
			}
		}
		if (strcmp(strcompare, argv[max]) == 0) {
			needmod = 1;
			for (j=3; j<max; l++, j++)
				;	/* Ignore adding tags */
		}
		memset(line, 0, sizeof line);
	}
}

/* Duplicate checking */
int checkdup(char src_a[][BUFFER], char *src_b, int a_limit)
{
	for (int i=0; i<a_limit; i++) {
		if (strcmp(src_a[i], src_b) == 0)
			return i;
	}
	return 0;
}

/* Add tags */
void addtags(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i = 0, j, k, l;
	char line[B_BUFFER];

	int mode = CATEGORY;
	char strcompare[B_BUFFER];
	char strtoformat[B_BUFFER];
	char linetags[B_BUFFER][BUFFER];

	/* Duplicate checking */
	int dup = 0;
	int validcategory = 0;
	int maxtags;

	char reject[B_BUFFER][BUFFER];
	int t=0, dubcheck=0;
	char strtoformat2[B_BUFFER];
	char strtoformat3[B_BUFFER];
	int strdiff;

	/* Numbers modification */
	char linenums[B_BUFFER][BUFFER];
	char numstrnew[S_BUFFER];

	/* Number change */
	int min, addby;
	addby = min = -1;
	int tagnum;

	if (argc <= 4) {
		printf("Error: You must state at least one tag and a"
				" category\n");
		return;
	}
	if (!(checkifexist(FILETARGET))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	getchange(&min, &addby, argc-1, argv);

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		/* newline changes to file mode */
		maxtags = 0;
		if (strcmp(line,"\n") == 0) {
			mode = FILES;
		} else if (mode == CATEGORY) {
			tilldd_strcpy(strcompare, line);
			maxtags = afterddff_strcpytomulti(linetags, line, 0);
		}
		strdiff = strcmp(strcompare, argv[argc-1]);
		if (mode == CATEGORY && strdiff == 0) {
			validcategory = 1;
			strcpy_wonl(strtoformat, line);
			for (j=3; j<argc-1; j++, dup=0) {
				if (maxtags > 0) {
					dup = checkdup(linetags, argv[j], 
							maxtags);
				}
				if (dup == 0 || maxtags == 0)
					strcat_as(strtoformat, argv[j]);
				else if (dup > 0) {
					printf("Duplicate \"%s\" won't be put"
							" in\n", 
							linetags[dup]);
				}
			}
			dubcheck = i;
			strcat(strtoformat, "\n");
			strcpy(flines[i++], strtoformat);
			memset(line, 0, sizeof line);
			continue;
		} else if (mode == CATEGORY && strdiff != 0) {
			for (j=3; j<argc-1; j++, dup=0) {
				dup = checkdup(linetags, argv[j], maxtags);
				if (dup > 0) {
					strcpy(reject[t++], argv[j]);
					printf("Duplicate \"%s\" won't be put"
							" in\n", 
							linetags[dup]);
				}
			}
		} else if (mode == FILES) {
			maxtags = afterddff_strcpytomulti(linenums, line, 1);
			if (maxtags == 0 || min == -1)
				goto skip_to_end_addtags;
			l = 0;
			getfname(strtoformat2, line);
			strcat(strtoformat2, " ");
			for (k=0; k<maxtags; k++) {
				tagnum = atoi(linenums[k]);
				if (tagnum >= min) {
					l++;
					sprintf(numstrnew, " %d", 
							tagnum+addby);
				} else
					sprintf(numstrnew, " %s", linenums[k]);
				strcat(strtoformat2, numstrnew);
			}
			if (l == 0) {	/* If no change needed */
				strcpy(flines[i++], line);
			} else {
				strcat(strtoformat2, "\n");
				rmdubspaces(strtoformat3, strtoformat2);
				strcpy(flines[i++], strtoformat3);
				memset(strtoformat3, 0, sizeof strtoformat3);
			}
			memset(strtoformat2, 0, sizeof strtoformat2);
			memset(line, 0, sizeof line);
			continue;
		}
skip_to_end_addtags:
		strcpy(flines[i++], line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);

	if (validcategory == 0) {
		printf("The category provided isn't valid\n");
		return; 
	}

	rm_rejects(flines[dubcheck], t, strtoformat, reject);
	writefile(flines, i);
}

/* Remove the rejects */
// CHECK src and src_b
void rm_rejects(char *src, int limit, char *src_b, char reject[][BUFFER])
{
	char strtoformat[B_BUFFER], strtocat[B_BUFFER];
	char linetags[B_BUFFER][BUFFER];
	int linechanged = 0;
	int maxtags, j;

	tilldd_strcpy(strtoformat, src);
	strcat(strtoformat, ":");
	for (int i=0; i<limit; i++) {
		maxtags = afterddff_strcpytomulti(linetags, src_b, 0);
		for (j=0; j<maxtags; j++) {
			if (strcmp(linetags[j], reject[i]) == 0) {
				linechanged = 1;
				break;
			}
			strcpy(strtocat, " ");
			strcat(strtocat, linetags[j]);
			strcat(strtoformat, strtocat);
		}
	}
	if (linechanged == 1) {
		strcat(strtoformat, "\n");
		strcpy(src, strtoformat);
	}
}

/* Remove extra white-spaces */
void rmdubspaces(char *dest, char *src)
{
	char prev = ' ';
	while (*src != '\0') {
		if (*src != prev || prev != ' ')
			*dest++ = *src;
		prev = *src++;
	}
	*dest = '\0';
}

int afterddff_strcpytomulti(char dest[][BUFFER], char *src, int mode)
{
	int i=0;
	char ignoretill = ':';		/* Mode 0 */

	/* Ignoring till colon */
	if (mode == 1) {
		if (*src == '"') {
			ignoretill = '"';
			src++;
		} else
			ignoretill = ' ';
	}
	while (*src != ignoretill && *src != '\n') {
		if (*src == '\n')
			return 0;	/* There will be no tags so early 0 */
		src++;
	}
	if (ignoretill == ':' || ignoretill == '"')
		src++;
	src++;
	for (int j=0; (dest[i][j] = *src) != '\0'; j++, *src++) {
		if (*src == ' ' || *src == '\n') {
			dest[i][j] = '\0';
			i++;	/* value reset */
			if (*src == '\n')
				break;
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
int checkifexist(char *filename)
{
	FILE *fp;

	if ((fp = fopen(filename, "r")) != NULL) {
		fclose(fp);
		return 1;
	}
	return 0;
}

/* Add category */
void addcategory(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i = 0, j, k = 0;
	char line[B_BUFFER];

	/* Duplication checking variables */
	int mode = CATEGORY;
	char dups[S_BUFFER][BUFFER];
	char str_j[S_BUFFER];
	char strcompare[B_BUFFER];
	int skip = 0;

	if (argc <= 3) {
		printf("Error: You must state the category\n");
		return;		/* Terminates if no category stated */
	}
	if (!(checkifexist(FILETARGET))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}
	fp = fopen(FILETARGET, "r");
	/* Reading each line */
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp("\n", line) == 0) {
			for (j=3; j<argc; j++) {
				sprintf(str_j, "%d", j);
				skip = checkdup(dups, str_j, k);
				/* Adding new category */
				if (skip == 0) {
					strcpy(flines[i], argv[j]);
					strcat(flines[i++], ":\n");
					printf("New Category: %s\n", argv[j]);
				} else {
					printf("Duplicate: \"%s\" won't be"
							" added in\n",
							argv[j]);
				}
				skip = 0;
			}
			mode = FILES;
		} else if (mode == CATEGORY) {
			tilldd_strcpy(strcompare, line);
			for (j=3; j<argc; j++) {
				if (strcmp(strcompare, argv[j]) == 0) {
					sprintf(dups[k++], "%d", j);
					break;
				}
			}
		}
		strcpy(flines[i++], line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	writefile(flines, i);
}

void tilldd_strcpy(char *dest, char *src)
{
	while ((*dest++ = *src++) != ':')
		;
	*--dest = '\0';
}

/* Add tag(s) to a file (or more) */
void addtagstofile(int argc, char *argv[])
{
	/* Let UNIX/UNIX-like system handle the wildcards */

	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i = 0, j, k, l;
	char line[B_BUFFER];

	char filenameofline[B_BUFFER];
	int mode = CATEGORY;

	/* Tags to add */
	char tagstoadd[B_BUFFER][BUFFER];
	int c, g=0, h=0;
	int tagsnums[BUFFER];

	/* Tags exclusion */
	int tags_amount;

	/* In-line variables */
	char newline[B_BUFFER];
	char newlinefixed[B_BUFFER];
	char str_num[S_BUFFER];

	/* Duplication checking variables */
	char linetags[B_BUFFER][BUFFER];
	int ltagsmax = 0;
	int *ltagsnums;
	int strdiff;

	if (argc <= 3) {
		printf("Error: You must state at least one filename\n");
		return;
	}
	if (!(checkifexist(FILETARGET))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	printf("Add tags you want to insert, split them by space, "
			"enter when done:\n");
	/* Adds tags to list */
	while ((c = getchar()) != '\n') {
		if (c == ' ') {
			tagstoadd[g][h] = '\0';
			g++;
			h = 0;
		} else
			tagstoadd[g][h++] = c;
	}
	g++;

	/* Get and exclude tags in file as well as assign to number */
	tags_amount = gettags_wfilt(tagsnums, tagstoadd, g);
	if (tags_amount == -1) {		/* Read error */
		printf("Error: Fail reading file\n");
		return;
	} else if (tags_amount == 0) {		/* No valid tags error */
		printf("Error: No valid tags, add them in before you apply"
				" them\n");
		return;
	}

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
			goto skip_to_end_cat;
		} else if (mode == CATEGORY) {
			goto skip_to_end_cat;
		}
		getfname(filenameofline, line);
		for (j=3; j<argc; j++) {
			if (strcmp(argv[j], filenameofline) != 0)
				continue;
			printf("FOUND: \"%s\"\n", filenameofline);
			ltagsnums = intcpy(tagsnums, tags_amount);
			ltagsmax = afterddff_strcpytomulti(linetags, line, 1);
			strcpy_wonl(newline, line);
			for (k=0; k<tags_amount; k++) {
				/* Duplication checking */
				for (l=0; l<ltagsmax; l++) {
					sprintf(str_num, "%d", ltagsnums[k]);
					strdiff = strcmp(str_num, linetags[l]);
					if (strdiff == 0) {
						ltagsnums[k] = -1;
						break;
					}
				}
				if (ltagsnums[k] != -1) {
					sprintf(str_num, " %d", ltagsnums[k]);
					strcat(newline, str_num);
				}
			}
			strcat(newline, "\n");
			rmdubspaces(newlinefixed, newline);
			strcpy(flines[i++], newlinefixed);
			memset(newline, 0, sizeof newline);
			memset(newlinefixed, 0, sizeof newlinefixed);
			goto skip_to_linereset_cat;
		}
skip_to_end_cat:
		strcpy(flines[i++], line);
skip_to_linereset_cat:
		memset(line, 0, sizeof line);
	}

	fclose(fp);
	writefile(flines, i);
}

/* Assigns tags numbers and filters current tags */
int gettags_wfilt(int numtags[BUFFER], char curtags[][BUFFER], int ctagstotal)
{
	FILE *fp;
	char line[B_BUFFER];

	char linetags[B_BUFFER][BUFFER];	/* Tags compared to per line */
	char allowedtags[B_BUFFER][BUFFER];	/* Tags been allowed */
	int i, j, k=0, n=0;
	int ltagstotal = 0;		/* Max item value of in-line tags */

	if (!(checkifexist(FILETARGET))) 
		return -1;		/* File reading error */

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			break;		/* End of tags reading */
		ltagstotal = afterddff_strcpytomulti(linetags, line, 0);
		for (j=0; j<ltagstotal; j++, n++) { 
			for (i=0; i<ctagstotal; i++) {
				if (strcmp(curtags[i], linetags[j]) == 0) {
					strcpy(allowedtags[k], curtags[i]);
					numtags[k++] = n;
				}
			}
		}
		memset(line, 0, sizeof line);
	}

	/* Assigning the allowed tags to current tags */
	for (i=0; i<ctagstotal; i++) {
		curtags[i][0] = '\0';
		if (i<k)
			strcpy(curtags[i], allowedtags[i]);
	}

	return k;
}

/* Gets the filename of its line */
void getfname(char *dest, char *src)
{
	int spacedname = 0;

	while ((*dest = *src++) != '\n') {
		if (*dest == '"')
			spacedname = !spacedname;
		else if (spacedname == 0 && *dest == ' ')
			break;
		else 
			dest++;
	}
	*dest = '\0';
}

/* Updates the file
 * NOTE: Does not take away deleted files, just adds in new ones
 */
void updatefile(void)
{
	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i=0, max, j=0;
	char line[B_BUFFER];

	int mode = CATEGORY;

	/* Directory variables */
	int newmax = 0;
	int oldmax = 0;
	char updatedls[B_BUFFER][BUFFER];
	char oldls[B_BUFFER][BUFFER];
	char qmoldls[BUFFER];

	/* Sorting variables */
	int out;

	if (!(checkifexist(FILETARGET))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	fp = fopen(FILETARGET, "r");
	newmax = dirtomstr("./", updatedls);
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			mode = FILES;
		else if (mode == FILES) {
			getfname(oldls[j], line);
			if (hasspace(oldls[j])) {
				sprintf(qmoldls, "\"%s\"", oldls[j]);
				strcpy(oldls[j], qmoldls);
			}
			j++;
		}
		strcpy(flines[i++], line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	max = i;
	oldmax = j;
	/* Ruling out not new files */
	for (i=0; i<newmax; i++)
		for (j=0; j<oldmax; j++) 
			if (strcmp(updatedls[i], oldls[j]) == 0)
				strcpy(updatedls[i], ".");	/* Not new */
	/* Putting in new files */
	for (i=0; i<newmax; i++)
		if (strcmp(updatedls[i], ".") != 0) {		/* Is new */
			strcat(updatedls[i], "\n");
			strcpy(flines[max++], updatedls[i]);
		}
	/* Sorts the file strings */
	out = str_sort(flines, max, 1);
	if (out == -1) {
		printf("Error: Unable to sort (can't find a newline in between"
				" the categories and filenames)\n");
		return;
	}
	writefile(flines, max);
}

/* Write new modification to file */
void writefile(char flines[][BUFFER], int max)
{
	FILE *fp;

	fp = fopen(FILETARGET, "w");
	for (int i = 0; i < max; i++)
		fprintf(fp, "%s", flines[i]); 
	fclose(fp);
}

/* Creates a new tags file */
void createfile(void)
{
	/* File writing */
	FILE *fp;

	/* Checking */
	int ans;

	/* sorting */
	char ep_dname_ls[B_BUFFER][BUFFER];
	int itr, upto;

	/* Check if file exists */
	if ((fp = fopen(FILETARGET, "r")) != NULL) {
		fclose(fp);
		printf("File already exist, overwrite? [y/n] ");
		ans = tolower(getchar());
		while (getchar() != '\n')
			;	/* Discard rest of input line */
		if (ans != 'y') 
			return;		// Terminate file creation 
		printf("Overwrite\n");
	} 

	itr = dirtomstr("./", ep_dname_ls);
	if (itr == -1) {
		printf("Error: Couldn't open current directory\n");
		return;
	}
	upto = str_sort(ep_dname_ls, itr, 0);
	fp = fopen(FILETARGET, "w+");		/* Make file "tags" */
	fprintf(fp, "category: placeholder\n\n");
	for (itr = 0; itr < upto; itr++) 
		fprintf(fp, "%s\n", ep_dname_ls[itr]);
	fclose(fp);
}

int dirtomstr(char *dirname, char dest[][BUFFER])
{
	/* Directory reading */
	DIR *dp;
	struct dirent *ep;
	int itr = 0;

	dp = opendir(dirname);
	if (dp == NULL)
		return -1;
	while ((ep = readdir(dp)) != NULL) {
		if (hasspace(ep->d_name))
			sprintf(dest[itr++], "\"%s\"", ep->d_name);
		else if (ep->d_name[0] != '.' && /* File not . or itself */
				strcmp(ep->d_name, FILETARGET) != 0)
			strcpy(dest[itr++], ep->d_name);
	}
	(void) closedir(dp);
	return itr;
}

int str_sort(char string[][BUFFER], int size, int type)
{
	int i, j, min = 0;
	char temp[BUFFER];
	char lowerstr[B_BUFFER][BUFFER];
	if (type == 1) {		/* Has full file strings */
		for (i=0; i<size; i++)
			if (strcmp(string[i], "\n") == 0)
				break;
		if (i >= size-1)
			return -1;	/* Error */
		min = i;
	}
	multi_strcpy(lowerstr, string, size);
	multi_strtolower(lowerstr, size);
	/* Multi string sorting */
	for (i=min; i<size; i++) 
		for (j=min; j<size-1; j++) 
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

int *intcpy(int *src, size_t len)
{
	int *dest = malloc(len * sizeof(int));
	memcpy(dest, src, len * sizeof(int));
	return dest;
}

int hasspace(char *src)
{
	while (*src != '\0')
		if (*src++ == ' ')
			return 1;
	return 0;
}

void usage(void)
{
	printf("usage: {create | read | version | help | search | "
			"add {category | tags | tags-to} | list "
			"{categories | tags-of | all}}\n");
}

