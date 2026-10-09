#ifndef FILEINFO_H
#define FILEINFO_H

#include <sys/types.h>
#include <sys/stat.h>
#include "ls.h"

void print_file_info(const char *path, const char *name,
                     const struct stat *st,
                     const LsOptions *opts);

void print_display_name(const char *name,
                        const struct stat *st,
                        const LsOptions *opts);

void print_block_count(const struct stat *st,
                       const LsOptions *opts);
void print_human_size(long long size);
#endif