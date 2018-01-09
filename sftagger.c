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

#define VERSION "2.0 RC3 - 2018/01/09"
#define FILETARGET "tags-dev"

enum {
	CATEGORY, FILES,
	FALSE = 0, TRUE,
	CAT_NAME = 0, TAG_NAME,
	ADD = 0, RENAME, REMOVE
};

static const char *usage_error[] = {
	"{category | tags | tags-to}",
	"{category | tag}",
	"{category | tags | from-tags | files}"
};

int specialchar(char c)
{
	if ((c > 31 && c < 48) || (c > 58 && c < 65) || (c > 90 && c < 97) ||
			(c > 122))
		return 1;
	return 0;
}

/* Adds in backslash if it's a special character */
void enspecch(char *dest, char *src)
{
	do {
		if (specialchar(*src))
			*dest++ = '\\';
	} while ((*dest++ = *src++) != '\0');
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
		memcpy(dest[i], src[i], sizeof(dest[i]));
}

void multi_strclr(char string[][BUFFER], int size)
{
	for (int i=0; i<size; i++)
		string[i][0] = '\0';
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
	printf("Error: You need to create a \"tags\" file first\n");
	return 0;
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
int createfile(void)
{
	FILE *fp;	/* File writing */
	DIR *dp;	/* Directory reading */

	/* Check if file exists */
	if ((fp = fopen(FILETARGET, "r")) != NULL) {
		fclose(fp);
		return -1;
	} 

	dp = opendir("./");
	if (dp == NULL) {
		printf("Error: Couldn't open current directory\n");
		return -2;
	}
	fp = fopen(FILETARGET, "w+");		/* Make file "tags" */
	fprintf(fp, "\n");
	fclose(fp);
	printf("New file \"%s\" created\n", FILETARGET);
	return 0;
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
				memcpy(temp, string[j], sizeof temp);
				memcpy(string[j], string[j+1], 
						sizeof string[j]);
				memcpy(string[j+1], temp, sizeof string[j+1]);
				memcpy(temp, lowerstr[j], sizeof temp);
				memcpy(lowerstr[j], lowerstr[j+1], 
						sizeof lowerstr[j]);
				memcpy(lowerstr[j+1], temp, 
						sizeof lowerstr[j+1]);
			}
	return size;
}

char determine_ignoretill(int mode, char firch)
{
	if (mode == 1)
		return ' ';
	return ':';
}

int afterddff_tagsamountout(char *src, int mode)
{
	int i=0;
	char ignoretill = determine_ignoretill(mode, *src);

	while (*src != ignoretill && *src != '\n') {
		if (*src == '\n')
			return 0;	/* There will be no tags so early 0 */
		src++;
	}
	if (ignoretill == ':')
		src++;
	src++;
	while (*src++ != '\0') {
		if (*src == ' ' || *src == '\n') {
			i++;
			if (*src == '\n')
				break;
		}
	}
	return i;
}

int afterddff_strcpytomulti(char dest[][BUFFER], char *src, int mode)
{
	int i=0;
	char ignoretill = determine_ignoretill(mode, *src);

	while (*src != ignoretill && *src != '\n') {
		if (*src == '\n')
			return 0;	/* There will be no tags so early 0 */
		src++;
	}
	if (ignoretill == ':')
		src++;
	src++;
	for (int j=0; (dest[i][j] = *src) != '\0'; j++, src++) {
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
					memcpy(allowedtags[k], curtags[i], 
							sizeof allowedtags[k]);
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
			memcpy(curtags[i], allowedtags[i], sizeof curtags[i]);
	}

	return k;
}

