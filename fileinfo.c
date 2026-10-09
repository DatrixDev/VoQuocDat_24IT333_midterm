#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fileinfo.h"

static long long blocks_used(const struct stat *st,
                             const LsOptions *opts)
{
    long long blocks = (long long)st->st_blocks;

    if (opts->kilo_blocks)
        blocks = (blocks + 1) / 2;

    return blocks;
}

void print_block_count(const struct stat *st,
                       const LsOptions *opts)
{
    printf("%lld ", blocks_used(st, opts));
}

void print_human_size(long long size)
{
    static const char *units[] = {
        "B", "K", "M", "G", "T", "P"
    };

    double value = (double)size;
    int unit = 0;

    while (value >= 1024.0 && unit < 5) {
        value /= 1024.0;
        unit++;
    }

    if (unit == 0)
        printf("%lldB", size);
    else
        printf("%.1f%s", value, units[unit]);
}

static char file_suffix(mode_t mode)
{
    if (S_ISDIR(mode))
        return '/';

    if (S_ISLNK(mode))
        return '@';

    if (S_ISFIFO(mode))
        return '|';

    if (S_ISSOCK(mode))
        return '=';

    if (S_ISREG(mode) &&
        (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
        return '*';

    return '\0';
}

void print_display_name(const char *name,
                        const struct stat *st,
                        const LsOptions *opts)
{
    const unsigned char *p =
        (const unsigned char *)name;

    while (*p != '\0') {
        if (opts->question_marks && !opts->wide_output &&
            (!isprint(*p) || *p == '\n' || *p == '\t')) {
            putchar('?');
        } else {
            putchar(*p);
        }
        p++;
    }

    if (opts->classify) {
        char suffix = file_suffix(st->st_mode);
        if (suffix != '\0')
            putchar(suffix);
    }
}

static void make_permissions(mode_t mode, char output[11])
{
    strcpy(output, "----------");

    if (S_ISDIR(mode)) output[0] = 'd';
    else if (S_ISLNK(mode)) output[0] = 'l';
    else if (S_ISCHR(mode)) output[0] = 'c';
    else if (S_ISBLK(mode)) output[0] = 'b';
    else if (S_ISFIFO(mode)) output[0] = 'p';
    else if (S_ISSOCK(mode)) output[0] = 's';

    const mode_t bits[] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };

    const char symbols[] = "rwxrwxrwx";

    for (int i = 0; i < 9; i++) {
        if (mode & bits[i])
            output[i + 1] = symbols[i];
    }

    if (mode & S_ISUID)
        output[3] = (mode & S_IXUSR) ? 's' : 'S';

    if (mode & S_ISGID)
        output[6] = (mode & S_IXGRP) ? 's' : 'S';

    if (mode & S_ISVTX)
        output[9] = (mode & S_IXOTH) ? 't' : 'T';
}

static time_t selected_time(const struct stat *st,
                            const LsOptions *opts)
{
    if (opts->time_access)
        return st->st_atime;

    if (opts->time_change)
        return st->st_ctime;

    return st->st_mtime;
}

void print_file_info(const char *path, const char *name,
                     const struct stat *st,
                     const LsOptions *opts)
{
    if (opts->show_inode)
        printf("%llu ",
               (unsigned long long)st->st_ino);

    if (opts->show_blocks)
        print_block_count(st, opts);

    if (!opts->long_format) {
        print_display_name(name, st, opts);
        putchar('\n');
        return;
    }

    char permissions[11];
    make_permissions(st->st_mode, permissions);

    char owner[64];
    char group[64];

    struct passwd *pw = getpwuid(st->st_uid);
    struct group *gr = getgrgid(st->st_gid);

    if (opts->numeric_ids || pw == NULL)
        snprintf(owner, sizeof(owner), "%u",
                 (unsigned)st->st_uid);
    else
        snprintf(owner, sizeof(owner), "%s", pw->pw_name);

    if (opts->numeric_ids || gr == NULL)
        snprintf(group, sizeof(group), "%u",
                 (unsigned)st->st_gid);
    else
        snprintf(group, sizeof(group), "%s", gr->gr_name);

    char date[64];
    time_t timestamp = selected_time(st, opts);
    struct tm *tm_info = localtime(&timestamp);

    if (tm_info != NULL)
        strftime(date, sizeof(date),
                 "%b %e %H:%M", tm_info);
    else
        strcpy(date, "?");

   printf("%s %2lu %s  %s  ", permissions,
       (unsigned long)st->st_nlink, owner, group);

    if (opts->human_readable)
        print_human_size((long long)st->st_size);
    else
        printf("%5lld", (long long)st->st_size);

    printf(" %s ", date);

    print_display_name(name, st, opts);

    if (S_ISLNK(st->st_mode)) {
        char target[4096];

        ssize_t n = readlink(path, target,
                             sizeof(target) - 1);

        if (n >= 0) {
            target[n] = '\0';
            printf(" -> %s", target);
        }
    }

    putchar('\n');
}