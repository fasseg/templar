#include "templar.h"

static struct option long_opts[] = {
    {"help", no_argument, 0, 'h'} 
};

int main(int argc, char **argv) {
	int opt;
	int opt_idx = 0;

	while ((opt = getopt_long(argc, argv, "h", long_opts, &opt_idx)) > 0) {
		switch(opt) {
			case 'h': 
				show_help(argv[0]);
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

    size_t dir_len = sizeof(char) * strlen(name);
    size_t file_len = sizeof(char) * strlen(name) + sizeof(char) * 2;

    if (create_dir(name) < 0 ) {
        templar_error("Unable to create skeleton directory");
    }
    
    char *hdr_name = malloc(file_len + 1);
    snprintf(hdr_name, file_len + 1, "%s.h", name);
    if (create_hdr(name, dir_len, hdr_name, file_len) < 0) {
        templar_error("Unable to create header file");
    }   

    char *src_name = malloc(file_len + 1);
    snprintf(src_name, file_len + 1, "%s.c", name);
    if (create_src(name, dir_len, src_name, file_len) < 0) {
        templar_error("Unable to create source file");
    }

    if (create_make(name, dir_len, name) < 0) {
       templar_error("Unable to create Makefile");
    }

    free(hdr_name);
    free(src_name);    
    return 0;    
}

int create_make(const char *dir_name, size_t dir_len, const char *name) {
    char *path = malloc(dir_len + sizeof(char) * 9 + 1);
    snprintf(path, dir_len + sizeof(char) * 9 + 1, "%s/Makefile", dir_name);

    if (access(path, F_OK) != -1) {
        templar_error("The make file does already exist");
    }

    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        templar_error("Unable to open make file");
    }

    fprintf(fp, "CC=gcc\nCFLAGS=-I. -g\nDEPS=%s.h\n", dir_name);
    fprintf(fp, "\n%%.o: %%.c $(DEPS)\n\t$(CC) -g -c -o $@ $< $(CFLAGS)\n\n");
    fprintf(fp, "all: %s.o\n\t$(CC) $(CFLAGS) -o %s %s.o\n\n", dir_name, dir_name, dir_name);
    fprintf(fp, "clean:\n\trm %s.o\n\trm %s\n", dir_name, dir_name);

    if (fclose(fp) == EOF) {
        templar_error("Unable to close make file");
    }

    printf("Creating file %s\n", path);

    free(path);
    return 0;
}

int create_hdr(const char *dir_name, size_t dir_len, const char *name, size_t name_len) {
    
    char *path = malloc(dir_len + name_len + 3);
    snprintf(path, dir_len + name_len + 3, "%s/%s", dir_name, name);

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

int create_src(const char *dir_name, size_t dir_len, const char *name, size_t name_len) {
    
    char *path = malloc(name_len + name_len + 3);
    snprintf(path, dir_len + name_len + 3, "%s/%s", dir_name, name);

	if (access(path, F_OK) != -1) {
		templar_error("The source file does already exist");
	}

    printf("Creating file %s\n", path);

    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        templar_error("Unable tp open source file");
    }

    fprintf(fp, "#include \"%s.h\"\n", dir_name);
    fprintf(fp, "\nint main(int argc, char **argv) {\n");
    fprintf(fp, "}\n");

    if (fclose(fp) == EOF) {
        templar_error("Unable to close source file");
    }

    free(path);
    return 0;
}

int create_dir(const char *name) {
	printf("Creating directory %s\n", name);
    return mkdir(name, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
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
}
