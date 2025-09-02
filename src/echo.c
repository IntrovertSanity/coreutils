#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <globals.h>
#include <unistd.h>
#include <ctype.h>

#define PROGRAM_NAME "echo"

void
usage (int status, char* program_name)
{

    assert(status == EXIT_SUCCESS);
    printf ("Usage: %s [SHORT-OPTION]... [STRING]...\nor:  %s LONG-OPTION\n", program_name, program_name);
    puts ("Echo the STRING(s) to standard output.\n\n-n             do not output the trailing newline\n");
    puts ("-e             enable interpretation of backslash escapes\n-E             disable interpretation of backslash escapes (default)\n");
    puts (HELP_OPTION_DESCRIPTION);
    puts (VERSION_OPTION_DESCRIPTION);
    puts ("\
    \n\
If -e is in effect, the following sequences are recognized:\n\
    \n\
    ");
    puts ("\
    \\\\      backslash\n\
    \\a      alert (BEL)\n\
    \\b      backspace\n\
    \\c      produce no further output\n\
    \\e      escape\n\
    \\f      form feed\n\
    \\n      new line\n\
    \\r      carriage return\n\
    \\t      horizontal tab\n\
    \\v      vertical tab\n\
    ");
    puts ("\
    \\0NNN   byte with octal value NNN (1 to 3 digits)\n\
    \\xHH    byte with hexadecimal value HH (1 to 2 digits)\n\
    ");
    printf (USAGE_BUILTIN_WARNING, PROGRAM_NAME);
    puts ("\n\
Consider using the printf(1) command instead,\n\
as it avoids problems when outputting option-like strings.\n\
    ");
  exit (status);
}

void do_echo(int argc, char** argv) {
    for (int i = 0; i < argc; ++i) {
        // Skip options (arguments starting with '-')
        if (argv[i][0] == '-' && argv[i][1] != '\0') continue;
        printf("%s ", argv[i]);
        if (i < argc - 1) printf(" ");
    }
}


// https://github.com/coreutils/coreutils/blob/c6397d08725e651fe81fbbd91df2043674206865/src/echo.c#L93
static int
hextobin (unsigned char c)
{
  switch (c)
    {
    default: return c - '0';
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    }
}

int main(int argc, char** argv) {
    
    char do_v9 = 0;
    char display_return = 1;
    int opt;
    
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(EXIT_SUCCESS, argv[0]);
    }

    while ((opt = getopt(argc, argv, "eEn")) != -1) {
        switch (opt) {
            case 'e':
                do_v9 = 1;
                break;
            case 'E' :
                do_v9 = 0;
                break;
            case 'n':
                display_return = 0;
                break;
            default:
                usage(EXIT_FAILURE, argv[0]);
                break;
        }
    }

    // Only print non-option arguments (skip options)
    int first_arg = 0;
    for (int i = optind; i < argc; ++i) {
        if (do_v9) {
            char const *s = argv[i];
            unsigned char c;
            while ((c = *s++)) {
                if (c == '\\' && *s) {
                    switch (c = *s++) {
                        case 'a': c = '\a'; break;
                        case 'b': c = '\b'; break;
                        case 'c': return EXIT_SUCCESS;
                        case 'e': c = '\x1B'; break;
                        case 'f': c = '\f'; break;
                        case 'n':
                            putchar('\n');
                            first_arg = 0; // Reset so next arg does not get a space before
                            continue;      // Skip normal putchar(c) below
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'v': c = '\v'; break;
                        case 'x': {
                            unsigned char ch = *s;
                            if (!isdigit(ch))
                                goto not_an_escape;
                            s++;
                            c = hextobin(ch);
                            ch = *s;
                            if (isdigit(ch)) {
                                s++;
                                c = c * 16 + hextobin(ch);
                            }
                        }
                        break;
                        case '0':
                            c = 0;
                            if (!('0' <= *s && *s <= '7'))
                                break;
                            c = *s++;
                        case '1': case '2': case '3':
                        case '4': case '5': case '6': case '7':
                            c -= '0';
                            if ('0' <= *s && *s <= '7')
                                c = c * 8 + (*s++ - '0');
                            if ('0' <= *s && *s <= '7')
                                c = c * 8 + (*s++ - '0');
                            break;
                        case '\\': break;

                        not_an_escape:
                        default:  putchar('\\'); break;
                    }
                }
                putchar(c);
                // If we just printed a character and there are more args, print a space unless last char was '\n'
                if (*s == '\0' && i < argc - 1 && c != '\n')
                    putchar(' ');
            }
            
        } else {
            if (first_arg) putchar(' ');
            printf("%s ", argv[i]);
        }
        first_arg = 1;
    }

    if (display_return)
        putchar('\n');

    return 0;
}