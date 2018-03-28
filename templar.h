#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define TEMPLAR_VERSION "0.0.1"
#define TEMPLAR_NAME "Templar"
#define LANG_C 1


void show_help();

void templar_error(const char *msg);

int parse_lang(char *lang);

int create_c_skel(const char *name);

int create_src(const char *dir_name, size_t dir_len, const char *name, size_t name_len);

int create_dir(const char *name); 

int create_hdr(const char *dir_name, size_t dir_len, const char *name, size_t name_len);

int create_make(const char *dir_name, size_t dir_len, const char *name);
