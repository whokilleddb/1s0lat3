#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sched.h>
#include "utils.h"

// Struct used for storing commands and parent-child processs communication
struct params {
    int fd[2];          // Array to serve as "Pipe"
    char **argv;        // Command to execute
};

// Print command to be run in isolation
static inline void print_isolated_cmd(int argc, char **argv){
    int i;
    for(i=0;i<argc;i++){fprintf(stdout,"%s ", argv[i]);}
    fprintf(stdout,"\n");
}

// Child process to be called to run a command 
int cmd_exec(void *arg){

    // Send a SIGKILL if the isolated process dies
    if (prctl(PR_SET_PDEATHSIG, SIGKILL)<0){
        exit_on_error("Cannot Set" RED("prctl()")"\n");
    }

    // Casting arg as param
    struct params *params = (struct params*) arg;
    
    // await 'setup done' signal from main
    char buf[2];
    if(read(params->fd[0],buf,2)!= 2){
        exit_on_error("Failed to read from pipe while awaiting 'setup done' from main");
    }

    prepare_mount_ns(ROOTFS);

    // Close reading end of the pipe once done
    if(close(params->fd[0])){
        exit_on_error("Failed to read close pipe :(");
    }

    // Drop superuser privileges
    if ((setuid(0)==-1) || setgid(0) == -1){
        exit_on_error("Error setting privileges :(");
    }

    char **argv  = params->argv;
    char *cmd = argv[0];

    if (execvp(cmd,argv)==-1){
        exit_on_error("Cannot execute command In Isolation:(");
        return -1;
    }

    exit(EXIT_SUCCESS); 
    return 0;
} 