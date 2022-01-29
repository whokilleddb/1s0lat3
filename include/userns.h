#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include "utils.h"
#include <unistd.h>

void write_to_file(char* path_to_file, char *line_to_write){
    FILE *f = fopen(path_to_file,"w");
    if (f == NULL){
        exit_on_error("Failed to open file for writing:(");
    }
    if (fwrite(line_to_write, 1, strlen(line_to_write), f) == 0){
        exit_on_error("Failed to write to file :(");
    }
    if (fclose(f) != 0){
        exit_on_error("Failed to close file :(");
    }
}

static void prepare_user_ns(int pid){
    char* path;
    char* line;
    
    /* Write UID to /proc/pid/uid_map
    Format:
    0 UID 1 

    This ensures that under isolated conditions, we have UID 1
    */
    if ((asprintf(&path, "/proc/%d/uid_map", pid) < 0) || (asprintf(&line, "0 %d 1\n", UID) < 0)){
        exit_on_error("Could not write to file :(");
    }
    write_to_file(path, line);

    // Disable the setgroups system call
    memset(path, 0, (int)sizeof(path));
    memset(line, 0, (int)sizeof(line));
    if ((asprintf(&path, "/proc/%d/setgroups", pid) < 0) || (asprintf(&line, "deny") < 0)){
        exit_on_error("Function asprintf() failed :(");
    }    
    write_to_file(path, line);

    /* Write UID to /proc/pid/gid_map
    Format:
    0 UID 1 

    This ensures that under isolated conditions, we have GID 1
    */
    memset(path, 0, (int)sizeof(path));
    memset(line, 0, (int)sizeof(line));

    if ((asprintf(&path, "/proc/%d/gid_map", pid) < 0) || (asprintf(&line, "0 %d 1\n", UID) < 0)){
        exit_on_error("Could not write to file :(");
    }
    write_to_file(path, line);

    free(path);
    free(line);
}