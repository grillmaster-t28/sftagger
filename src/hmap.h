#ifndef HMAP_H
#define HMAP_H

#define HMAP_KEY_MAX		256
#define HMAP_VAL_MAX		1280
#define HMAP_WIDTH		10
#define HMAP_ARRAY_LENGTH	60

typedef struct {
	char	key[HMAP_KEY_MAX];
	char	value[HMAP_VAL_MAX];
} item;

typedef struct {
	item		items[HMAP_WIDTH][HMAP_ARRAY_LENGTH];
	unsigned int	next[HMAP_WIDTH];
} hmap;

hmap hmap_init(void);
void hmap_set(hmap *hm, const char *key, const char *value);
void hmap_setcat(hmap *hm, const char *key, const char *value);
const char *hmap_get(hmap *hm, const char *key);
void hmap_print(hmap *hm);
#endif /* HMAP_H */
