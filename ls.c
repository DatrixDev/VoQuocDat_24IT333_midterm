#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "ls.h"
#include "fileinfo.h"
#include <sys/ioctl.h>
#include <unistd.h>
typedef struct
{
    char *name;
    char *path;
    struct stat st;
} FileEntry;

static const LsOptions *sort_options;

static char *copy_string(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);

    if (copy != NULL)
        memcpy(copy, s, len);

    return copy;
}

static char *join_path(const char *dir, const char *name)
{
    size_t a = strlen(dir);
    size_t b = strlen(name);

    char *result = malloc(a + b + 2);

    if (result == NULL)
        return NULL;

    snprintf(result, a + b + 2, "%s/%s", dir, name);
    return result;
}

static int visible_name(const char *name,
                        const LsOptions *opts)
{
    if (opts->show_all)
        return 1;

    if (opts->almost_all)
        return strcmp(name, ".") != 0 &&
               strcmp(name, "..") != 0;

    return name[0] != '.';
}

static int compare_times(const struct stat *a,
                         const struct stat *b,
                         const LsOptions *opts)
{
    struct timespec ta, tb;

    if (opts->time_access)
    {
        ta = a->st_atimespec;
        tb = b->st_atimespec;
    }
    else if (opts->time_change)
    {
        ta = a->st_ctimespec;
        tb = b->st_ctimespec;
    }
    else
    {
        ta = a->st_mtimespec;
        tb = b->st_mtimespec;
    }

    if (ta.tv_sec > tb.tv_sec)
        return -1;
    if (ta.tv_sec < tb.tv_sec)
        return 1;

    if (ta.tv_nsec > tb.tv_nsec)
        return -1;
    if (ta.tv_nsec < tb.tv_nsec)
        return 1;

    return 0;
}

static int compare_entries(const void *pa, const void *pb)
{
    const FileEntry *a = pa;
    const FileEntry *b = pb;

    int result = 0;

    if (sort_options->sort_size)
    {
        if (a->st.st_size > b->st.st_size)
            result = -1;
        else if (a->st.st_size < b->st.st_size)
            result = 1;
    }
    else if (sort_options->sort_time)
    {
        result = compare_times(&a->st, &b->st,
                               sort_options);
    }

    if (result == 0)
        result = strcmp(a->name, b->name);

    if (sort_options->reverse)
        result = -result;

    return result;
}

static void free_entries(FileEntry *entries, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(entries[i].name);
        free(entries[i].path);
    }

    free(entries);
}

static int add_entry(FileEntry **entries, int *count,
                     int *capacity, const char *name,
                     const char *path,
                     const struct stat *st)
{
    if (*count == *capacity)
    {
        int new_capacity =
            *capacity == 0 ? 16 : *capacity * 2;

        FileEntry *tmp = realloc(
            *entries,
            (size_t)new_capacity * sizeof(FileEntry));

        if (tmp == NULL)
            return -1;

        *entries = tmp;
        *capacity = new_capacity;
    }

    FileEntry *entry = &(*entries)[*count];

    entry->name = copy_string(name);
    entry->path = copy_string(path);

    if (entry->name == NULL || entry->path == NULL)
    {
        free(entry->name);
        free(entry->path);
        return -1;
    }

    entry->st = *st;
    (*count)++;

    return 0;
}

static long long total_blocks(FileEntry *entries,
                              int count,
                              const LsOptions *opts)
{
    long long total = 0;

    for (int i = 0; i < count; i++)
        total += entries[i].st.st_blocks;

    if (opts->kilo_blocks)
        total = (total + 1) / 2;

    return total;
}

static void print_column_entry(const FileEntry *entry,
                               const LsOptions *opts)
{
    if (opts->show_inode)
        printf("%llu ",
               (unsigned long long)entry->st.st_ino);

    if (opts->show_blocks)
        print_block_count(&entry->st, opts);

    print_display_name(entry->name, &entry->st, opts);
}

