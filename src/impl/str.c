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

void
strucat(char *dest, const char *src)
{
	while (*dest)
		++dest;

	/* Put in comma before concatenation */
	*dest++ = ',';

	while ((*dest++ = *src++) != '\0')
		;

	*dest = '\0';
}

void
strucpy(char *dest, const char *src)
{
	while ((*dest++ = *src++) != '\0')
		;
	*dest = '\0';
}

