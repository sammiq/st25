/* plat_files.c - DOS style file names on a case sensitive file system, and
 * the C library's non-ISO string functions. */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#include "platform.h"

static char data_dir[1024] = ".";

void plat_set_data_dir(const char *dir)
{
    snprintf(data_dir, sizeof(data_dir), "%s", dir);
}

const char *plat_data_dir(void)
{
    return data_dir;
}

/* Find an entry of dir called name in any case.  Returns 1 and the real
 * name in out if there is one. */
static int find_entry(const char *dir, const char *name, char *out, size_t outsize)
{
    DIR *d;
    struct dirent *e;
    int found = 0;

    if ((d = opendir(dir)) == NULL)
        return 0;
    while ((e = readdir(d)) != NULL) {
        if (strcasecmp(e->d_name, name) == 0) {
            snprintf(out, outsize, "%s", e->d_name);
            found = 1;
            if (strcmp(e->d_name, name) == 0)
                break;
        }
    }
    closedir(d);
    return found;
}

/* Resolve a DOS path to a real one under the data directory.  Components
 * that do not exist are appended as given, so that a file can be created.
 * Returns 1 if the whole path exists. */
static int resolve(const char *path, char *out, size_t outsize)
{
    char comp[256];
    size_t len;
    int exists = 1;

    snprintf(out, outsize, "%s", data_dir);
    len = strlen(out);
    if (path[0] != 0 && path[1] == ':')         /* drive letter */
        path += 2;
    while (*path == '/' || *path == '\\')
        path++;
    while (*path) {
        char *c = comp;

        while (*path && *path != '/' && *path != '\\' && c < comp + sizeof(comp) - 1)
            *c++ = *path++;
        *c = 0;
        while (*path == '/' || *path == '\\')
            path++;
        if (comp[0] == 0 || strcmp(comp, ".") == 0)
            continue;
        if (len + 1 + strlen(comp) + 1 > outsize)
            return 0;
        if (exists) {
            char real[256];

            if (find_entry(out, comp, real, sizeof(real)))
                strcpy(comp, real);
            else
                exists = 0;
        }
        out[len++] = '/';
        strcpy(out + len, comp);
        len += strlen(comp);
    }
    return exists;
}

int plat_open(const char *path, int flags)
{
    char real[1100];

    resolve(path, real, sizeof(real));
    return open(real, flags, 0644);
}

int32_t plat_filelength(int fd)
{
    struct stat st;

    if (fstat(fd, &st) != 0)
        return -1;
    return (int32_t)st.st_size;
}

int plat_access(const char *path, int mode)
{
    char real[1100];

    resolve(path, real, sizeof(real));
    return access(real, mode);
}

void *plat_fopen(const char *path, const char *mode)
{
    char real[1100];

    resolve(path, real, sizeof(real));
    return fopen(real, mode);
}

int plat_scan_dir(const char *dir, void (*cb)(const char *name))
{
    char real[1100];
    DIR *d;
    struct dirent *e;

    if (!resolve(dir, real, sizeof(real)))
        return 0;
    if ((d = opendir(real)) == NULL)
        return 0;
    while ((e = readdir(d)) != NULL)
        if (e->d_name[0] != '.')
            cb(e->d_name);
    closedir(d);
    return 1;
}

char *strupr(char *s)
{
    char *p;

    for (p = s; *p; p++)
        *p = toupper((unsigned char)*p);
    return s;
}

char *strlwr(char *s)
{
    char *p;

    for (p = s; *p; p++)
        *p = tolower((unsigned char)*p);
    return s;
}
