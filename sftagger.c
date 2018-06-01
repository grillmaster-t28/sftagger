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
#define B_BUFFER 2560

#define VERSION "3.1 RC1 - 2018/06/01"
#define FILETARGET "tags"
#define FILETARGETTEMP ".temp-" FILETARGET

#define RENAME(x, y) \
	if (rename(x, y)) \
		printf("Error: Unable to rename file\n");

#define PCHECK(x, y) \
	!strcmp(argv[x], y)

#define TCHECK(x, y, z) \
	!strcmp(argv[x], y) && type == z

enum {
	CATEGORY, FILES,
	FALSE = 0, TRUE,
	CAT_NAME = 0, TAG_NAME,
	ADD = 0, RENAME, REMOVE,
	ADD_TAGS = 0, RM_TAGS, RM_CAT
};

static const char *usage_error[] = {
	"{category | tags | tags-to}",
	"{category | tag}",
	"{category | tags | from-tags | files}"
};

int specialchar(char c)
{
	if (!(c >= '-' && c <= '/') && c != '_' && !isalnum(c) && !(c < 32))
		return 1;
	return 0;
}

/* Adds in backslash if it's a special character */
void enspecch(char *dest, const char src[])
{
	do {
		if (specialchar(*src))
			*dest++ = '\\';
	} while ((*dest++ = *src++) != '\0');
}

void strtolower(char *dest, const char src[])
{
	while ((*dest++ = tolower(*src++)) != '\0')
		;
	*--dest = '\0';
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
		memcpy(dest[i], src[i], strlen(src[i]) + 1);
}

void multi_strclr(char string[][BUFFER], int size)
{
	for (int i=0; i<size; i++)
		string[i][0] = '\0';
}

/* Remove extra white-spaces */
void rmdubspaces(char *dest, const char src[])
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
void strcat_as(char *dest, const char add[])
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
void strcpy_wonl(char *dest, const char src[])
{
	while ((*dest++ = *src++) != '\n')
		;
	*--dest = '\0';
}

/* Check if the file exists */
int checkifexist(char filename[])
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
void writefile(char flines[][BUFFER], int max, const char filename[])
{
	FILE *fp;

	fp = fopen(filename, "w");
	for (int i = 0; i < max; i++)
		fprintf(fp, "%s", flines[i]); 
	fclose(fp);
}

