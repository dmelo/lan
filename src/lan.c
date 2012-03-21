#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "lan.h"

#define STR_SIZE 1024
#define MINARGS(X) if(argc < X) end(1)

void end(int code)
{
    if(1 == code) {
        fprintf(stderr, "Usage: lan object operation [subop] file1 [file2 [file3 ...]]\n\n");
        exit(1);
    }

}

int lan_init()
{
    char cwd[PATH_MAX];

    // TODO: check if dirs exists first. if it does, return 1.

    mkdir(".lan", 0xffff); // TODO: FIX THIS MASKS
    mkdir(".lan/labels", 0xffff);
    mkdir(".lan/notes", 0xffff);

    printf("Initialized empty lan in %s/.lan\n", getcwd(cwd, sizeof(cwd)));

    return 0;
}

int _lan_check_lanfs()
{
    // TODO: check .lan exists
    // TODO: check current dir is lanfs
    return 0;
}

void _lan_trim_linebreak(char *line)
{
    int end = strlen(line) - 1;
    if('\n' == line[end])
        line[end] = '\0';
}

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

char *_label_get_filename_by_label(char *label)
{
    char *filename = (char *) malloc(PATH_MAX * sizeof(char));
    sprintf(filename, ".lan/labels/%s", label);

    return filename;
}


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

    newfiles = (char ** ) malloc(*size * sizeof(char *));
    memset(newfiles, 0, sizeof(char *) * (*size));
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


int label_add(char *label, char **files, int size)
{
    char **old_files = NULL;
    int old_size = 0, i;
    char *filename = _label_get_filename_by_label(label);
    FILE *fd;

    old_files = (char **) malloc(1024 * 1024 * sizeof(char *));
    _label_get_files_list(label, old_files, &old_size);
    if(NULL != (fd = fopen(filename, "a"))) {
        if(0 == size) { // new files
            if(NULL != (fd = fopen(filename, "w"))) {
                for(i = 0; i < size; i++)
                    fprintf(fd, "%s\n", files[i]);
            }
        }
        else {
            _label_get_nonduplicated_lines(old_files, old_size, files, &size);
                for(i = 0; i < size; i++)
                    fprintf(fd, "%s\n", files[i]);
        }
        fclose(fd);
    }
    else {
        fprintf(stderr, "Error: could'n open file %s for writing", filename);
        exit(2);
    }


    // disalocating resources.
    free(filename);
    for(i = 0; i < old_size; i++)
        free(old_files[i]);
    free(old_files);

    return 0;
}

int label_info(char *file)
{
    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;

    MINARGS(2);
    if(strcmp("label", argv[1]) == 0) {
        MINARGS(3);
        if(strcmp(argv[2], "add") == 0) {
            MINARGS(5);
            _lan_check_files(argc - 4, &(argv[4]));
            ret = label_add(argv[3], &(argv[4]), argc - 4);
        }
        else if(strcmp(argv[2], "info"))
            ret = label_info(argv[3]);
        else
            end(1);
    }
    else if(strcmp("note", argv[1]) == 0) {

    }
    else if(strcmp("manage", argv[1]) == 0) {

    }
    else if(strcmp("init", argv[1]) == 0) {
        ret = lan_init();
    }

    return ret;
}
