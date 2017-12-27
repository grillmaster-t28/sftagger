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

#define VERSION "PRE-RELEASE 2017/12/28 (1.0 RC2)"

#define CATEGORY 0
#define FILES 1

#define FALSE 0
#define TRUE 1

int str_sort(char string[][BUFFER], int size, int type);
void multi_strtolower(char string[][BUFFER], int size);
void multi_strcpy(char dest[][BUFFER], char src[][BUFFER], int size);
void multi_strclr(char string[][BUFFER], int size);
void multi_strprint(char string[][BUFFER], int size);
void tilldd_strcpy(char *dest, char *src);
int afterddff_strcpytomulti(char dest[][BUFFER], char *src, int mode);
void multistrcat(char dest[][BUFFER], char src[][BUFFER], int start, int end);
int checkifexist(char *file);
void strcpy_wonl(char *dest, char *src);
void strcat_as(char *dest, char *add);
void fnol_strcpy(char *dest, char *src);
int gettags_wfilter(int numtags[BUFFER], char alltags[][BUFFER], char *filename, int ctagstotal);
void strcpy_qm(char *dest, char *src);
int * intcpy(int *src, size_t len);
int hasspace(char *src);
int dirtomstr(char *dirname, char dest[][BUFFER]);
void writefile(char *filename, char flines[][BUFFER], int max);
void rmdubspaces(char *dest, char *src);

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
	} else
		usage();
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
			printf("Error: You must give: {tags | category | tags-to}\n");
	} else
		printf("Error: You must give at least one parameter: {tags | category | tags-to}\n");
}

void list(int argc, char *argv[])
{
	if (argc > 2) {
		if (strcmp(argv[2], "categories") == 0)
			listcattags(argc, argv, 0);
		else if (strcmp(argv[2], "tags-of") == 0)
			listcattags(argc, argv, 1);
		else if (strcmp(argv[2], "all") == 0)
			listcattags(argc, argv, 2);
		else
			printf("Error: You must give: {categories | tags-of | all}\n");
	} else
		printf("Error: You must give at least one parameter: {categories | tags-of | all}\n");
}

void listcattags(int argc, char *argv[], int type)
{
	FILE *fp;
	char line[B_BUFFER];

	char category[B_BUFFER];
	int i;

	if ((fp = fopen("tags", "r")) != NULL) {
		while (fgets(line, B_BUFFER, fp) != NULL) {
			if (strcmp(line, "\n") == 0)
				break;
			tilldd_strcpy(category, line);
			if (type == 0)
				printf("%s ", category);
			else if (type == 1) {
				for (i=3; i<argc; i++)
					if (strcmp(category, argv[i]) == 0) {
						printf("%s", line);
						break;
					}
			} else if (type == 2)
				printf("%s", line);
			memset(line, 0, sizeof line);
		}
		if (type == 0)
			putchar('\n');
		fclose(fp);
	} else
		printf("File \"tags\" doesn't exit in current directory\n");
}

