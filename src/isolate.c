// Necessary headers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>

// User-defined headers
#include "isolate.h"
#include "utils.h"

// Extract the command (along with the given arguments) to be run in isolation
int parse_args(int argc, char* argv[], struct params *params){
    if (argc <=1){return -1;}
    char** param_args = ++argv;
    params->argv = param_args;
    return argc-1;
}

// Check if the program is being run as root
void check_root(){
    if (geteuid()!=0){
        fprintf(stderr,"[" RED("-") "] The Program Must Be Run As " RED("ROOT") "\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]){
    fprintf(stdout,"[" GREEN("+") "] " CYAN("1s0lat3") " by " MAGENTA("@whokilleddb")"\n");
    
    // Check if program is being run as root
    check_root();

    // Create a variable to store commands to be run in isolation and pipes
    struct params cli_params;
    memset(&cli_params, 0, sizeof(struct params));
    
    int result = parse_args(argc, argv, &cli_params);
    if (result == -1){
        fprintf(stderr,"[" RED("-") "] Nothing To Do :(\n");
        fprintf(stdout,"[" CYAN(">") "] Usage: %s "BLUE("COMMAND")"\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    
    fprintf(stdout,"[" MAGENTA(">") "] Command to be run in " CYAN("1s0lati0n")": ");
    print_isolated_cmd(result, cli_params.argv );

    // Create  pipes
    if(pipe(cli_params.fd) < 0){
        exit_on_error(":(");
    }

    // Setup memory for the child process
    char *stack;
    char **stackhead;
    
    stack = (char *)malloc(STACKSIZE);
    if (!stack){
        exit_on_error("Failed to allocate memory for the child thread :(");
    }
    memset(stack,0,sizeof(stack));
    stackhead = stack + STACKSIZE - 1;

    // When the command exits, it leaves a return status code
    // Start with cloning the UTS Namespace
    int cmd_pid = clone(cmd_exec, stackhead, SIGCHLD |CLONE_NEWUTS, &cli_params);
    
    // Check if clone was successful
    if (cmd_pid < 0){
       free(stack);
       exit_on_error("Failed To Clone :(");
    }

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

    free(stack);   
    fprintf(stdout,"[" GREEN("+") "] Bye :D\n");
    return 0;
}