/* Gets the filename of its line */
void getfname(char *dest, char *src)
{
	int spec_ch = 0;

	do {
		if (!spec_ch && *src == '\\') {
			spec_ch = 1;
		} else if (spec_ch)
			spec_ch = 0;
		else if (!spec_ch && *src == ' ')
			break;
	} while ((*dest++ = *src++) != '\n');
	*dest = '\0';
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

void tilldd_strcpy(char *dest, char *src)
{
	while ((*dest++ = *src++) != ':')
		;
	*--dest = '\0';
}

void listcattags(void)
{
	FILE *fp;
	char line[B_BUFFER];

	fp = fopen(FILETARGET, "r");
	if (fp == NULL) {
		printf("File \"tags\" doesn't exit in current directory\n");
		return;
	}
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			break;
		printf("%s", line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);
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
	return -1;
}

/* Remove the rejects */
void rm_rejects(char *src, int limit, char reject[][BUFFER])
{
	char strtoformat[B_BUFFER], strtocat[B_BUFFER];
	char linetags[B_BUFFER][BUFFER];
	int linechanged = 0;
	int maxtags, j;

	tilldd_strcpy(strtoformat, src);
	strncat(strtoformat, ":", 2);
	for (int i=0; i<limit; i++) {
		maxtags = afterddff_strcpytomulti(linetags, src, 0);
		for (j=0; j<maxtags; j++) {
			if (strcmp(linetags[j], reject[i]) == 0) {
				linechanged = 1;
				break;
			}
			memcpy(strtocat, " ", sizeof(strtocat));
			strncat(strtocat, linetags[j], strlen(linetags[j]));
			strncat(strtoformat, strtocat, strlen(strtocat));
		}
	}
	if (linechanged == 1) {
		strncat(strtoformat, "\n", 2);
		memcpy(src, strtoformat, sizeof &src);
	}
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

	int file_sig;

	if (argc <= 3) {
		printf("Error: You must state the category\n");
		return;		/* Terminates if no category stated */
	}
	file_sig = createfile();
	if (file_sig == -2)
		return;
	else if (file_sig == 0) {
		for (j=3; j<argc; j++) {
			memcpy(flines[i], argv[j], 
					sizeof(flines[i]));
			strncat(flines[i++], ":\n", 3);
			printf("New Category: %s\n", argv[j]);
		}
		flines[i++][0] = '\n';
		writefile(flines, i);
		return;
	}

	fp = fopen(FILETARGET, "r");
	/* Reading each line */
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp("\n", line) == 0) {
			for (j=3; j<argc; j++) {
				snprintf(str_j, 11, "%d", j);
				skip = checkdup(dups, str_j, k);
				/* Adding new category */
				if (skip == -1) {
					memcpy(flines[i], argv[j], 
							sizeof(flines[i]));
					strncat(flines[i++], ":\n", 3);
					printf("New Category: %s\n", argv[j]);
				} else {
					printf("Duplicate: \"%s\" won't be"
							" added in\n",
							argv[j]);
					skip = -1;
				}
			}
			mode = FILES;
		} else if (mode == CATEGORY) {
			tilldd_strcpy(strcompare, line);
			for (j=3; j<argc; j++) {
				if (strcmp(strcompare, argv[j]) == 0) {
					snprintf(dups[k++], 11, "%d", j);
					break;
				}
			}
		}
		memcpy(flines[i], line, sizeof(flines[i]));
		i++;
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	writefile(flines, i);
}

