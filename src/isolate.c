// Necessary headers
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>


// User-defined headers
#include "utils.h"
#include "mountns.h"
#include "userns.h"
#include "networkns.h"


// Flags used in name space creation
#define NSFLAGS (SIGCHLD | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET)

// Stack size to store child process stack
#define STACKSIZE 1024*1024


// Struct used for storing commands and parent-child processs communication
struct params {
    int fd[2];          // Array to serve as "Pipe"
    char **argv;        // Command to execute
};


// Extract the command (along with the given arguments) to be run in isolation
int parse_args(int argc, char* argv[], struct params *params){
    if (argc <=1){return -1;}
    char** param_args = ++argv;
    params->argv = param_args;
    return argc-1;
}


// Print command to be run in isolation
void print_isolated_cmd(int argc, char **argv){
    int i;
    for(i=0; i<argc; i++){
        fprintf(stdout,"%s ", argv[i]);
    }
    fprintf(stdout,"\n");
}

// Child process to be called to run a command 
int cmd_exec(void *arg){
    // Send a SIGKILL if the isolated process dies
    if (prctl(PR_SET_PDEATHSIG, SIGKILL)<0){
        fprintf(stderr,"[" RED("!") "] Cannot Set" RED("prctl()")"\n");
        return -1;
    }

    // Casting arg as param
    struct params *params = (struct params*) arg;
    
    // await 'setup done' signal from main
    char buf[2];
    if(read(params->fd[0],buf,2)!= 2){
        fprintf(stderr, "[" RED("!") "] Failed to read from pipe while awaiting "RED("'setup done'")" from main");
        return -1;
    }

    // Prepare MOUNT namespace
    if (prepare_mountns() != 0){
        fprintf(stderr,"[" RED("!") "] Failed to create "RED("Mount") "namespace\n");
        return -1;
    }
    fprintf(stdout,"[" GREEN("i") "] Successfully created " GREEN("Mount") " namespace\n");

    // Close reading end of the pipe once done
    if(close(params->fd[0])){
        fprintf(stderr, "[" RED("!") "] Failed to close pipe\n");
        return -1;
    }

    // Drop superuser privileges
    if ((setuid(0)==-1) || setgid(0) == -1){
        fprintf(stderr, "[" RED("!") "] Could not set privileges\n");
        return -1;
    }

    char **argv  = params->argv;
    char *cmd = argv[0];

    if (execvp(cmd,argv)==-1){
        fprintf(stderr,"[" RED("!")"] Cannot execute command in Isolation :(\n");
        return -1;
    }

    exit(EXIT_SUCCESS); 
    return 0;
} 

// Main Functions
int main(int argc, char* argv[]){
    fprintf(stdout,"[" GREEN("+") "] " CYAN("1s0lat3") " by " MAGENTA("@whokilleddb")"\n");


    // Check if program is being run as root
    if (geteuid()!=0){
        exit_on_error("["RED("-")"] Program must be run as "RED("ROOT")"\n");
    }


    // Create a variable to store commands to be run in isolation and pipes
    struct params cli_params;
    if(memset(&cli_params, 0, sizeof(struct params)) == NULL){
        exit_on_error("[" RED("-") "] The function " RED("memset()") " failed\n");
    }    


    // Check and parse command line arguments
    int result = parse_args(argc, argv, &cli_params);
    if (result == -1){
        fprintf(stderr,"[" RED("-") "] Nothing To Do :(\n");
        fprintf(stdout,"[" CYAN(">") "] Usage: %s "BLUE("COMMAND")"\n",argv[0]);
        exit(EXIT_FAILURE);
    }


    // Print command line arguments
    fprintf(stdout,"[" MAGENTA(">") "] Command to be run in " CYAN("1s0lati0n")": ");
    print_isolated_cmd(result, cli_params.argv );


    // Create pipes for communication with child process
    if(pipe(cli_params.fd) < 0){
        exit_on_error("[" RED("-") "] Failed to create "RED("pipe()")"\n");
    }


    // Setup memory for the child process
    char *stack;
    char *stackhead;
    stack = (char *)malloc(STACKSIZE);
    if (!stack){
        exit_on_error("[" RED("-") "] Failed to allocate stack memory for child process\n");
    }

    if(memset(stack,0,STACKSIZE) == NULL){
        exit_on_error("[" RED("!") "] The function " RED("memset()") " failed\n");
    }
    stackhead = stack + STACKSIZE ;
    

    // When the command exits, it leaves a return status code
    // Start with cloning the UTS Namespace
    int cmd_pid = clone(cmd_exec, stackhead, NSFLAGS, &cli_params);
    

    // Check if clone was successful
    if (cmd_pid < 0){
       free(stack);
       exit_on_error("[" RED("!") "] The function " RED("clone()") " failed\n");
    }
    fprintf(stdout,"[" GREEN("i") "] Successfully created " GREEN("UTS") " namespace\n");


    // Prepare USER Namespace 
    if (prepare_userns(cmd_pid) != 0){
        exit_on_error("[" RED("!") "] Failed to create " RED("USER") " namespace\n");
    }
    fprintf(stdout,"[" GREEN("i") "] Successfully created " GREEN("USER") " namespace\n");
    
    // Prepare Network Namespace 
    if (prepare_networkns(cmd_pid) != 0){
        exit_on_error("[" RED("!") "] Failed to create " RED("NETWORK") " namespace\n");
    }
    fprintf(stdout,"[" GREEN("i") "] Successfully created " GREEN("NETWORK") " namespace\n");

    // Send 'setup done' signal to Child process
    if (write(cli_params.fd[1],"OK",2)!=2){
        free(stack);
        exit_on_error("Cannot Send 'setup done' to child process");
    }


    // Close pipe after writing
    if (close(cli_params.fd[1])<0){
        free(stack);
        exit_on_error("Cannot close pipe :(");
    } 


    // Wait for child process to complete execution
    if(waitpid(cmd_pid,NULL,0) == -1){
        char buff[50];
        memset(buff,0,sizeof(buff));
        snprintf(buff, 50, "Failed to wait for PID: %d\n", cmd_pid);
        free(stack);
        exit_on_error(buff);
    }


    // Free Memory like a good boi
    free(stack);   
    fprintf(stdout,"[" GREEN("+") "] Bye :D\n");
    return 0;
}