static int column_entry_width(const FileEntry *entry,
                              const LsOptions *opts)
{
    int width = (int)strlen(entry->name);

    if (opts->classify)
        width++;

    if (opts->show_inode)
        width += snprintf(NULL, 0, "%llu ",
                          (unsigned long long)entry->st.st_ino);

    if (opts->show_blocks)
    {
        long long blocks = entry->st.st_blocks;

        if (opts->kilo_blocks)
            blocks = (blocks + 1) / 2;

        width += snprintf(NULL, 0, "%lld ", blocks);
    }

    return width;
}
static int list_directory(const char *path,
                          const LsOptions *opts,
                          int show_header)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        perror(path);
        return EXIT_FAILURE;
    }

    FileEntry *entries = NULL;
    int count = 0;
    int capacity = 0;
    int result = EXIT_SUCCESS;

    struct dirent *de;

    while ((de = readdir(dir)) != NULL)
    {
        if (!visible_name(de->d_name, opts))
            continue;

        char *fullpath = join_path(path, de->d_name);

        if (fullpath == NULL)
        {
            result = EXIT_FAILURE;
            break;
        }

        struct stat st;

        if (lstat(fullpath, &st) == -1)
        {
            perror(fullpath);
            result = EXIT_FAILURE;
            free(fullpath);
            continue;
        }

        if (add_entry(&entries, &count, &capacity,
                      de->d_name, fullpath, &st) == -1)
        {
            fprintf(stderr, "myls: out of memory\n");
            result = EXIT_FAILURE;
            free(fullpath);
            break;
        }

        free(fullpath);
    }

    closedir(dir);

    if (!opts->no_sort && count > 1)
    {
        sort_options = opts;
        qsort(entries, (size_t)count,
              sizeof(FileEntry), compare_entries);
    }

    if (show_header)
        printf("%s:\n", path);

    if (opts->long_format || opts->show_blocks)
    {
        printf("total ");

        if (opts->human_readable && opts->long_format)
        {
            long long total_size = 0;

            for (int i = 0; i < count; i++)
                total_size += entries[i].st.st_size;

            print_human_size(total_size);
        }
        else
        {
            printf("%lld",
                   total_blocks(entries, count, opts));
        }

        putchar('\n');
    }

    if (!opts->long_format && count > 0)
    {

        int max_width = 0;

        for (int i = 0; i < count; i++)
        {
          int width = column_entry_width(&entries[i], opts);

            if (width > max_width)
                max_width = width;
        }

        int column_width = max_width + 1;
        int terminal_width = 80;
        struct winsize ws;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
            ws.ws_col > 0)
        {
            terminal_width = ws.ws_col;
        }

        int columns = terminal_width / column_width;

        if (columns < 1)
            columns = 1;

        int rows = (count + columns - 1) / columns;

        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < columns; col++)
            {
                int index = col * rows + row;

                if (index >= count)
                    continue;

                print_column_entry(&entries[index], opts);

                if (col < columns - 1 &&
                    index + rows < count)
                {
                    int width = column_entry_width(&entries[index], opts);

                    if (opts->classify)
                        width++;

                    for (int s = width; s < column_width; s++)
                        putchar(' ');
                }
            }

            putchar('\n');
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            print_file_info(entries[i].path,
                            entries[i].name,
                            &entries[i].st, opts);
        }
    }

    if (opts->recursive)
    {
        for (int i = 0; i < count; i++)
        {
            if (!S_ISDIR(entries[i].st.st_mode))
                continue;

            if (strcmp(entries[i].name, ".") == 0 ||
                strcmp(entries[i].name, "..") == 0)
                continue;

            printf("\n");

            if (list_directory(entries[i].path,
                               opts, 1) != EXIT_SUCCESS)
                result = EXIT_FAILURE;
        }
    }

    free_entries(entries, count);
    return result;
}

int list_paths(int count, char *paths[],
               const LsOptions *opts)
{
    int result = EXIT_SUCCESS;

    for (int i = 0; i < count; i++)
    {
        struct stat st;

        if (lstat(paths[i], &st) == -1)
        {
            perror(paths[i]);
            result = EXIT_FAILURE;
            continue;
        }

        if (i > 0)
            printf("\n");

        if (opts->directory_only || !S_ISDIR(st.st_mode))
        {
            print_file_info(paths[i], paths[i],
                            &st, opts);
        }
        else
        {
            int show_header = count > 1;

            if (list_directory(paths[i], opts,
                               show_header) != EXIT_SUCCESS)
                result = EXIT_FAILURE;
        }
    }

    return result;
}