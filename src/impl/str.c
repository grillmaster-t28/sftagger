int
strsize(const char *s)
{
	int i;

	for (i = 0; *s != '\0'; ++i, ++s)
	{
	}

	return i;
}

int
strsame(const char *a, const char *b)
{
	for (; *a != '\0' && *b != '\0'; ++a, ++b)
	{
		if (*a != *b)
		{
			return 0;
		}
	}

	if (*a != '\0' || *b != '\0')
	{
		return 0;
	}

	return 1;
}

/*
 * strucat - strcat but unsigned char and delimiates with SEP_ITEM (1)
 */
void
strucat(unsigned char *dest, const unsigned char *src)
{
	while (*dest)
		++dest;

	while (*dest++ = *src++)
		;

	/* SEP_ITEM */
	--dest;
	*dest++ = 1;
}

