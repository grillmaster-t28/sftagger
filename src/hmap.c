#include "hmap.h"
#include "impl.h"

#include <stdio.h>

inline hmap
hmap_init(void)
{
	hmap hm = (const hmap){0};
	return hm;
}

static unsigned int
hmap_getHash(const char *key)
{
	unsigned int hash = 0;

	for (; *key != '\0'; ++key)
	{
		hash += *key;
	}

	return hash % HMAP_WIDTH;
}

void
hmap_setbase(hmap *hm, const char *key, const char *value, int flag)
{
	unsigned int hash = hmap_getHash(key);

	for (unsigned int i=0; i < hm->next[hash]; ++i)
	{
		if (strsame(hm->items[hash][i].key, key))
		{
			switch (flag)
			{
			case 0:
				strucpy(hm->items[hash][i].value, value);
				break;
			case 1:
				strucat(hm->items[hash][i].value, value);
				break;
			}
			return;
		}
	}

	strucpy(hm->items[hash][hm->next[hash]].key, key);
	strucpy(hm->items[hash][hm->next[hash]++].value, value);
}

inline void
hmap_set(hmap *hm, const char *key, const char *value)
{
	hmap_setbase(hm, key, value, 0);
}

inline void
hmap_setcat(hmap *hm, const char *key, const char *value)
{
	hmap_setbase(hm, key, value, 1);
}

const char *
hmap_get(hmap *hm, const char *key)
{
	unsigned int hash = hmap_getHash(key);
	for (unsigned int i=0; i < hm->next[hash]; ++i)
	{
		if (strsame(hm->items[hash][i].key, key))
		{
			return hm->items[hash][i].value;
		}
	}
	return NULL;
}

void
hmap_print(hmap *hm)
{
	for (unsigned int i=0; i < HMAP_WIDTH; ++i)
	{
		for (unsigned int j=0; j < hm->next[i]; ++j)
		{
			printf("%d %d | %s: %s\n", i, j,
					hm->items[i][j].key,
					hm->items[i][j].value);
		}
	}
}
