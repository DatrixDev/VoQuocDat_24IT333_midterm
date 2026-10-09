#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ls.h"

static void usage(void)
{
    fprintf(stderr,
            "Usage: myls [-Aa c d F f h i k l n q R r S s t u w] [path ...]\n");
}

int main(int argc, char *argv[])
{
    LsOptions opts = {0};
    int opt;

    while ((opt = getopt(argc, argv,
                         "Aac dFfhiklnqRrSstuw")) != -1)
    {
        switch (opt)
        {
        case 'A':
            opts.almost_all = 1;
            opts.show_all = 0;
            break;

        case 'a':
            opts.show_all = 1;
            opts.almost_all = 0;
            break;

        case 'c':
            opts.time_change = 1;
            opts.time_access = 0;
            break;

        case 'd':
            opts.directory_only = 1;
            break;

        case 'F':
            opts.classify = 1;
            break;

        case 'f':
            opts.no_sort = 1;
            opts.show_all = 1;
            opts.almost_all = 0;
            break;

        case 'h':
            opts.human_readable = 1;
            break;

        case 'i':
            opts.show_inode = 1;
            break;

        case 'k':
            opts.kilo_blocks = 1;
            break;

        case 'l':
            opts.long_format = 1;
            opts.numeric_ids = 0;
            break;

        case 'n':
            opts.numeric_ids = 1;
            opts.long_format = 1;
            break;

        case 'q':
            opts.question_marks = 1;
            break;

        case 'R':
            opts.recursive = 1;
            break;

        case 'r':
            opts.reverse = 1;
            break;

        case 'S':
            opts.sort_size = 1;
            opts.sort_time = 0;
            break;

        case 's':
            opts.show_blocks = 1;
            break;

        case 't':
            opts.sort_time = 1;
            opts.sort_size = 0;
            break;

        case 'u':
            opts.time_access = 1;
            opts.time_change = 0;
            break;

        case 'w':
            opts.wide_output = 1;
            opts.question_marks = 0;
            break;

        default:
            usage();
            return EXIT_FAILURE;
        }
    }

    if (opts.no_sort)
    {
        opts.sort_size = 0;
        opts.sort_time = 0;
        opts.reverse = 0;
    }

    int path_count = argc - optind;

    if (path_count == 0)
    {
        char *default_paths[] = {"."};
        return list_paths(1, default_paths, &opts);
    }

    return list_paths(path_count, argv + optind, &opts);
}