/* Add tag(s) to file(s) and remove tag(s) from file(s) */
void ar_tagsfiles(int argc, char *argv[], int type)
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

	/* Tags found */
	char filesfound[B_BUFFER][BUFFER];
	int m = 0, found;
	char newnumtags[B_BUFFER];
	char strtagnum[B_BUFFER];

	/* Tags sorting */
	int out;

	/* Arg-able string */
	char argablestr[B_BUFFER];

	if (argc <= 3) {
		printf("Error: You must state at least one filename\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	if (type == 0) {
		printf("Add tags you want to insert, split them by space, "
				"enter when done:\n");
	} else if (type == 1) {
		printf("Add tags you want to remove from specified file(s)"
				" given, split them by space, enter when"
				" done:\n");
	}
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
		memset(argablestr, 0, sizeof argablestr);
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
			goto skip_to_end_cat;
		} else if (mode == CATEGORY) {
			goto skip_to_end_cat;
		}
		getfname(filenameofline, line);
		for (j=3; j<argc; j++) {
			enspecch(argablestr, argv[j]);
			if (strcmp(argablestr, filenameofline) != 0)
				continue;
			printf("FOUND: \"%s\"\n", argv[j]);
			memcpy(filesfound[m], argv[j], sizeof filesfound[m]);
			m++;
			ltagsnums = intcpy(tagsnums, tags_amount);
			ltagsmax = afterddff_strcpytomulti(linetags, line, 1);
			if (type == 0) {
				strcpy_wonl(newline, line);
				for (k=0; k<tags_amount; k++) {
					/* Duplication checking */
					for (l=0; l<ltagsmax; l++) {
						snprintf(str_num, 11, "%d", 
								ltagsnums[k]);
						strdiff = strcmp(str_num, 
								linetags[l]);
						if (strdiff == 0) {
							ltagsnums[k] = -1;
							break;
						}
					}
					if (ltagsnums[k] == -1)
						continue;
					snprintf(str_num, 12, " %d", 
							ltagsnums[k]);
					strncat(newline, str_num, 
							strlen(str_num));
				}
			} else if (type == 1) {
				snprintf(newline, strlen(filenameofline)+1, 
						"%s ", filenameofline);
				int todel = 0, delt = 0;
				for (k=0; k<ltagsmax; k++) {
					for (l=0; l<tags_amount; l++) {
						snprintf(str_num, 11, "%d", 
								ltagsnums[l]);
						strdiff = strcmp(str_num, 
								linetags[k]);
						if (strdiff == 0) {
							todel = 1;
							delt++;
							break;
						}
					}
					if (todel)
						continue;
					snprintf(str_num, 
							strlen(linetags[k])+2, 
							" %s", linetags[k]);
					strncat(newline, str_num, 
							strlen(str_num));
				}
				if (delt == ltagsmax) {
					printf("File \"%s\" removed (has no"
							" tags assigned to it)"
							"\n", argv[j]);
					goto skip_to_linereset_cat;
				}
			}
			strncat(newline, "\n", 2);
			rmdubspaces(newlinefixed, newline);
			memcpy(flines[i], newlinefixed, sizeof flines[i]);
			i++;
			memset(newline, 0, sizeof newline);
			memset(newlinefixed, 0, sizeof newlinefixed);
			goto skip_to_linereset_cat;
		}
skip_to_end_cat:
		memcpy(flines[i], line, sizeof flines[i]);
		i++;
skip_to_linereset_cat:
		memset(line, 0, sizeof line);
	}
	fclose(fp);

	if (type == 1)
		goto skip_to_filetags_filewrite;

	for (k=0; k<tags_amount; k++) {
		snprintf(strtagnum, 12, " %d", tagsnums[k]);
		strncat(newnumtags, strtagnum, strlen(strtagnum));
	}
	strncat(newnumtags, "\n", 2);

	/* Putting in new files */
	for (j=3; j<argc; j++) {
		found = 0;
		for (k=0; k<m; k++) {
			if (strcmp(argv[j], filesfound[k]) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0) {
			enspecch(argablestr, argv[j]);
			snprintf(flines[i++], strlen(argablestr)+
					strlen(newnumtags)+1, "%s%s", 
					argablestr, newnumtags);
			printf("New file added: \"%s\"\n", argv[j]);
			memset(argablestr, 0, sizeof argablestr);
		}
	}

	/* Sorts the file strings */
	out = str_sort(flines, i, 1);
	if (out == -1) {
		printf("Error: Unable to sort (can't find a newline in between"
				" the categories and filenames)\n");
		return;
	}

skip_to_filetags_filewrite:
	writefile(flines, i);
}

/* Remove specified files from the list */
void removefiles(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char line[B_BUFFER];
	char flines[B_BUFFER][BUFFER];
	int i = 0, j;
	int mode = CATEGORY;
	
	/* Checking variables */
	char filename[B_BUFFER];
	char argablestr[B_BUFFER];

	if (argc <= 3) {
		printf("Error: You must state at least one filename to"
				" remove\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
			goto removefiles_lineadd;
		} else if (mode == CATEGORY)
			goto removefiles_lineadd;
		getfname(filename, line);
		for (j=3; j<argc; j++) {
			enspecch(argablestr, argv[j]);
			if (strcmp(argablestr, filename) == 0) {
				printf("Removing file: \"%s\"\n", filename);
				goto removefiles_linereset;
			}
		}
removefiles_lineadd:
		memcpy(flines[i], line, sizeof flines[i]);
		i++;
removefiles_linereset:
		memset(filename, 0, sizeof filename);
		memset(line, 0, sizeof line);
	}

	writefile(flines, i);
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
	} else if (!(checkifexist(FILETARGET)))
		return;

	for (i=0; i<argc-2; i++)
		memcpy(ftagsnames[i], argv[i+2], sizeof(ftagsnames[i]));
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
				snprintf(str_num, 11, "%d", ftagsnums[j]);
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
	for (i=0; i<k; i++)
		printf("%s ", slines[i]);
	putchar('\n');
}