unsigned int countlines(const char filename[])
{
	FILE *fp = fopen(filename, "r");
	int i = 0;
	char line[B_BUFFER];
	while (fgets(line, B_BUFFER, fp) != NULL)
		i++;
	fclose(fp);
	return i;
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

void strswap(char *str1, char *str2)
{
	char strT[BUFFER];
	memcpy(strT, str1, strlen(str1)+1);
	memcpy(str1, str2, strlen(str2)+1);
	memcpy(str2, strT, strlen(strT)+1);
	memset(strT, 0, sizeof strT);
}

/* Quicksorting strings */
int partition(char **str, int low, int high) 
{
	char pivot[BUFFER];
	char lwrstr1[BUFFER], lwrstr2[BUFFER];
	strtolower(pivot, str[high]);
	int i = low - 1;
	for (int j = low; j <= high - 1; j++) {
		strtolower(lwrstr1, str[j]);
		if (strcmp(lwrstr1, pivot) < 0)
			strswap(str[++i], str[j]);
		memset(lwrstr1, 0, sizeof lwrstr1);
	}
	strtolower(lwrstr1, str[high]);
	strtolower(lwrstr2, str[i + 1]);
	strswap(str[i + 1], str[high]);
	memset(lwrstr1, 0, sizeof lwrstr1);
	memset(lwrstr2, 0, sizeof lwrstr2);
	memset(pivot, 0, sizeof pivot);
	return (i + 1);
}

void quicksort(char **str, int low, int high)
{
	if (low < high) {
		int pivot = partition(str, low, high);
		quicksort(str, low, pivot - 1);
		quicksort(str, pivot + 1, high);
	}
}

char determine_ignoretill(int mode, char firch)
{
	if (mode == 1)
		return ' ';
	return ':';
}

int afterddff_tagsamountout(const char src[], int mode)
{
	int i=0;
	char ignoretill = determine_ignoretill(mode, *src);
	char prev = ' ';

	while (*src != ignoretill || prev == '\\') {
		if (*src == '\n')
			return 0;	/* There will be no tags so early 0 */
		prev = *src++;
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

int afterddff_strcpytomulti(char dest[][BUFFER], const char src[], int mode)
{
	int i=0;
	char ignoretill = determine_ignoretill(mode, *src);
	char prev = ' ';

	while (*src != ignoretill || prev == '\\') {
		if (*src == '\n')
			return 0;	/* There will be no tags so early 0 */
		prev = *src++;
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
							strlen(curtags[i])+1);
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
			memcpy(curtags[i], allowedtags[i], 
					strlen(allowedtags[i]) + 1);
	}

	return k;
}

/* Gets the filename of its line */
void getfname(char *dest, const char src[])
{
	int spec_ch = 0;

	do {
		if ((!spec_ch && *src == '\\') || spec_ch)
			spec_ch = !spec_ch;
		else if (!spec_ch && *src == ' ')
			break;
	} while ((*dest++ = *src++) != '\n');
	*dest = '\0';
}

int *intcpy(int *src, size_t len)
{
	int *dest = malloc(len * sizeof(int));
	memcpy(dest, src, len * sizeof(int));
	return dest;
}

void tilldd_strcpy(char *dest, const char src[])
{
	while ((*dest++ = *src++) != ':')
		;
	*--dest = '\0';
}

int emptycat(int on[], int from)
{
	do {
		if (*on == from)
			return 1;
	} while (*on++);
	return 0;
}

void listcattags(void)
{
	FILE *fp;
	char line[B_BUFFER];
	int i;
	int ti = 0;		/* Tag integer */
	int cn = 0;		/* Category Number */
	int ei = 0;		/* Empty category integer */
	int mode = CATEGORY;
	int ltagstotal;
	char linetags[B_BUFFER][BUFFER];
	int tagnum;

	struct tagsnums {
		char name[BUFFER];
		int amount;
		int catnum;		/* Category Number */
	} tag[B_BUFFER];

	struct catna {
		char name[BUFFER];
		int amount;
	} cat[B_BUFFER];

	int prevcatnum = -1;
	int emptycats[BUFFER];

	fp = fopen(FILETARGET, "r");
	if (fp == NULL) {
		printf("File \"tags\" doesn't exit in current directory\n");
		return;
	}

	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
		} else if (mode == CATEGORY) {
			ltagstotal = afterddff_strcpytomulti(linetags, 
					line, 0);
			if (ltagstotal == 0)
				emptycats[ei++] = cn;
			for (i = 0; i < ltagstotal; i++, ti++) {
				tag[ti].catnum = cn;
				tag[ti].amount = 0;
				memcpy(tag[ti].name, linetags[i], 
						strlen(linetags[i]) + 1);
			}
			cat[cn].amount = 0;
			tilldd_strcpy(cat[cn++].name, line);
		} else {
			ltagstotal = afterddff_strcpytomulti(linetags,
					line, 1);
			for (i = 0; i < ltagstotal; i++) {
				tagnum = atoi(linetags[i]);
				tag[tagnum].amount++;
				cat[tag[tagnum].catnum].amount++;
			}
		}
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	tag[ti].catnum = -1;

	for (i = 0; i < ti; i++) {
		if (prevcatnum != tag[i].catnum) {
			printf("%s (%d):\n\t", cat[tag[i].catnum].name, 
					cat[tag[i].catnum].amount);
		}
		if (tag[i+1].catnum == tag[i].catnum && tag[i+1].catnum != -1)
			printf("%s (%d), ", tag[i].name, tag[i].amount);
		else	/* Last tag in category */
			printf("%s (%d)\n", tag[i].name, tag[i].amount);
		prevcatnum = tag[i].catnum;
	}
	if (ei > 0)
		printf("\nEmpty categories: ");
	for (i = 0; i < ei; i++) {
		if (i == ei-1)
			printf("%s\n", cat[emptycats[i]].name);
		else
			printf("%s, ", cat[emptycats[i]].name);
	}
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
int checkdup(char src_a[][BUFFER], const char src_b[], int a_limit)
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
			memcpy(strtocat, " ", 2);
			strncat(strtocat, linetags[j], strlen(linetags[j]));
			strncat(strtoformat, strtocat, strlen(strtocat));
		}
	}
	if (linechanged == 1) {
		strncat(strtoformat, "\n", 2);
		memcpy(src, strtoformat, strlen(strtoformat) + 1);
	}
}

