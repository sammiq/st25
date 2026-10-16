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

/* The player's own files (saved games, the settings file) live under
 * $XDG_DATA_HOME/startrek, else ~/.local/share/startrek, not in the game data. */
static char user_dir_path[1024];
static int user_dir_ready;

static void make_dirs(char *path)
{
    char *p;

    for (p = path + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(path, 0755);
            *p = '/';
        }
    mkdir(path, 0755);
}

static void copy_file(const char *from, const char *to)
{
    FILE *in = fopen(from, "rb"), *out;
    char buf[4096];
    size_t n;

    if (!in)
        return;
    out = fopen(to, "wb");
    if (out) {
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, n, out);
        fclose(out);
    }
    fclose(in);
}

static int is_user_file(const char *path)
{
    return strchr(path, '/') == NULL && strchr(path, '\\') == NULL
        && (strncasecmp(path, "savegame.", 9) == 0 || strcasecmp(path, "intrplay.cfg") == 0);
}

static const char *user_dir(void)
{
    if (!user_dir_ready) {
        const char *xdg = getenv("XDG_DATA_HOME"), *home = getenv("HOME");
        DIR *d;
        struct dirent *e;

        user_dir_ready = 1;
        if (xdg && xdg[0] == '/')
            snprintf(user_dir_path, sizeof(user_dir_path), "%s/startrek", xdg);
        else if (home)
            snprintf(user_dir_path, sizeof(user_dir_path), "%s/.local/share/startrek", home);
        else
            snprintf(user_dir_path, sizeof(user_dir_path), "%s", data_dir);
        make_dirs(user_dir_path);
        /* saves an earlier version left in the game data directory come along, once */
        if (strcmp(user_dir_path, data_dir) != 0) {
            char to[1400];

            snprintf(to, sizeof(to), "%s/savegame.dir", user_dir_path);
            if (access(to, F_OK) != 0 && (d = opendir(data_dir)) != NULL) {
                while ((e = readdir(d)) != NULL)
                    if (strncasecmp(e->d_name, "savegame.", 9) == 0 || strcasecmp(e->d_name, "intrplay.cfg") == 0) {
                        char from[1400];

                        snprintf(from, sizeof(from), "%s/%s", data_dir, e->d_name);
                        snprintf(to, sizeof(to), "%s/%s", user_dir_path, e->d_name);
                        copy_file(from, to);
                    }
                closedir(d);
            }
        }
    }
    return user_dir_path;
}

static char amiga_music_path[1024];
static int amiga_music_set;

void plat_set_amiga_music_dir(const char *dir)
{
    snprintf(amiga_music_path, sizeof(amiga_music_path), "%s", dir);
    amiga_music_set = 1;
}

const char *plat_amiga_music_dir(void)
{
    struct stat st;

    if (!amiga_music_set) {
        snprintf(amiga_music_path, sizeof(amiga_music_path), "%s/amiga", user_dir());
        if (stat(amiga_music_path, &st) != 0 || !S_ISDIR(st.st_mode))
            amiga_music_path[0] = 0;
        amiga_music_set = 1;
    }
    return amiga_music_path[0] ? amiga_music_path : NULL;
}

/* Resolve a DOS path to a real one under the data directory.  Components
 * that do not exist are appended as given, so that a file can be created.
 * Returns 1 if the whole path exists. */
static int resolve(const char *path, char *out, size_t outsize)
{
    char comp[256];
    size_t len;
    int exists = 1;

    if (is_user_file(path)) {
        snprintf(out, outsize, "%s/%s", user_dir(), path);
        return access(out, F_OK) == 0;
    }
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
