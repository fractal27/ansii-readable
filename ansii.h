#ifndef __ANSII_H
#define __ANSII_H
#include <stdio.h>

enum ansii_types {
       BG, FG, FONT_SET, FONT_RESET, RESET,
       SCREEN, CURSOR, ALTBUF, ERASE
};

struct ansii_t;
struct hash;



unsigned long hash(char* key, struct hash* to_compute);

struct ansii_t erase_to_ansii(char* key, struct hash* keyhash);
struct ansii_t cursor_to_ansii(char* key, struct hash* keyhash);
struct ansii_t altbuf_to_ansii(char* key, struct hash* keyhash);
struct ansii_t scattr_to_ansii(char* key, struct hash* keyhash);
unsigned int fntattr_to_ansii(char* font_attr, struct hash* keyhash);
struct ansii_t color_to_ansii(char* color, struct hash* keyhash);
// struct ansii_t parse_ansii_type(enum ansi_types ansii_type,char* value);

int ansii_transform(FILE* from, FILE* to);

#endif // __ANSII_H
