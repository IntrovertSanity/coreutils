#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <globals.h>


#define PROGRAM_NAME "uniq"

#define MAX_LINE_LENGTH 1024

void usage(int status, char* program_name) {
    assert(status == EXIT_SUCCESS);
    printf("Usage: %s [OPTION]... [INPUT [OUTPUT]]\n", program_name);
    puts("Filter adjacent matching lines from INPUT (or standard input),\nwriting to OUTPUT (or standard output).\n");
    puts("Options:");
    puts("  -c, --count           prefix lines by the number of occurrences");
    puts("  -d, --repeated        only print duplicate lines, one for each group");
    puts("  -u, --unique          only print unique lines");
    puts("  -i, --ignore-case     ignore differences in case when comparing");
    puts("  -s, --skip-chars=N    avoid comparing the first N characters");
    puts("  -f, --skip-fields=N   avoid comparing the first N fields");
    puts(HELP_OPTION_DESCRIPTION);
    puts(VERSION_OPTION_DESCRIPTION);
    exit(status);
}
void to_lower(char *str, int ignore_case) {
    if (ignore_case) {
        for (int i = 0; str[i]; i++) {
            str[i] = tolower((unsigned char)str[i]);
        }
    }
}

int compare_lines(const char *line1, const char *line2, int skip_chars, int skip_fields, int ignore_case) {
    
    if (skip_chars > 0) {
        line1 += skip_chars;
        line2 += skip_chars;
    }

    if (skip_fields > 0) {
        for (int i = 0; i < skip_fields; i++) {
            while (*line1 && !isspace((unsigned char)*line1)) line1++;  
            while (*line1 && isspace((unsigned char)*line1)) line1++;  
            while (*line2 && !isspace((unsigned char)*line2)) line2++;  
            while (*line2 && isspace((unsigned char)*line2)) line2++; 
        }
    }

    if (ignore_case) {
        return strcasecmp(line1, line2);
    } else {
        return strcmp(line1, line2);
    }
}

void uniq(FILE *input, FILE *output, int count, int repeated, int unique, int ignore_case, int skip_chars, int skip_fields) {
    char prev_line[MAX_LINE_LENGTH], current_line[MAX_LINE_LENGTH];
    int prev_count = 0;
    int first_line = 1;

    while (fgets(current_line, MAX_LINE_LENGTH, input)) {
        size_t len = strlen(current_line);
        if (len > 0 && current_line[len - 1] == '\n') {
            current_line[len - 1] = '\0';
        }

        to_lower(current_line, ignore_case);

        if (first_line) {
            strcpy(prev_line, current_line);
            prev_count = 1;
            first_line = 0;
            continue;
        }

        if (compare_lines(prev_line, current_line, skip_chars, skip_fields, ignore_case) == 0) {
            prev_count++;
        } else {
            if (unique) {
                if (prev_count == 1) {
                    fprintf(output, "%s\n", prev_line);
                }
            } else if (!repeated || prev_count > 1) {
                if (count) {
                    fprintf(output, "%6d %s\n", prev_count, prev_line); 
                } else {
                    fprintf(output, "%s\n", prev_line); 
                }
            }
            strcpy(prev_line, current_line);
            prev_count = 1;
        }
    }

    if (unique) {
        if (prev_count == 1) {
            fprintf(output, "%s\n", prev_line);
        }
    } else if (!repeated || prev_count > 1) {
        if (count) {
            fprintf(output, "%6d %s\n", prev_count, prev_line);
        } else {
            fprintf(output, "%s\n", prev_line);
        }
    }
}

int main(int argc, char *argv[]) {
    int count = 0, repeated = 0, unique = 0, ignore_case = 0, skip_chars = 0, skip_fields = 0;
    FILE *input = stdin, *output = stdout;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) {
                count = 1;
            } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--repeated") == 0) {
                repeated = 1;
            } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--unique") == 0) {
                unique = 1;
            } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ignore-case") == 0) {
                ignore_case = 1;
            } else if (strncmp(argv[i], "-s", 2) == 0 || strncmp(argv[i], "--skip-chars", 12) == 0) {
                skip_chars = atoi(argv[i] + 2);
            } else if (strncmp(argv[i], "-f", 2) == 0 || strncmp(argv[i], "--skip-fields", 13) == 0) {
                skip_fields = atoi(argv[i] + 2);
            } else if (strcmp(argv[i], "--help") == 0) {
                usage(EXIT_SUCCESS, argv[0]);
            }
        } else {
            input = fopen(argv[i], "r");
            if (!input) {
                fprintf(stderr, "Error: Unable to open input file %s\n", argv[i]);
                exit(1);
            }
        }
    }

    uniq(input, output, count, repeated, unique, ignore_case, skip_chars, skip_fields);

    if (input != stdin) {
        fclose(input);
    }

    return 0;
}