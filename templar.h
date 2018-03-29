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

struct project {
    char *name;
    size_t name_len;
    char *dir_name;
    size_t dir_name_len;
    char *hdr_name;
    size_t hdr_name_len;
    char *src_name;
    size_t src_name_len;
};

typedef struct project project;

void show_help();

void templar_error(const char *msg);

int parse_lang(const char *lang);

int create_c_skel(const char *name);

int create_src(project proj);

int create_dir(project proj); 

int create_hdr(project proj);

int create_make(project proj);