/* Add/Remove tags */
void ar_tags(int argc, char *argv[], int type)
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
	int t=0, dubcheck = -1;
	char strtoformat2[B_BUFFER];
	char strtoformat3[B_BUFFER];
	int strdiff;

	/* Numbers modification */
	char linenums[B_BUFFER][BUFFER];
	char numstrnew[S_BUFFER];

	/* Number change */
	int min, addby;
	addby = min = -1;
	int tagnum = -1;

	/* Number remover */
	int z=0, y=0, x;
	int rm_nums[B_BUFFER];
	int wontbeadded = 0;
	char stradd[B_BUFFER];
	int base = 0;
	int rm_tagsnums[B_BUFFER];
	int needrm;

	/* Category remover */
	char rmtags[B_BUFFER][BUFFER];
	int rmmax;

	if (argc <= 4 && type != 2) {
		printf("Error: You must state at least one tag and a"
				" category\n");
		return;
	} else if (argc != 4 && type == 2) {
		printf("Error: You must state one category\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	if (type == 2) {	/* Set warning for remove category command */
		printf("CAUTION: You're going to remove the category:"
				" \"%s\".\n"
				"This will remove ALL tags within the category"
				" which will also remove the numerical"
				" assignment for ALL those tags also!\n"
				"\nDo you like to continue? Y/N [N]: ",
				argv[3]);
		int c = getchar();
		int ans = c;
		while (c != '\n')
			c = getchar();
		if (ans != 'Y')
			return;
	}

	if (type == 0)
		getchange(&min, &addby, argc-1, argv);
	else if (type >= 1)
		addby = 0;

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
			if (type == 2) {
				multi_strcpy(rmtags, linetags, maxtags);
				rmmax = maxtags;
				for (j=0; j<rmmax; j++) {
					dup = checkdup(linetags, rmtags[j], 
							maxtags);
					rm_tagsnums[y++] = dup+base;
				}
				memset(line, 0, sizeof line);
				base += maxtags;
				continue;
			}
			for (j=3; j<argc-1 && type != 2; j++, dup=0) {
				if (maxtags > 0) {
					dup = checkdup(linetags, argv[j], 
							maxtags);
				}
				if ((dup == -1 || maxtags == 0) && type == 0) {
					strcat_as(strtoformat, argv[j]);
					printf("\"%s\" added as tag under"
							" \"%s\"\n", argv[j], 
							argv[argc-1]);
				} else if (dup >= 0 && type == 0) {
					printf("Duplicate \"%s\" won't be put"
							" in\n", argv[j]);
				} else if (dup >= 0 && type == 1) {
					printf("\"%s\" will be removed from"
							" \"%s\"\n", 
							argv[j], argv[argc-1]);
					rm_nums[z++] = dup;
					rm_tagsnums[y++] = dup+base;
				}
			}
			if (type == 1 && z > 0) {
				memset(strtoformat, 0, sizeof strtoformat);
				snprintf(strtoformat, strlen(strcompare)+2, 
						"%s:", strcompare);
				for (j=0; j<maxtags; j++) {
					for (k=0; k<z; k++) {
						if (j == rm_nums[k]) {
							wontbeadded = 1;
							break;
						}
					}
					if (wontbeadded) {
						wontbeadded = 0;
						continue;
					}
					snprintf(stradd, strlen(linetags[j])+2,
							" %s", linetags[j]);
					strncat(strtoformat, stradd, 
							strlen(stradd));
				}
				z = 0;
			}
			dubcheck = i;
			strncat(strtoformat, "\n", 2);
			memcpy(flines[i], strtoformat, sizeof(flines[i]));
			i++;
			memset(line, 0, sizeof line);
			base += maxtags;
			continue;
		} else if (mode == CATEGORY && strdiff != 0) {
			for (j=3; j<argc-1; j++, dup=0) {
				dup = checkdup(linetags, argv[j], maxtags);
				if (dup >= 0) {
					memcpy(reject[t], argv[j], 
							sizeof(reject[t]));
					t++;
					if (type == 0) {
						printf("Duplicate \"%s\" won't"
								" be put in\n",
								argv[j]);
					}
				}
			}
			base += maxtags;
		} else if (mode == FILES) {
			maxtags = afterddff_strcpytomulti(linenums, line, 1);
			if (type == 0 && (maxtags == 0 || min == -1))
				goto skip_to_end_addtags;
			l = 0;
			wontbeadded = 0;
			if (type != 0)
				addby = 0;
			getfname(strtoformat2, line);
			strncat(strtoformat2, " ", 2);
			for (k=0; k<maxtags; k++) {
				tagnum = atoi(linenums[k]);
				needrm = 0;
				for (x=0; x<y && type >= 1; x++) {
					if (tagnum > rm_tagsnums[x])
						addby = -(x+1);
					else if (tagnum == rm_tagsnums[x]) {
						needrm = 1;
						break;
					}
				}
				if (tagnum >= min && type == 0) {
					l++;
					snprintf(numstrnew, 12, " %d", 
							tagnum+addby);
				} else if (type == 0 || 
						(!needrm && type >= 1)) {
					if (tagnum < rm_tagsnums[0] ||
							type == 0) {
						snprintf(numstrnew, 12, " %d",
								tagnum);
					} else {
						l++;
						snprintf(numstrnew, 12, " %d",
								tagnum+addby);
					}
				} else if (type >= 1) {
					memcpy(numstrnew, "\0", 
							sizeof numstrnew);
					l++;
				}
				strncat(strtoformat2, numstrnew, 
						strlen(numstrnew));
			}
			if (l == 0) {	/* If no change needed */
				memcpy(flines[i], line, sizeof(flines[i]));
				i++;
			} else {
				strncat(strtoformat2, "\n", 2);
				rmdubspaces(strtoformat3, strtoformat2);
				maxtags = afterddff_tagsamountout(strtoformat3,
						1);
				if (maxtags > 0) {
					memcpy(flines[i], strtoformat3, 
							sizeof(flines[i]));
					i++;
				}
				memset(strtoformat3, 0, sizeof strtoformat3);
			}
			memset(strtoformat2, 0, sizeof strtoformat2);
			memset(line, 0, sizeof line);
			continue;
		}