void readfile(void)
{
	FILE *fp;
	char line[B_BUFFER];

	if ((fp = fopen("tags", "r")) != NULL) {
		while (fgets(line, B_BUFFER, fp) != NULL) {
			printf("%s", line);
			memset(line, 0, sizeof line);
		}
		fclose(fp);
	} else 
		printf("File \"tags\" doesn't exit in current directory\n");
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
	int tags_amount;
	int tagsnums[BUFFER];
	char tagstosearch[B_BUFFER][BUFFER];

	/* In-line variables */
	int ltagstotal = 0;
	char linetags[B_BUFFER][BUFFER];
	char str_num[BUFFER];
	int filematches = 0;

	if (argc <= 2) {
		printf("Error: You must state at least one tag\n");
		return;
	}
	if (!(checkifexist("tags"))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	for (i=0; i<argc-2; i++)
		strcpy(tagstosearch[i], argv[i+2]);
	/* Get and exclude tags in file as well as assign to number */
	tags_amount = gettags_wfilter(tagsnums, tagstosearch, "tags", argc-2);

	fp = fopen("tags", "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			mode = FILES;
		else if (mode == FILES) {
			ltagstotal = afterddff_strcpytomulti(linetags, line, 1);
			if (ltagstotal > 0) {
				filematches = 0;
				for (i=0; i<ltagstotal; i++) {
					for (j=0; j<tags_amount; j++) {
						sprintf(str_num, "%d", tagsnums[j]);
						if (strcmp(linetags[i], str_num) == 0) {
							filematches++;
							break;
						}
					}
				}
				if (filematches == tags_amount)
					fnol_strcpy(slines[k++], line);
			}
		}
		memset(line, 0, sizeof line);
	}
	fclose(fp);
	if (k == 0)
		printf("Notice: No files found with the tags given\n");
	else {
		for (i=0; i<k; i++) {
			if (hasspace(slines[i])) {
				printf("\"%s\" ", slines[i]);
			} else
				printf("%s ", slines[i]);
		}
		putchar('\n');
	}
}

/* Add tags */
void addtags(int argc, char *argv[])
{
	FILE *fp;

	/* File reading variables */
	char flines[B_BUFFER][BUFFER];
	int i = 0, j, k, total_tags = 0;
	char line[B_BUFFER];

	int mode = CATEGORY;
	char strcompare[B_BUFFER];
	char strtoformat[B_BUFFER];
	char linetags[B_BUFFER][BUFFER];

	/* Duplicate checking */
	int duplicate = 0;
	int validcategory = 0;

	char reject[B_BUFFER][BUFFER];
	int t=0, dubcheck=0;
	char strtoformat2[B_BUFFER];
	char strtoformat3[B_BUFFER];
	char strtocat[B_BUFFER];
	int linechanged = 0;

	/* Tags updating */
	struct oldnewtag {
		int old;
		int new;
	} numtags[B_BUFFER];
	int l=0, m=0, si=0;
	int needmod = 0;	/* If it need modification */

	/* Numbers modification */
	char linenums[B_BUFFER][BUFFER];
	char numstrold[S_BUFFER];
	char numstrnew[S_BUFFER];
	int changetonew = 0;

	if (argc <= 4) {
		printf("Error: You must state at least one tag and a category\n");
		return;
	}
	if (!(checkifexist("tags"))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	fp = fopen("tags", "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			break;
		/* mode == CATEGORY */
		tilldd_strcpy(strcompare, line);
		total_tags = afterddff_strcpytomulti(linetags, line, 0);
		if (needmod == 1)
			for (j=0; j<total_tags; m++, l++, j++, si++) {
				for (k=3; k<argc-1; k++)
					if (strcmp(argv[k], linetags[j]) == 0)
						break;
				if (k == argc-1) {	/* Not a duplicate */
					numtags[si].old = m;
					numtags[si].new = l;
				} else
					l--;
			}
		else
			for (j=0; j<total_tags; j++) {
				for (k=3; k<argc-1; k++) {
					if (strcmp(argv[k], linetags[j]) == 0)
						break;
				}	/* Ignore non edited categories */
				if (k == argc-1) {
					m++;
					l++;
				} else
					l--;
			}
		if (strcmp(strcompare, argv[argc-1]) == 0) {
			needmod = 1;
			for (j=3; j<argc-1; l++, j++)
				;	/* Ignore adding tags */
		}
		memset(line, 0, sizeof line);
	}

	fp = fopen("tags", "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		/* newline changes to file mode */
		if (strcmp(line,"\n") == 0)
			mode = FILES;
		else if (mode == CATEGORY) {
			tilldd_strcpy(strcompare, line);
			/* accept if category not there */
			total_tags = afterddff_strcpytomulti(linetags, line, 0);
			if (strcmp(strcompare, argv[argc-1]) == 0)  {
				validcategory = 1;
				strcpy_wonl(strtoformat, line);
				/* Gets tags (after the double dots) */
				/* Skips duplicate checking if 0 */
				if (total_tags == 0) {
					for (j=3; j<argc-1; j++) 
						strcat_as(strtoformat, argv[j]);
				} else {
					for (j=3; j<argc-1; j++) {
						/* Duplicate checking */
						for (k=0; k<total_tags; k++) {
							if (strcmp(linetags[k], argv[j]) == 0) {
								printf("Duplicate \"%s\" won't be put in\n", linetags[k]);
								duplicate = 1;
								break;
							}
						}
						if (duplicate == 0)
							strcat_as(strtoformat, argv[j]);
					duplicate = 0;
					}
				}
				dubcheck = i;
				strcat(strtoformat, "\n");
				strcpy(flines[i++], strtoformat);
				memset(line, 0, sizeof line);
				continue;
			} else {
				/* Skips duplicate checking if 0 */
				if (total_tags > 0) {
					for (j=3; j<argc-1; j++) {
						/* Duplicate checking */
						for (k=0; k<total_tags; k++) {
							if (strcmp(linetags[k], argv[j]) == 0) {
								printf("Duplicate \"%s\" won't be put in\n", linetags[k]);
								duplicate = 1;
								break;
							}
						}
						if (duplicate == 1)
							strcpy(reject[t++], argv[j]);
						duplicate = 0;
					}
				}
			}
		} else if (mode == FILES) {
			// WORK ON
			total_tags = afterddff_strcpytomulti(linenums, line, 1);
			l = 0;
			if (total_tags > 0) {
				fnol_strcpy(strtoformat2, line);
				for (k=0; k<total_tags; k++) {
					changetonew = 0;
					for (j=0; j<si; j++) {
						sprintf(numstrold, "%d", numtags[j].old);
						if (strcmp(linenums[k], numstrold) == 0) {
							sprintf(numstrnew, "%d ", numtags[j].new);
							changetonew = 1;
							l++;
							break;
						}
					}
					if (changetonew == 0)
						sprintf(numstrnew, "%s ", linenums[k]);
					strcat(strtoformat2, numstrnew);
				}
				if (l == 0) {	/* If no change needed */
					rmdubspaces(strtoformat3, line);
					strcpy(flines[i++], strtoformat3);
				} else {
					strcat(strtoformat2, "\n");
					rmdubspaces(strtoformat3, strtoformat2);
					strcpy(flines[i++], strtoformat3);
				}
				memset(strtoformat2, 0, sizeof strtoformat2);
				memset(line, 0, sizeof line);
				continue;
			}
		}
		strcpy(flines[i++], line);
		memset(line, 0, sizeof line);
	}
	fclose(fp);

	if (validcategory == 0) {
		printf("The category provided isn't valid\n");
		return; 
	}

	/* Remove the rejects */
	memset(strtoformat2, 0, sizeof strtoformat2);
	tilldd_strcpy(strcompare, flines[dubcheck]);
	strcat(strcompare, ":");
	strcpy(strtoformat2, strcompare);
	for (j=0; j<t; j++) {
		total_tags = afterddff_strcpytomulti(linetags, strtoformat, 0);
		for (k=0; k<total_tags; k++) {
			if (strcmp(linetags[k], reject[j]) != 0) {
				strcpy(strtocat, " ");
				strcat(strtocat, linetags[k]);
				strcat(strtoformat2, strtocat);
			} else
				linechanged = 1;
		}
	}

	if (linechanged == 1) {
		strcat(strtoformat2, "\n");
		strcpy(flines[dubcheck], strtoformat2);
	}
	writefile("tags", flines, i);
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
	if (mode == 0 || ignoretill == '"')
		src++;
	for (int j=0; (dest[i][j] = *src) != '\0'; j++, *src++) {
		if (*src == ' ' || *src == '\n') {
			dest[i][j] = '\0';
			i++;	/* value reset */
			j = -1;
			if (*src == '\n')
				break;
		}
	}
	return i;	/* Number of tags */
}

void multistrcat(char dest[][BUFFER], char src[][BUFFER], int start, int end)
{
	int j = 0;
	printf("multistrcat: i: %d k: %d\n", start, end);	// TEST
	while (start < end)
		strcpy(dest[start++], src[j++]);
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

	if ((fp = fopen(file, "r")) != NULL) {
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
	int i = 0, j, k = 0, m;
	char line[B_BUFFER];

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
	while (fgets(line, B_BUFFER, fp) != NULL) {
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
		memset(line, 0, sizeof line);
	}	
	fclose(fp);
	writefile("tags", flines, i);
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
	int strchanged = 0;

	/* Duplication checking variables */
	char linetags[B_BUFFER][BUFFER];
	int ltagstotal = 0;
	int *linetagsnums;

	if (argc <= 3) {
		printf("Error: You must state at least one filename\n");
		return;
	}
	if (!(checkifexist("tags"))) {
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
	tags_amount = gettags_wfilter(tagsnums, tagstoadd, "tags", g);
	if (tags_amount == -1) {		/* Read error */
		printf("Error: Fail reading file\n");
		return;
	} else if (tags_amount == 0) {		/* No valid tags error */
		printf("Error: No valid tags, add them in before you apply"
				" them\n");
		return;
	}

	fp = fopen("tags", "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0) 
			mode = FILES;
		else if (mode == FILES) {
			strchanged = 0;
			fnol_strcpy(filenameofline, line);
			for (j=3; j<argc; j++) {
				if (strcmp(argv[j], filenameofline) == 0) {
					printf("FOUND: \"%s\"\n", filenameofline);	// PRINT
					strcpy_wonl(newline, line);
					linetagsnums = intcpy(tagsnums, tags_amount);
					ltagstotal = afterddff_strcpytomulti(linetags, line, 1);
					/* Duplication checking */
					if (ltagstotal > 0) {
						for (k=0; k<tags_amount; k++)
							for (l=0; l<ltagstotal; l++) {
								sprintf(str_num, "%d", linetagsnums[k]);
								if (strcmp(str_num, linetags[l]) == 0)
									linetagsnums[k] = -1;
							}
						for (k=0; k<tags_amount; k++) {
							if (linetagsnums[k] != -1) {
								sprintf(str_num, " %d", linetagsnums[k]);
								strcat(newline, str_num);
							}
						}
					}
					strcat(newline, "\n");
					// WORK ON
					rmdubspaces(newlinefixed, newline);
					strcpy(flines[i++], newlinefixed);
					strchanged = 1;
					memset(newline, 0, sizeof newline);
					break;
				}
			}
			if (strchanged == 1)
				continue;	/* Skips to next iteration of loop */
		} 
		strcpy(flines[i++], line);
		memset(line, 0, sizeof line);
	}

	fclose(fp);
	writefile("tags", flines, i);
}

int gettags_wfilter(int numtags[BUFFER], char curtags[][BUFFER], char *filename, int ctagstotal)
{
	FILE *fp;

	/* File reading variables */
	char line[B_BUFFER];

	char linetags[B_BUFFER][BUFFER];	/* Tags compared to per line */
	char allowedtags[B_BUFFER][BUFFER];	/* Tags been allowed */
	int i, j, k=0, n=0;
	int ltagstotal = 0;		/* Max item value of in-line tags */

	if (!(checkifexist(filename))) 
		return -1;		/* File reading error */

	fp = fopen(filename, "r");
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			break;		/* End of tags reading */
		ltagstotal = afterddff_strcpytomulti(linetags, line, 0);
		for (j=0; j<ltagstotal; j++, n++) 
			for (i=0; i<ctagstotal; i++) 
				if (strcmp(curtags[i], linetags[j]) == 0) {
					strcpy(allowedtags[k], curtags[i]);
					numtags[k++] = n;
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
void fnol_strcpy(char *dest, char *src)
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

	if (!(checkifexist("tags"))) {
		printf("Error: You need to create a \"tags\" file first\n");
		return;
	}

	fp = fopen("tags", "r");
	newmax = dirtomstr("./", updatedls);
	while (fgets(line, B_BUFFER, fp) != NULL) {
		if (strcmp(line, "\n") == 0)
			mode = FILES;
		else if (mode == FILES) {
			fnol_strcpy(oldls[j], line);
			if (hasspace(oldls[j])) {
				strcpy_qm(qmoldls, oldls[j]);
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
	writefile("tags", flines, max);
}

/* Write new modification to file */
void writefile(char *filename, char flines[][BUFFER], int max)
{
	FILE *fp;

	fp = fopen("tags", "w");
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
	int choice, answer;

	/* sorting */
	char ep_dname_ls[B_BUFFER][BUFFER];
	int itr, upto;

	/* Check if file exists */
	if ((fp = fopen("tags", "r")) != NULL) {
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
	itr = dirtomstr("./", ep_dname_ls);
	if (itr == -1)
		perror("Couldn't open current directory");
	else {
		upto = str_sort(ep_dname_ls, itr, 0);
		/* Category placeholder */
		fprintf(fp, "category: placeholder\n\n");
		for (itr = 0; itr < upto; itr++) 
			fprintf(fp, "%s\n", ep_dname_ls[itr]);
	}
	
	fclose(fp);
}

int dirtomstr(char *dirname, char dest[][BUFFER])
{
	/* Directory reading */
	DIR *dp;
	struct dirent *ep;
	int itr = 0;

	dp = opendir(dirname);
	if (dp != NULL) {
		while ((ep = readdir(dp)) != NULL) {
			if (hasspace(ep->d_name))
				strcpy_qm(dest[itr++], ep->d_name);
			else if (ep->d_name[0] != '.' && 
					strcmp(ep->d_name, "tags") != 0)
				strcpy(dest[itr++], ep->d_name);
				/* file not . or itself */
		}
		(void) closedir(dp);
		return itr;
	} else
		return -1;
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

/* Used if the filename has space, puts in quotation marks */
void strcpy_qm(char *dest, char *src)
{
	*dest++ = '"';
	while ((*dest++ = *src++) != '\0')
		;
	*--dest = '"';
	*++dest = '\0';
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

