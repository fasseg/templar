#include "templar.h"

static int verbose = 0;

static struct option long_opts[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'}
};


int main(int argc, char **argv) {
    int opt;
    int opt_idx = 0;

    while ((opt = getopt_long(argc, argv, "hv", long_opts, &opt_idx)) > 0) {
        switch(opt) {
            case 'h': 
                show_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                verbose = 1;
                break;                
        }
    }

    if (argc - optind != 2) {
        templar_error("Invalid number of arguments");
    }

    int lang = parse_lang(argv[optind++]);
    if (lang < 0) {
        templar_error("Invalid language");
    }

    const char *name = argv[optind];
    if (strlen(name) < 1) {
        templar_error("Name can not be empty");
    }

    switch(lang) {
        case LANG_C:
            create_c_skel(name);
            break;
    }

    return 0;
}

int create_c_skel(const char *name) {

    project proj;

    proj.name_len = sizeof(char) * strlen(name);
    proj.name = malloc(proj.name_len + 1);
    proj.dir_name_len = proj.name_len;
    proj.dir_name = malloc(proj.dir_name_len + 1);
    proj.hdr_name_len = proj.name_len + sizeof(char) * 2;
    proj.hdr_name = malloc(proj.hdr_name_len + 1);
    proj.src_name_len = proj.name_len + sizeof(char) * 2;
    proj.src_name = malloc(proj.src_name_len + 1);

    memcpy(proj.name, name, proj.name_len + 1);
    strncpy(proj.dir_name, proj.name, proj.name_len + 1);
    snprintf(proj.hdr_name, proj.hdr_name_len + 1, "%s.h", proj.name);
    snprintf(proj.src_name, proj.src_name_len + 1, "%s.c", proj.name);

    if (verbose) {
        printf("Project name: %s\n", proj.name);
        printf("Directory name: %s\n", proj.dir_name);
        printf("Header file name: %s\n", proj.hdr_name);
        printf("Source file name: %s\n", proj.src_name);
    }

    size_t dir_len = sizeof(char) * strlen(name);
    size_t file_len = sizeof(char) * strlen(name) + sizeof(char) * 2;

    if (create_dir(proj) < 0 ) {
        templar_error("Unable to create skeleton directory");
    }

    if (create_hdr(proj) < 0) {
        templar_error("Unable to create header file");
    }   

    if (create_src(proj) < 0) {
        templar_error("Unable to create source file");
    }

    if (create_make(proj) < 0) {
        templar_error("Unable to create Makefile");
    }
    
    free(proj.name);
    free(proj.dir_name);
    free(proj.hdr_name);
    free(proj.src_name);
    return 0;    
}

int create_make(project proj) {
    char *path = malloc(proj.dir_name_len + sizeof(char) * 9 + 1);
    snprintf(path, proj.dir_name_len + sizeof(char) * 9 + 1, "%s/Makefile", proj.dir_name);

    if (access(path, F_OK) != -1) {
        templar_error("The make file does already exist");
    }

    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        templar_error("Unable to open make file");
    }

    fprintf(fp, "CC=gcc\nCFLAGS=-I. -g\nDEPS=%s.h\n", proj.dir_name);
    fprintf(fp, "\n%%.o: %%.c $(DEPS)\n\t$(CC) -g -c -o $@ $< $(CFLAGS)\n\n");
    fprintf(fp, "all: %s.o\n\t$(CC) $(CFLAGS) -o %s %s.o\n\n", proj.name, proj.name, proj.name);
    fprintf(fp, "clean:\n\trm %s.o\n\trm %s\n", proj.name, proj.name);

    if (fclose(fp) == EOF) {
        templar_error("Unable to close make file");
    }

    printf("Creating file %s\n", path);

    free(path);
    return 0;
}

int create_hdr(project proj) {

    char *path = malloc(proj.dir_name_len + proj.hdr_name_len + 2);
    snprintf(path, proj.dir_name_len + proj.hdr_name_len + 2, "%s/%s", proj.dir_name, proj.hdr_name);

    printf("Creating file %s\n", path);

    if (access(path, F_OK) != -1) {
        templar_error("The header file does already exist");
    }

    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        templar_error("Unable to create header file");
    }

    fprintf(fp, "#include <stdlib.h>\n#include <stdio.h>\n");

    if (fclose(fp) == EOF) {
        templar_error("Unable to close header file");
    } 

    free(path);
    return 0;
}

int create_src(project proj) {

    char *path = malloc(proj.dir_name_len + proj.src_name_len + 2);
    snprintf(path, proj.dir_name_len + proj.src_name_len + 2, "%s/%s", proj.dir_name, proj.src_name);

    if (access(path, F_OK) != -1) {
        templar_error("The source file does already exist");
    }

    printf("Creating file %s\n", path);

    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        templar_error("Unable tp open source file");
    }

    fprintf(fp, "#include \"%s.h\"\n", proj.dir_name);
    fprintf(fp, "\nint main(int argc, char **argv) {\n");
    fprintf(fp, "}\n");

    if (fclose(fp) == EOF) {
        templar_error("Unable to close source file");
    }

    free(path);
    return 0;
}

int create_dir(project proj) {
    printf("Creating directory %s\n", proj.dir_name);
    return mkdir(proj.dir_name, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

int parse_lang(char *lang) {

    int *ch = (int *) lang;

    // convert to lower case
    for (;*ch;++ch) *ch = tolower(*ch);

    if (strcmp(lang, "c") == 0) {
        return LANG_C;
    }

    return -1;
}

void templar_error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void show_help() {
    printf("%s v%s\n\n", TEMPLAR_NAME, TEMPLAR_VERSION);
    printf("Templar generates a program skeleton for you\n\n");
    printf("Usage: %s [options] <lang> <name>\n", TEMPLAR_NAME);
    printf("\nOptions:\n");
    printf("\t-h/--help\t\t\tShow this help dialog and exit\n");
    printf("\n");
}
