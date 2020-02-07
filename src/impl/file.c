#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

const char *
homedir(void)
{
	const char *h = NULL;

	h = getenv("HOME");
	if (h == NULL)
	{
		h = getpwuid(getuid())->pw_dir;
	}

	return h;
}

void
setpath(char *path)
{
	sprintf(path, "%s/.config/sftagger/db", homedir());
}

int
create(void)
{
	struct stat st = {0};
	const char *hd = homedir();
	char homedir_c[256];
	char homedir_cs[256];
	char path[256];
	FILE *fp = NULL;
	unsigned char content[] = {
		3	/* End */
	};

	sprintf(homedir_c, "%s/.config/", hd);
	sprintf(homedir_cs, "%s/.config/sftagger/", hd);
	setpath(path);

	if (stat(homedir_c, &st) == -1)
	{
		mkdir(homedir_c, 0700);
	}

	if (stat(homedir_cs, &st) == -1)
	{
		mkdir(homedir_cs, 0700);
	}

	fp = fopen(path, "wb");

	if (fp == NULL)
	{
		return 1;
	}

	fwrite(content, 1, sizeof(content), fp);

	fclose(fp);
	return 0;
}

