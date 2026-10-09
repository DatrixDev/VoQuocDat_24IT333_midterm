#ifndef LS_H
#define LS_H

#include <time.h>

typedef struct {
    int show_all;
    int almost_all;
    int directory_only;
    int long_format;
    int numeric_ids;
    int reverse;
    int show_inode;
    int show_blocks;
    int kilo_blocks;
    int human_readable;
    int classify;
    int no_sort;
    int question_marks;
    int recursive;
    int sort_size;
    int sort_time;
    int time_access;
    int time_change;
    int wide_output;
} LsOptions;

int list_paths(int count, char *paths[], const LsOptions *opts);

#endif