skip_to_end_addtags:
		memcpy(flines[i], line, sizeof flines[i]);
		i++;
		memset(line, 0, sizeof line);
	}
	fclose(fp);

	if (validcategory == 0) {
		printf("The category provided isn't valid\n");
		return; 
	}

	if (type != 2 && dubcheck != -1)
		rm_rejects(flines[dubcheck], t, reject);
	writefile(flines, i);
}

/* Returns string after colon */
char *afterdd_strreturn(char *src)
{
	int i=0;
	char *dest = malloc(B_BUFFER);
	while (*src++ != ':')
		;
	src++;
	while ((dest[i++] = *src++) != '\0')
		;
	dest[--i] = '\0';
	return dest;
}

/* Renames tag or category */
void rename_tc(int argc, char *argv[], char *str_rntype)
{
	FILE *fp;

	char flines[B_BUFFER][BUFFER];
	char line[B_BUFFER];

	int type = CATEGORY;
	char strcompare[B_BUFFER];
	int renamed = 0;
	int i = 0, ltagsmax = 0, j;
	int rntype = !(strcmp(str_rntype, "category")) ? CAT_NAME : TAG_NAME;
	char linetags[B_BUFFER][BUFFER];
	char temptag[B_BUFFER];

	if (argc != 5) {
		printf("Error: You must state only one replacement name and"
				" only one current %s name\n", str_rntype);
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;
	
	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			type = FILES;
		else if (type == CATEGORY)
			tilldd_strcpy(strcompare, line);
		if (type == CATEGORY && rntype == TAG_NAME)
			ltagsmax = afterddff_strcpytomulti(linetags, line, 0);
		if (type == CATEGORY && renamed == 0 && 
				rntype == CAT_NAME && 
				strcmp(strcompare, argv[4]) == 0) {
			printf("FOUND: \"%s\", replacement: \"%s\"\n", argv[4],
					argv[3]);
			char *afdline = afterdd_strreturn(line);
			snprintf(flines[i], strlen(argv[3])+3+strlen(afdline),
					"%s: %s", 
					argv[3], afdline);
			i++;
			renamed = 1;
			continue;
		} else if (type == CATEGORY && renamed == 0 &&
				rntype == TAG_NAME &&
				ltagsmax > 0) {
			snprintf(flines[i], strlen(strcompare)+2, "%s:", 
					strcompare);
			for (j=0; j<ltagsmax; j++) {
				if (strcmp(linetags[j], argv[4]) == 0) {
					printf("FOUND: \"%s\", replacement:"
							" \"%s\"\n", argv[4], 
							argv[3]);
					snprintf(linetags[j], 
							strlen(argv[3])+2, 
							" %s", argv[3]);
					renamed = 1;
				} else {
					memcpy(temptag, linetags[j], 
							sizeof temptag);
					snprintf(linetags[j], 
							strlen(temptag)+2, 
							" %s", temptag);
				}
				strncat(flines[i], linetags[j], 
						strlen(linetags[j]));
			}
			strncat(flines[i++], "\n", 2);
			continue;
		}
		memcpy(flines[i], line, sizeof flines[i]);
		i++;
	}
	writefile(flines, i);
}

