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
        printf("Usage: lan object operation [subop] file1 [file2 [file3 ...]]");
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

int _label_get_files_list(char *label, char **files, int *size)
{
    char file_name[PATH_MAX];
    FILE *fd;
    int i = 0;

    sprintf(file_name, ".lan/labels/%s", label);
    fd = fopen(file_name, "r");
    if(NULL != fd) {
        files = (char **) malloc(1024 * 1024 * sizeof(char *));
        while(!feof(fd)) {
            files[i] = (char *) malloc(PATH_MAX * sizeof(char));
            fgets(files[i], PATH_MAX, fd);
            i++;
        }
        *size = i;
        fclose(fd);
    }
    else {
        files = NULL;
        *size = 0;
    }

    return 0;
}

int label_add(char *label, char **files, int size)
{
    char **old_files = NULL;
    int old_size = 0;

    _label_get_files_list(label, old_files, &old_size);
    if(0 == size) { // TODO: open file as "w" and write everything from files
        
    }
    else {
    }

    return 0;
}

int label_info(char *file)
{
    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;
    char **arg_list = NULL;
    int size = 0;
    MINARGS(2);
    if(strcmp("label", argv[1]) == 0) {
        MINARGS(3);
        if(strcmp(argv[2], "add") == 0) {
            MINARGS(5);
            ret = label_add(argv[3], arg_list, size);
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
