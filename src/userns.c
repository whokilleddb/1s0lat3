#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include "userns.h"

#define UID 1000

// Write to a file
int write_to_file(char* path_to_file, char *line_to_write){
    FILE *f = fopen(path_to_file,"w");
    if (f == NULL){
        fprintf(stderr,"[" RED("-") "] Could not open %s for writing\n", path_to_file);
        return -1;
    }
    if (fwrite(line_to_write, 1, strlen(line_to_write), f) == 0){
        fprintf(stderr,"[" RED("-") "] Could not write to %s\n", path_to_file);
        return -1;
    }
    if (fclose(f) != 0){
        fprintf(stderr,"[" RED("-") "] Could not close %s\n", path_to_file);
        return -1;
    }
    return 0;
}


// Prepare User Namespace
int prepare_userns(int pid){
    char* path;
    char* line;
    
    /* Write UID to /proc/pid/uid_map
    Format:
    0 UID 1 

    This ensures that under isolated conditions, we have UID 1
    */
    if ((asprintf(&path, "/proc/%d/uid_map", pid) < 0) || (asprintf(&line, "0 %d 1\n", UID) < 0)){
        fprintf(stderr,"[" RED("-") "] out of memory\n");
        return -1;
    }

    // Write tp uid_map
    if (write_to_file(path, line) != 0){
        fprintf(stderr,"[" RED("-") "] Could not prepare " RED("uid_map") "\n");
        return -1;
    }

    // Zero out memory 
    if(memset(path, 0, (int)sizeof(path)) == NULL || memset(line, 0, (int)sizeof(line)) == NULL){
        fprintf(stderr,"[" RED("-") "] The function " RED("memset()") " failed\n");
        return -1;
    }
    
    // Disable the setgroups system call
    if ((asprintf(&path, "/proc/%d/setgroups", pid) < 0) || (asprintf(&line, "deny") < 0)){
        fprintf(stderr,"[" RED("-") "] out of memory\n");
        return -1;
    }    


    // Write tp uid_map
    if (write_to_file(path, line) != 0){
        fprintf(stderr,"[" RED("-") "] Could not disable " RED("setgroups") " syscall\n");
        return -1;
    }


    // Zero out memory 
    if(memset(path, 0, (int)sizeof(path)) == NULL || memset(line, 0, (int)sizeof(line)) == NULL){
        fprintf(stderr,"[" RED("-") "] The function " RED("memset()") " failed\n");
        return -1;
    }
    
    /* Write UID to /proc/pid/gid_map
    Format:
    0 UID 1 

    This ensures that under isolated conditions, we have GID 1
    */
    if ((asprintf(&path, "/proc/%d/gid_map", pid) < 0) || (asprintf(&line, "0 %d 1\n", UID) < 0)){
        fprintf(stderr,"[" RED("-") "] out of memory\n");
        return -1;
    }
        
    // Write tp uid_map
    if (write_to_file(path, line) != 0){
        fprintf(stderr,"[" RED("-") "] Could not prepare " RED("gid_map") "\n");
        return -1;
    }

    free(path);
    free(line);
    return 0;
}


