#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "lan.h"

#define STR_SIZE 1024
#define MINARGS(X) if(argc < X) end(1)

// TODO: make this with "..." and pre-define the most common errors.
void end(int code)
{
    if(1 == code) {
        fprintf(stderr, "Usage: lan object operation [subop] file1 [file2 [file3 ...]]\n\n");
        fprintf(stderr, "\toperation: [label|note]\n");
        fprintf(stderr, "\tsubop: [add|info|rm|search]\n\n");
        exit(1);
    }
}

/**
 * TODO: put this one to be called during mount.
 */
int lan_init()
{
    char cwd[PATH_MAX];
    int i;

    char** dirs;
    dirs = (char **) malloc(3 * sizeof(char *));
    dirs[0] = ".lan";
    dirs[1] = ".lan/labels";
    dirs[2] = ".lan/notes";

    for(i = 0; i < 3; i++) {
        struct stat st;
        if(stat(dirs[i], &st) == 0) {
            fprintf(stderr, "Directory %s already exists\n", dirs[i]);
            exit(1);
        }
        mkdir(dirs[i], 0777);
    }

    printf("Initialized empty lan in %s/.lan\n", getcwd(cwd, sizeof(cwd)));

    return 0;
}

/**
 * Check if filesystem still consistent.
 */
int _lan_check_lanfs()
{
    // TODO: check .lan exists
    // TODO: check current dir is lanfs
    return 0;
}

/**
 * In case a line ends with newline, remove the ending newline.
 */
void _lan_trim_linebreak(char *line)
{
    int end = strlen(line) - 1;
    if('\n' == line[end])
        line[end] = '\0';
}

/**
 * Check if all the files exists and are really files.
 */
void _lan_check_files(int argc, char **argv)
{
    int i;
    struct stat buf;

    for(i = 0; i < argc; i++) {
        if(0 == stat(argv[i], &buf)) {
            if(S_ISDIR(buf.st_mode)) {
                fprintf(stderr, "Error: %s is a directory.\n", argv[i]);
                exit(4);
            }
            else {
                // everything is ok.
            }
        }
        else {
            fprintf(stderr, "Error: file %s does not exists.\n", argv[i]);
            exit(3);
        }
    }
}

/**
 * Given the label name, returns the full name of the file that represents this
 * label.
 */
char *_label_get_filename_by_label(char *label)
{
    char *filename = (char *) malloc(PATH_MAX * sizeof(char));
    sprintf(filename, ".lan/labels/%s", label);

    return filename;
}

/**
 * Given the label name, list all files with this label.
 *
 * @param *label Label name
 * @param **files output var for the list of files within
 * @param *size output var with the number of files in **files.
 */
int _label_get_files_list(char *label, char **files, int *size)
{
    char *filename = _label_get_filename_by_label(label);
    FILE *fd;
    int i = 0;

    if(NULL != (fd = fopen(filename, "r"))) {
        while(!feof(fd)) {
            files[i] = (char *) malloc(PATH_MAX * sizeof(char));
            fgets(files[i], PATH_MAX, fd);
            _lan_trim_linebreak(files[i]);
            if(strlen(files[i]) > 0)
                i++;
        }
        *size = i;
        fclose(fd);
    }
    else {
        *size = 0;
    }

    free(filename);

    return 0;
}

int _label_get_nonduplicated_lines(char **oldfiles, int oldsize, char **files, int *size)
{
    char **newfiles;
    int i, j, newsize, isRepeated;

    newfiles = (char ** ) malloc((*size + oldsize) * sizeof(char *));
    memset(newfiles, 0, sizeof(char *) * (*size + oldsize));
    newsize = 0;

    // Put all the non-repeated lines on newfiles.
    for(i = 0; i < *size; i++) {
        isRepeated = 0;
        for(j = 0; j < oldsize; j++) {
            if(strcmp(oldfiles[j], files[i]) == 0) {
                isRepeated = 1;
                break;
            }
        }
        if(0 == isRepeated) {
            newfiles[newsize] = (char *) files[i];
            newsize++;
        }
    }

    // replace files by newfiles.
    for(i = 0; i < *size; i++)
        files[i] = newfiles[i];
    *size = newsize;

    return 0;
}

int label_list()
{
    DIR *dp;
    struct dirent *ep;

    dp = opendir(".lan/labels");
    if(NULL != dp) {
        while(NULL != (ep = readdir(dp)))
            if(ep->d_type & 0x8)
                printf("%s\n", ep->d_name);
        closedir(dp);
    }

    return 0;
}