/* Add category */
void addcategory(int argc, char *argv[])
{
	if (argc <= 3) {
		printf("Error: You must state the category\n");
		return;		/* Terminates if no category stated */
	}

	FILE *fp;

	/* File reading variables */
	int j, k = 0;

	int file_sig = createfile();
	if (file_sig == -2)
		return;
	else if (file_sig == 0) {
		fp = fopen(FILETARGET, "w");
		for (j=3; j<argc; j++) {
			fprintf(fp, "%s:\n", argv[j]);
			printf("New Category: %s\n", argv[j]);
		}
		fprintf(fp, "\n");
		fclose(fp);
		return;
	}

	FILE *fpt;
	char line[B_BUFFER];

	/* Duplication checking variables */
	int mode = CATEGORY;
	char dups[S_BUFFER][BUFFER];
	char str_j[S_BUFFER];
	char strcompare[B_BUFFER];
	int skip = 0;

	fp = fopen(FILETARGET, "r");
	fpt = fopen(FILETARGETTEMP, "w");
	/* Reading each line */
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp("\n", line) == 0) {
			for (j=3; j<argc; j++) {
				snprintf(str_j, 11, "%d", j);
				skip = checkdup(dups, str_j, k);
				/* Adding new category */
				if (skip == -1) {
					fprintf(fpt, "%s:\n", argv[j]);
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
		fprintf(fpt, "%s", line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	fclose(fpt);
	RENAME(FILETARGETTEMP, FILETARGET);
}

void filesaddcheck(char line[], char *newline, int tags_amount,
		int ltagsnums[], int ltagsmax, char linetags[][BUFFER])
{
	int l;
	char str_num[S_BUFFER];

	strcpy_wonl(newline, line);
	for (int k=0; k<tags_amount; k++) {
		/* Duplication checking */
		for (l=0; l<ltagsmax; l++) {
			snprintf(str_num, 11, "%d", ltagsnums[k]);
			if (!strcmp(str_num, linetags[l])) {
				ltagsnums[k] = -1;
				break;
			}
		}
		if (ltagsnums[k] == -1)
			continue;
		snprintf(str_num, 12, " %d", ltagsnums[k]);
		strncat(newline, str_num, strlen(str_num));
	}
}

int filesrmcheck(char *newline, char filenameofline[], int ltagsmax, 
		int tags_amount, int ltagsnums[], char linetags[][BUFFER],
		char file[])
{
	int todel = 0, delt = 0, l;
	char str_num[S_BUFFER];

	snprintf(newline, strlen(filenameofline)+1, 
			"%s ", filenameofline);
	for (int k=0; k<ltagsmax; k++) {
		todel = 0;
		for (l=0; l<tags_amount; l++) {
			snprintf(str_num, 11, "%d", ltagsnums[l]);
			if (!strcmp(str_num, linetags[k])) {
				todel = 1;
				delt++;
				break;
			}
		}
		if (todel)
			continue;
		snprintf(str_num, strlen(linetags[k])+2, " %s", linetags[k]);
		strncat(newline, str_num, strlen(str_num));
	}
	if (delt == ltagsmax) {
		printf("File \"%s\" removed (has no tags assigned to it)\n",
				file);
		return 0;	/* Skip file */
	}
	return 1;
}

/* Add tag(s) to file(s) and remove tag(s) from file(s) */
void ar_tagsfiles(int argc, char **argv, int type)
{
	if (argc <= 3) {
		printf("Error: You must state at least one filename\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;
	
	/* Let UNIX/UNIX-like system handle the wildcards */

	FILE *fp;
	FILE *fpt;

	/* File reading variables */
	unsigned long j, k;
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

	/* Duplication checking variables */
	char linetags[B_BUFFER][BUFFER];
	int ltagsmax = 0;
	int *ltagsnums;

	/* Tags found */
	unsigned int filelines = countlines(FILETARGET);
	char **filesfound = malloc(filelines * sizeof *filesfound);
	int m = 0, found;
	char newnumtags[B_BUFFER];
	char strtagnum[B_BUFFER];

	/* Arg-able string */
	char argablestr[B_BUFFER];

	if (type == ADD) {
		printf("Add tags you want to insert, split them by space, "
				"enter when done:\n");
	} else if (type == REMOVE) {
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
	fpt = fopen(FILETARGETTEMP, "w");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		memset(argablestr, 0, sizeof argablestr);
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
		} else if (mode == FILES) {
			getfname(filenameofline, line);
			for (j=3; j<argc; j++) {
				enspecch(argablestr, argv[j]);
				if (strcmp(argablestr, filenameofline) != 0)
					continue;
				printf("FOUND: \"%s\"\n", argv[j]);
				filesfound[m] = malloc(B_BUFFER * sizeof *filesfound[m]);
				memcpy(filesfound[m], argv[j],
						strlen(argv[j])+1);
				m++;
				ltagsnums = intcpy(tagsnums, tags_amount);
				ltagsmax = afterddff_strcpytomulti(linetags,
						line, 1);
				if (type == ADD) {
					filesaddcheck(line, newline,
							tags_amount, ltagsnums,
							ltagsmax, linetags);
				} else if (type == REMOVE && 
						!filesrmcheck(newline, 
							filenameofline,
							ltagsmax, tags_amount,
							ltagsnums, linetags,
							argv[j])) {
						goto skip_to_linereset_cat;
				}
				strncat(newline, "\n", 2);
				rmdubspaces(newlinefixed, newline);
				fprintf(fpt, "%s", newlinefixed);
				memset(newline, 0, sizeof newline);
				memset(newlinefixed, 0, sizeof newlinefixed);
				goto skip_to_linereset_cat;
			}
		}
		fprintf(fpt, "%s", line);
skip_to_linereset_cat:
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	fclose(fpt);

	if (type == REMOVE)
		goto skip_to_filetags_filewrite;

	char **nflines = malloc(argc * sizeof *nflines);
	int nfl = 0;		// Limit
	int nfm = 0;		// Min

	for (k=0; k<tags_amount; k++) {
		snprintf(strtagnum, 12, " %d", tagsnums[k]);
		strncat(newnumtags, strtagnum, strlen(strtagnum));
	}

	/* Putting in new files */
	for (j=3; j<argc; j++) {
		found = 0;
		for (k=0; k<m; k++) {
			if (!strcmp(argv[j], filesfound[k])) {
				found = 1;
				break;
			}
		}
		if (found == 0 && strcmp(argv[j], FILETARGET)) {
			enspecch(argablestr, argv[j]);
			nflines[nfl] = malloc(B_BUFFER * sizeof *nflines[nfl]);
			snprintf(nflines[nfl++], strlen(argablestr)+
					strlen(newnumtags)+2, "%s%s\n", 
					argablestr, newnumtags);
			printf("New file/directory added: \"%s\"\n", argv[j]);
			memset(argablestr, 0, sizeof argablestr);
		}
	}

	/* Sorts the new files strings */
	quicksort(nflines, 0, nfl-1);

	/* Applying the new files (temp1 to temp2 file) */
	fpt = fopen(FILETARGETTEMP, "r");
	FILE *fpt2 = fopen(FILETARGETTEMP "-2", "w");
	mode = CATEGORY;
	char *lowernfline = (char *) malloc(B_BUFFER);
	while (fgets(line, B_BUFFER, fpt) != NULL) {
		if (!strcmp(line, "\n")) {
			mode = FILES;
		} else if (mode == FILES) {
			for (j=nfm; j<nfl; j++) {
				strtolower(lowernfline, nflines[j]);
				if (strcmp(lowernfline, line) < 0) {
					fprintf(fpt2, "%s", nflines[j]);
					nfm++;
					memset(lowernfline, 0, B_BUFFER);
				} else {
					break;
				}
			}
		}
		fprintf(fpt2, "%s", line);
		memset(line, 0, sizeof line);
	}
	free(lowernfline);
	/* Leftovers */
	for (j=nfm; j<nfl; j++)
		fprintf(fpt2, "%s", nflines[j]);
	fclose(fpt);
	free(nflines);
	RENAME(FILETARGETTEMP "-2", FILETARGETTEMP);

skip_to_filetags_filewrite:
	free(filesfound);
	RENAME(FILETARGETTEMP, FILETARGET);
}

/* Remove specified files from the list */
void removefiles(int argc, char *argv[])
{
	if (argc <= 3) {
		printf("Error: You must state at least one filename to"
				" remove\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	FILE *fp;
	FILE *fpt;

	/* File reading variables */
	char line[B_BUFFER];
	int i = 0, j;
	int mode = CATEGORY;
	
	/* Checking variables */
	char filename[B_BUFFER];
	char argablestr[B_BUFFER];

	fp = fopen(FILETARGET, "r");
	fpt = fopen(FILETARGETTEMP, "w");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
		} else if (mode == FILES) {
			getfname(filename, line);
			for (j=3; j<argc; j++) {
				enspecch(argablestr, argv[j]);
				if (strcmp(argablestr, filename) == 0) {
					printf("Removing file: \"%s\"\n",
							filename);
					goto removefiles_linereset;
				}
			}
		}
		fprintf(fpt, "%s", line);
		i++;
removefiles_linereset:
		memset(filename, 0, sizeof filename);
		memset(line, 0, sizeof line);
	}

	fclose(fp);
	fclose(fpt);
	RENAME(FILETARGETTEMP, FILETARGET);
}

void outallfiles(void)
{
	FILE *fp;

	/* File reading variables */
	char line[B_BUFFER];
	char linefile[BUFFER];
	int mode = CATEGORY;

	if (!(checkifexist(FILETARGET)))
		return;

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
		} else if (mode == FILES) {
			getfname(linefile, line);
			printf("%s ", linefile);
		}
		memset(line, 0, sizeof line);
	}
	fclose(fp);

	putchar('\n');
}

void filesinfos(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	int i, ti = 0;
	char line[B_BUFFER];
	int mode = CATEGORY;

	int ltagstotal;

	struct tagsnums {
		char name[BUFFER];
		char catn[BUFFER];
	} tag[B_BUFFER];

	char linecat[BUFFER];
	char linetags[B_BUFFER][BUFFER];
	char linefile[BUFFER];

	int filematch;
	int tagnum;
	int filesfound = 0;

	char endchars[3] = ", ";

	if (argc <= 2) {
		printf("Error: You must state at least one file\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	fp = fopen(FILETARGET, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
		} else if (mode == CATEGORY) {
			ltagstotal = afterddff_strcpytomulti(linetags, 
					line, 0);
			tilldd_strcpy(linecat, line);
			for (i = 0; i < ltagstotal; i++, ti++) {
				memcpy(tag[ti].catn, linecat, 
						strlen(linecat) + 1);
				memcpy(tag[ti].name, linetags[i], 
						strlen(linetags[i]) + 1);
			}
		} else {
			filematch = 0;
			getfname(linefile, line);
			for (i = 2; i < argc; i++) {
				if (!strcmp(linefile, argv[i])) {
					filematch = 1;
					filesfound = 1;
					break;
				}
			}
			if (filematch) {
				ltagstotal = afterddff_strcpytomulti(linetags,
						line, 1);
				printf("%s [%d tags]:\n\t", linefile, 
						ltagstotal);
				for (i = 0; i < ltagstotal; i++) {
					tagnum = atoi(linetags[i]);
					if (i == ltagstotal-1) {
						memcpy(endchars, "\n", 2);
					}
					printf("%s [%s]%s", tag[tagnum].name, 
							tag[tagnum].catn, 
							endchars);
				}
				memcpy(endchars, ", ", 3);
			}
		}
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	if (!filesfound) {
		printf("Notice: No files found with the files given\n");
		return;
	}

}

int linematchtags(char linetags[B_BUFFER][BUFFER], int ltagstotal,
		int ftagsnums[], int maxtags)
{
	int j, filematches = 0;
	char str_num[BUFFER];
	for (int i=0; i<ltagstotal; i++) {
		for (j=0; j<maxtags; j++) {
			snprintf(str_num, 11, "%d",
					ftagsnums[j]);
			if (strcmp(linetags[i], str_num) ==
					0) {
				filematches++;
				break;
			}
		}
	}
	return filematches;
}

void searchtags(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char linefile[B_BUFFER];
	int i = 0, found = 0;
	char line[B_BUFFER];
	int mode = CATEGORY;

	/* Tags filtering */
	int maxtags;
	int ftagsnums[BUFFER];
	char ftagsnames[B_BUFFER][BUFFER];

	/* In-line variables */
	int ltagstotal = 0;
	char linetags[B_BUFFER][BUFFER];

	if (argc <= 2) {
		printf("Error: You must state at least one tag\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	for (i=0; i<argc-2; i++)
		memcpy(ftagsnames[i], argv[i+2], strlen(argv[i+2]) + 1);
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
		} else if (mode == FILES) {
			ltagstotal = afterddff_strcpytomulti(linetags, 
					line, 1);
			if (linematchtags(linetags, ltagstotal, ftagsnums,
						maxtags) == maxtags) {
				found++;
				getfname(linefile, line);
				printf("%s ", linefile);
			}
		}
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	if (!found)
		printf("Notice: No files found with the tags given");
	putchar('\n');
}

/* Add/Remove tags */
void ar_tags(int argc, char *argv[], int type)
{
	if (argc <= 4 && type != RM_CAT) {
		printf("Error: You must state at least one tag and a"
				" category\n");
		return;
	} else if (argc != 4 && type == RM_CAT) {
		printf("Error: You must state one category\n");
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;

	if (type == RM_CAT) {	/* Set warning for remove category command */
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

	FILE *fp;
	FILE *fpt;

	/* File reading variables */
	char cflines[B_BUFFER][BUFFER];
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
	int t = 0, dubcheck = -1;
	char strtoformat2[B_BUFFER];
	char strtoformat3[B_BUFFER];

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

	if (type == ADD_TAGS)
		getchange(&min, &addby, argc-1, argv);
	else if (type >= 1)
		addby = 0;

	fp = fopen(FILETARGET, "r");
	fpt = fopen(FILETARGETTEMP, "w");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		/* newline changes to file mode */
		maxtags = 0;
		if (strcmp(line, "\n") == 0) {
			mode = FILES;
			if (type != 2 && dubcheck != -1)
				rm_rejects(cflines[dubcheck], t, reject);
			for (j = 0; j < i; j++)
				fprintf(fpt, "%s", cflines[j]);
		} else if (mode == CATEGORY) {
			tilldd_strcpy(strcompare, line);
			maxtags = afterddff_strcpytomulti(linetags, line, 0);
		}
		if (mode == CATEGORY && !strcmp(strcompare, argv[argc-1])) {
			validcategory = 1;
			strcpy_wonl(strtoformat, line);
			if (type == RM_CAT) {
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
			for (j=3; j<argc-1 && type != RM_CAT; j++, dup=0) {
				if (maxtags > 0) {
					dup = checkdup(linetags, argv[j], 
							maxtags);
				}
				if ((dup == -1 || maxtags == 0) && type ==
						ADD_TAGS) {
					strcat_as(strtoformat, argv[j]);
					printf("\"%s\" added as tag under"
							" \"%s\"\n", argv[j], 
							argv[argc-1]);
				} else if (dup >= 0 && type == ADD_TAGS) {
					printf("Duplicate \"%s\" won't be put"
							" in\n", argv[j]);
				} else if (dup >= 0 && type == RM_TAGS) {
					printf("\"%s\" will be removed from"
							" \"%s\"\n", 
							argv[j], argv[argc-1]);
					rm_nums[z++] = dup;
					rm_tagsnums[y++] = dup+base;
				}
			}
			if (type == RM_TAGS && z > 0) {
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
			memcpy(cflines[i], strtoformat, strlen(strtoformat)+1);
			i++;
			memset(line, 0, sizeof line);
			base += maxtags;
			continue;
		} else if (mode == CATEGORY) {
			for (j=3; j<argc-1; j++, dup=0) {
				dup = checkdup(linetags, argv[j], maxtags);
				if (dup >= 0) {
					memcpy(reject[t], argv[j], 
							strlen(argv[j]) + 1);
					t++;
					if (type == ADD_TAGS) {
						printf("Duplicate \"%s\" won't"
								" be put in\n",
								argv[j]);
					}
				}
			}
			base += maxtags;
			memcpy(cflines[i], line, strlen(line) + 1);
			i++;
			memset(line, 0, sizeof line);
			continue;
		} else if (mode == FILES) {
			maxtags = afterddff_strcpytomulti(linenums, line, 1);
			if (type == ADD_TAGS && (maxtags == 0 || min == -1)) {
				fprintf(fpt, "%s", line);
				memset(line, 0, sizeof line);
				continue;
			}
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
				if (tagnum >= min && type == ADD_TAGS) {
					l++;
					snprintf(numstrnew, 12, " %d", 
							tagnum+addby);
				} else if (type == ADD_TAGS || 
						(!needrm && type >= 1)) {
					if (tagnum < rm_tagsnums[0] ||
							type == ADD_TAGS) {
						snprintf(numstrnew, 12, " %d",
								tagnum);
					} else {
						l++;
						snprintf(numstrnew, 12, " %d",
								tagnum+addby);
					}
				} else if (type >= 1) {
					memcpy(numstrnew, "\0", 2);
					l++;
				}
				strncat(strtoformat2, numstrnew, 
						strlen(numstrnew));
			}
			if (l == 0) {	/* If no change needed */
				fprintf(fpt, "%s", line);
			} else {
				rmdubspaces(strtoformat3, strtoformat2);
				maxtags = afterddff_tagsamountout(strtoformat3,
						1);
				if (maxtags > -1) {
					fprintf(fpt, "%s\n", strtoformat3);
				} else {
					printf("WARNING: Filename has no tags,"
							" won't be included."
							"\n");
				}
				memset(strtoformat3, 0, sizeof strtoformat3);
			}
			memset(strtoformat2, 0, sizeof strtoformat2);
			memset(line, 0, sizeof line);
			continue;
		}
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	fclose(fpt);

	if (validcategory == 0) {
		printf("The category provided isn't valid\n");
		return; 
	}

	RENAME(FILETARGETTEMP, FILETARGET);
}

/* Returns string after colon */
char *afterdd_strreturn(const char src[])
{
	int i=0;
	char *dest = malloc(B_BUFFER);
	while (*src++ != ':')
		;
	while ((dest[i++] = *src++) != '\n')
		;
	dest[i] = '\0';
	return dest;
}

/* Renames tag or category */
void rename_tc(int argc, char *argv[], char *str_rntype)
{
	if (argc != 5) {
		printf("Error: You must state only one replacement name and"
				" only one current %s name\n", str_rntype);
		return;
	} else if (!(checkifexist(FILETARGET)))
		return;
	else if (!strcmp(argv[3], argv[4])) {
		printf("Error: Replacement name the same as original name\n");
		return;
	}

	FILE *fp;
	FILE *fpt;

	char line[B_BUFFER];

	int type = CATEGORY;
	char strcompare[B_BUFFER];
	int renamed = 0;
	int ltagsmax = 0, j;
	int rntype = !(strcmp(str_rntype, "category")) ? CAT_NAME : TAG_NAME;
	char linetags[B_BUFFER][BUFFER];
	char temptag[B_BUFFER];
	int dup = 0;
	char linetagsformat[B_BUFFER] = "\0";

	fp = fopen(FILETARGET, "r");
	fpt = fopen(FILETARGETTEMP, "w");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			type = FILES;
		else if (type == CATEGORY)
			tilldd_strcpy(strcompare, line);
		if (type == CATEGORY && rntype == TAG_NAME)
			ltagsmax = afterddff_strcpytomulti(linetags, line, 0);
		if (type == CATEGORY && !renamed && 
				rntype == CAT_NAME && 
				strcmp(strcompare, argv[4]) == 0) {
			printf("FOUND: \"%s\", replacement: \"%s\"\n", argv[4],
					argv[3]);
			char *afdline = afterdd_strreturn(line);
			fprintf(fpt, "%s:%s", argv[3], afdline);
			renamed = 1;
			continue;
		} else if (type == CATEGORY && !renamed &&
				rntype == TAG_NAME &&
				ltagsmax > 0) {
			if (checkdup(linetags, argv[3], ltagsmax) != -1)
				dup = 1;
			for (j=0; j<ltagsmax; j++) {
				if (!strcmp(linetags[j], argv[4]) && !dup) {
					printf("FOUND: \"%s\", replacement:"
							" \"%s\"\n", argv[4], 
							argv[3]);
					snprintf(linetags[j], 
							strlen(argv[3])+2, 
							" %s", argv[3]);
					renamed = 1;
				} else {
					memcpy(temptag, linetags[j], 
							strlen(linetags[j])+1);
					snprintf(linetags[j], 
							strlen(temptag)+2, 
							" %s", temptag);
				}
				strncat(linetagsformat, linetags[j],
						strlen(linetags[j]));
			}
			fprintf(fpt, "%s:%s\n", strcompare, linetagsformat);
			memset(linetagsformat, 0, sizeof linetagsformat);
			continue;
		} else if (type == CATEGORY && renamed &&
				rntype == TAG_NAME &&
				ltagsmax > 0) {
			if (checkdup(linetags, argv[3], ltagsmax) != -1)
				dup = 1;
		}
		fprintf(fpt, "%s", line);
	}
	fclose(fp);
	fclose(fpt);

	if (!renamed || dup) {
		printf("Error: Failed to rename from \"%s\" to \"%s\" due to"
				" attempt duplication or tag not found\n", 
				argv[4], argv[3]);
		return;
	}

	RENAME(FILETARGETTEMP, FILETARGET);
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
	printf("usage: {create | read | version | help | search | all |"
			" categories | show-tags | add %s | rename %s |"
			" remove %s}\n",
			usage_error[ADD], usage_error[RENAME],
			usage_error[REMOVE]);
}

void par2(int argc, char *argv[], int type)
{
	if (argc > 2) {
		if (TCHECK(2, "tags", ADD))
			ar_tags(argc, argv, ADD_TAGS);
		else if (TCHECK(2, "category",  ADD))
			addcategory(argc, argv);
		else if (TCHECK(2, "tags-to", ADD))
			ar_tagsfiles(argc, argv, ADD);
		else if (TCHECK(2, "tag", RENAME))
			rename_tc(argc, argv, "tag");
		else if (TCHECK(2, "category", RENAME))
			rename_tc(argc, argv, "category");
		else if (TCHECK(2, "tags", REMOVE))
			ar_tags(argc, argv, RM_TAGS);
		else if (TCHECK(2, "category", REMOVE))
			ar_tags(argc, argv, RM_CAT);
		else if (TCHECK(2, "from-tags", REMOVE))
			ar_tagsfiles(argc, argv, REMOVE);
		else if (TCHECK(2, "files", REMOVE))
			removefiles(argc, argv);
		else
			printf("Error: You must give: %s\n",
					usage_error[type]);
	} else
		printf("Error: You must give at least one parameter:"
				" %s\n", usage_error[type]);
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (PCHECK(1, "read"))
			readfile();
		else if (PCHECK(1, "version"))
			printf("%s\n", VERSION);
		else if (PCHECK(1, "help"))
			usage();
		else if (PCHECK(1, "add"))
			par2(argc, argv, ADD);
		else if (PCHECK(1, "search"))
			searchtags(argc, argv);
		else if (PCHECK(1, "categories"))
			listcattags();
		else if (PCHECK(1, "create"))
			createfile();
		else if (PCHECK(1, "rename"))
			par2(argc, argv, RENAME);
		else if (PCHECK(1, "remove"))
			par2(argc, argv, REMOVE);
		else if (PCHECK(1, "all"))
			outallfiles();
		else if (PCHECK(1, "show-tags"))
			filesinfos(argc, argv);
		else
			usage();
	} else
		usage();
	return 0;
}