void par2(int argc, char *argv[], int type)
{
	if (argc > 2) {
		if (!strcmp(argv[2], "tags") && type == ADD)
			ar_tags(argc, argv, 0);
		else if (!strcmp(argv[2], "category") && type == ADD)
			addcategory(argc, argv);
		else if (!strcmp(argv[2], "tags-to") && type == ADD)
			ar_tagsfiles(argc, argv, 0);
		else if (!strcmp(argv[2], "tag") && type == RENAME)
			rename_tc(argc, argv, "tag");
		else if (!strcmp(argv[2], "category") && type == RENAME)
			rename_tc(argc, argv, "category");
		else if (!strcmp(argv[2], "tags") && type == REMOVE)
			ar_tags(argc, argv, 1);
		else if (!strcmp(argv[2], "category") && type == REMOVE)
			ar_tags(argc, argv, 2);
		else if (!strcmp(argv[2], "from-tags") && type == REMOVE)
			ar_tagsfiles(argc, argv, 1);
		else if (!strcmp(argv[2], "files") && type == REMOVE)
			removefiles(argc, argv);
		else
			printf("Error: You must give: %s\n",
					usage_error[type]);
	} else
		printf("Error: You must give at least one parameter:"
				" %s\n", usage_error[type]);
}

void readfile(void)
{
	FILE *fp;
	char line[B_BUFFER];

	if ((fp = fopen(FILETARGET, "r")) == NULL) {
		printf("File \"%s\" doesn't exit in current directory\n", 
				FILETARGET);
		return;
	}
	while (fgets(line, B_BUFFER, fp) != NULL) {
		printf("%s", line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);
}

void usage(void)
{
	printf("usage: {create | read | version | help | search | categories |"
			" add %s | rename %s | remove %s}\n",
			usage_error[ADD], usage_error[RENAME],
			usage_error[REMOVE]);
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (!strcmp(argv[1], "read"))
			readfile();
		else if (!strcmp(argv[1], "version"))
			printf("%s\n", VERSION);
		else if (!strcmp(argv[1], "help"))
			usage();
		else if (!strcmp(argv[1], "add"))
			par2(argc, argv, ADD);
		else if (!strcmp(argv[1], "search"))
			searchtags(argc, argv);
		else if (!strcmp(argv[1], "categories"))
			listcattags();
		else if (!strcmp(argv[1], "create"))
			createfile();
		else if (!strcmp(argv[1], "rename"))
			par2(argc, argv, RENAME);
		else if (!strcmp(argv[1], "remove"))
			par2(argc, argv, REMOVE);
		else
			usage();
	} else
		usage();
	return 0;
}