int label_add(char *label, char **files, int size)
{
    char **old_files = NULL;
    int old_size = 0, i;
    char *filename = _label_get_filename_by_label(label);
    FILE *fd;

    old_files = (char **) malloc(1024 * 1024 * sizeof(char *));
    _label_get_files_list(label, old_files, &old_size);
    if(0 == size) { // new files
        if(NULL != (fd = fopen(filename, "w"))) {
            for(i = 0; i < size; i++) {
                fprintf(fd, "%s\n", files[i]);
            }
            fclose(fd);
        }
    }
    else {
        _label_get_nonduplicated_lines(old_files, old_size, files, &size);
        if (NULL != (fd = fopen(filename, "a"))) {
            for(i = 0; i < size; i++) {
                fprintf(fd, "%s\n", files[i]);
            }
            fclose(fd);
        }
    }
    /*
    else {
        fprintf(stderr, "Error: could'n open file %s for writing", filename);
        exit(2);
    }
    */


    // disalocating resources.
    free(filename);
    for(i = 0; i < old_size; i++)
        free(old_files[i]);
    free(old_files);

    return 0;
}

int label_info(char *file)
{
    DIR *dp;
    struct dirent *ep;
    FILE *fd;
    char filename[PATH_MAX];
    char *fullpath;

    dp = opendir(".lan/labels");
    if(NULL != dp) {
        while(NULL != (ep = readdir(dp))) {
            if(ep->d_type & 0x8) {
                fullpath = _label_get_filename_by_label(ep->d_name);
                if(NULL != (fd = fopen(fullpath, "r"))) {
                    while(!feof(fd)) {
                        fgets(filename, PATH_MAX, fd);
                        _lan_trim_linebreak(filename);
                        if(strcmp(filename, file) == 0) {
                            printf("%s\n", ep->d_name);
                            break;
                        }
                    }
                    fclose(fd);
                }
                free(fullpath);
            }
        }
        closedir(dp);
    }

    return 0;
}

int label_rm(char *label, char **files, int size)
{
    FILE *fd; 
    char *fullpath = _label_get_filename_by_label(label);
    char **newfiles = malloc(size * sizeof(char *));
    char *file = malloc(PATH_MAX * sizeof(char));
    int newsize = 0, i, unique;

    if(NULL != (fd = fopen(fullpath, "r"))) {
        // Reads all the files inside the label
        while(fgets(file, PATH_MAX, fd)) {
            _lan_trim_linebreak(file);
            unique = 1;
            for(i = 0; i < size; i++) {
                if(strcmp(file, files[i]) == 0) {
                    unique = 0;
                    break;
                }
            }
            if(unique) {
                newfiles[newsize] = file;
                newsize++;
            }
        }

        fclose(fd);
        // Writes only the files not mean for deletion.
        if(NULL != fopen(fullpath, "w")) {
            for(i = 0; i < newsize; i++)
                fprintf(fd, "%s\n", newfiles[i]);
            fclose(fd);
        }
        else {
            fprintf(stderr, "Error: could not open file %s for writing\n", fullpath);
            exit(2);
        }
    }
    else {
        fprintf(stderr, "Error: could not open file %s for reading\n", fullpath);
        exit(1);
    }

    free(fullpath);

    return 0;
}

int label_search(char *label)
{
    char *fullpath = _label_get_filename_by_label(label);
    char *file = malloc(PATH_MAX * sizeof(char));
    FILE *fd;

    if(NULL != (fd = fopen(fullpath, "r"))) {
        while(fgets(file, PATH_MAX, fd)) {
            _lan_trim_linebreak(file);
            puts(file);
        }
        fclose(fd);
    }
    else {
        fprintf(stderr, "Error: could not open file %s for reading\n", fullpath);
        exit(1);
    }

    free(fullpath);

    return 0;
}

/**
 * Handles the fact that a file have changed it's name on the filesystem.
 */
int manage_rename(char *oldfile, char *newfile)
{
    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;

    MINARGS(2);
    if(strcmp("label", argv[1]) == 0) {
        if(2 == argc) {
            label_list();
        }
        else if(strcmp(argv[2], "add") == 0) {
            MINARGS(5);
            _lan_check_files(argc - 4, &(argv[4]));
            ret = label_add(argv[3], &(argv[4]), argc - 4);
        }
        else if(strcmp(argv[2], "info") == 0)
            ret = label_info(argv[3]);
        else if(strcmp(argv[2], "rm") == 0) {
            MINARGS(5);
            _lan_check_files(argc - 4, &(argv[4]));
            ret = label_rm(argv[3], &(argv[4]), argc - 4);
        }
        else if(strcmp(argv[2], "search") == 0) {
            if(4 != argc)
                end(1);
            ret = label_search(argv[3]);
        }
        else
            end(1);
    }
    else if(strcmp("note", argv[1]) == 0) {

    }
    else if(strcmp("manage", argv[1]) == 0) {
        if(strcmp("rename", argv[2]) == 0) {
            MINARGS(5);
            ret = manage_rename(argv[3], argv[4]);
        }
        else
            end(1);
    }
    else if(strcmp("init", argv[1]) == 0) {
        ret = lan_init();
    }

    return ret;
}
