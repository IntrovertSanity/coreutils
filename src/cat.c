#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define PROGRAM_NAME "echo"

#define BUFFER_SIZE (1 * 1024 * 1024)

void usage(int status, char* program_name) {
    assert(status == EXIT_SUCCESS);
    printf("Usage: %s [SHORT-OPTION]... [STRING]...\nor:  %s LONG-OPTION\n", program_name, program_name);
    puts("Concatenate FILE(s) to standard output.\n");
    puts ("-n, --number             number all output lines\n");
    puts(HELP_OPTION_DESCRIPTION);
    puts(VERSION_OPTION_DESCRIPTION);
    printf ("\
\n\
Examples:\n\
  %s f - g  Output f's contents, then standard input, then g's contents.\n\
  %s        Copy standard input to standard output.\n\
",
              program_name, program_name);

    exit(status);
}

void do_read(char* filename, int number_lines) {
    FILE* file;
    if (strcmp(filename, "-") == 0) {
        file = stdin;
    } else {
        file = fopen(filename, "r");
        if (!file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    if (number_lines) {
        char line[4096];
        int line_num = 1;
        while (fgets(line, sizeof(line), file)) {
            printf("%6d  %s", line_num++, line);
        }
    } else {
        char buffer[BUFFER_SIZE];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            fwrite(buffer, 1, bytesRead, stdout);
        }
    }

    if (file != stdin) {
        fclose(file);
    }
}

int main(int argc, char *argv[]) {
    int number_lines = 0;

    int argi = 1;
    while (argi < argc) {
        if (strcmp(argv[argi], "--help") == 0 || strcmp(argv[argi], "-h") == 0) {
            usage(EXIT_SUCCESS, argv[0]);
        } else if (strcmp(argv[argi], "--number") == 0 || strcmp(argv[argi], "-n") == 0) {
            number_lines = 1;
            argi++;
        } else {
            break;
        }
    }

    if (argi == argc) {
        do_read("-", number_lines);
    } else {
        while (argi < argc) {
            do_read(argv[argi], number_lines);
            argi++;
        }
    }

    return 0;
}
