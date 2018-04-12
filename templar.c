#include "templar.h"

static bool verbose = false;
static bool flag_getopt = false;
static bool flag_usage = false;
static char indent[] = {0x20, 0x20, 0x20, 0x20};

static struct option long_opts[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {"getopt", no_argument, 0, 'g'}
};

int main(int argc, char **argv) {
    int opt;
    int opt_idx = 0;

    while ((opt = getopt_long(argc, argv, "hvgu", long_opts, &opt_idx)) > 0) {
        switch(opt) {
            case 'h': 
                show_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                verbose = true;
                break;        
            case 'g':
                flag_getopt = true;
                break;
            case 'u':
                flag_usage = true;
                break;
        }
    }

    if (argc - optind != 1) {
        templar_error("Invalid number of arguments");
    }

    const char *name = argv[optind];
    if (strlen(name) < 1) {
        templar_error("Name can not be empty");
    }

    create_c_skel(name);

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
    fprintf(fp, "clean:\n\trm %s.o\n\trm %s\n\n", proj.name, proj.name);
    fprintf(fp, "install:\n\tinstall -m 755 -o root %s /usr/local/bin/\n", proj.name);

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

    if (flag_getopt) {
        fprintf(fp, "#include <getopt.h>\n");
    }

    fprintf(fp, "#define PROGRAM_NAME \"%c%s\"\n", toupper(*proj.name), (proj.name + 1));
    fprintf(fp, "#define PROGRAM_VERSION \"0.0.1\"\n");

    if (flag_usage) {
        fprintf(fp, "void show_help(char *progr_name);\n");
    }

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
    if (flag_getopt) {
        fprintf(fp, "%sint opt;\n", &indent);
        fprintf(fp, "%sint opt_idx = 0;\n\n", &indent);
        fprintf(fp, "%sstatic struct option opts[] = {\n", &indent);
        fprintf(fp, "%s%s{\"help\", no_argument, 0, 'h'}\n", &indent, &indent);
        fprintf(fp, "%s};\n\n", &indent);
        fprintf(fp, "%swhile((opt = getopt_long(argc, argv, \"h\", opts, &opt_idx)) > 0) {\n", &indent);
        fprintf(fp, "%s%sswitch(opt) {\n", &indent, &indent);
        fprintf(fp, "%s%s%scase 'h':\n", &indent, &indent, &indent);
        fprintf(fp, "%s%s%s%sshow_help(argv[0]);\n", &indent, &indent, &indent, &indent);
        fprintf(fp, "%s%s%s%sexit(EXIT_SUCCESS);\n", &indent, &indent, &indent, &indent);
        fprintf(fp, "%s%s%s%sbreak;\n", &indent, &indent, &indent, &indent);
        fprintf(fp, "%s%s}\n", &indent, &indent);
        fprintf(fp, "%s}\n", &indent);
    }

    fprintf(fp, "}\n");

    if (flag_usage) {
        fprintf(fp, "void show_help(char * progr_name) {\n");
        fprintf(fp, "\tprintf(\"%%s v%%s\\n\", PROGRAM_NAME, PROGRAM_VERSION);\n");
        fprintf(fp, "\tprintf(\"This is a short description of the program\\n\");\n");
        fprintf(fp, "\tprintf(\"Usage: %%s [options]\\n\", progr_name);\n");
        fprintf(fp, "\tprintf(\"Options:\\n\");\n");
        fprintf(fp, "\tprintf(\"\\t-h/--help\\t\\tShow this help dialog and exit\\n\");\n");
        fprintf(fp, "}\n");
    }

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

void templar_error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

void show_help(const char *prog_name) {
    printf("%s v%s\n\n", TEMPLAR_NAME, TEMPLAR_VERSION);
    printf("Templar generates a program skeleton for you\n\n");
    printf("Usage: %s [options] <name>\n", prog_name);
    printf("\nOptions:\n");
    printf("\t-h/--help\t\tShow this help dialog and exit\n");
    printf("\t-v/--verbose\t\tShow verbose output\n");
    printf("\t-g/--getopt\t\tInclude getopt skeleton\n");
    printf("\t-u/--usage\t\tInclude usage dialog\n");
    printf("\n");
}
