#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void run_batch_mode(const char *filename);
void run_interactive_mode(void);
void execute_command(char *cmd); 

void run_batch_mode(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1) {
        execute_command(line);
    }
    free(line);
    fclose(file);
}

void run_interactive_mode() {
    char *line = NULL;
    size_t len = 0;
    printf("Welcome to my shell!\n");
    
    while (1) {
        printf("mysh> ");
        if (getline(&line, &len, stdin) == -1) {
            if (feof(stdin)) { // Check for EOF
                break;
            } else {
                perror("readline");
                continue;
            }
        }
        execute_command(line);
    }
    free(line);
}


void execute_command(char *cmd) {
    // Remove newline character
    size_t len = strlen(cmd);
    if (len > 0 && cmd[len-1] == '\n') {
        cmd[len-1] = '\0';
    }

    // Tokenize the command to get the arguments
    char *args[128]; // This array will hold the command and its arguments
    int argc = 0;
    char *token = strtok(cmd, " ");
    while (token != NULL) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    args[argc] = NULL; // Terminate the array of arguments with a NULL pointer

    // Special case for "exit" command
    if (strcmp(args[0], "exit") == 0) {
        printf("mysh: exiting\n");
        exit(EXIT_SUCCESS);
    }

    // Handle the 'cd' command separately
    if (strcmp(args[0], "cd") == 0) {
        char *path = argc > 1 ? args[1] : getenv("HOME"); // If no directory specified, use HOME

        // Implement 'cd -' to go back to the previous directory
        static char *previous_dir = NULL;
        if (args[1] && strcmp(args[1], "-") == 0) {
            if (previous_dir) {
                path = previous_dir;
            } else {
                fprintf(stderr, "cd: OLDPWD not set\n");
                return;
            }
        } else {
            // Save the current directory as previous before changing it
            free(previous_dir);
            previous_dir = getcwd(NULL, 0);
        }

        if (chdir(path) != 0) {
            perror("cd");
        }
        return; // Return early since we've handled 'cd' command
    }

    // For all other commands, use fork and exec
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        // Child process
        // Suppress error messages from execvp
        fclose(stderr);
        execvp(args[0], args);
        // exec only returns if there is an error
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0){ // further error handling
            int status;
            pid_t wpid = waitpid(pid, &status, 0);

            if (wpid == -1) {
                perror("waitpid"); // Error in waitpid
            } else {
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status != 0) {
                        printf("Command exited with status %d\n", exit_status);
                    }
                } else if (WIFSIGNALED(status)) {
                    int term_sig = WTERMSIG(status);
                    printf("Command terminated by signal %d (%s)\n", term_sig, strsignal(term_sig));
                } else if (WIFSTOPPED(status)) {
                    int stop_sig = WSTOPSIG(status);
                    printf("Command stopped by signal %d (%s)\n", stop_sig, strsignal(stop_sig));
                }
            }
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc == 2) {
        // Batch mode
        run_batch_mode(argv[1]);
    } else if (argc == 1) {
        // Interactive mode
        run_interactive_mode();
    } else {
        fprintf(stderr, "Usage: %s [scriptfile]\n", argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
 * RUNNING THE BATCH MODE TEST CASES :
 * 
 * compile the program: gcc -Wall -o mysh mysh.c
 * 
 * To run the cases :
 * 
 * (1) Basic test of commands
 * ./mysh test_basic.sh
 * 
 * (2) Error handling
 * ./mysh test_error_handling.sh
 * 
 * (3) Multiple and Empty Commands
 * ./mysh test_multiple_empty.sh
 * 
 * (4) Exit Command
 * ./mysh test_exit.sh
 * 
 * (5) Environment Variables and Built-in Commands
 * ./mysh test_env_builtin.sh
 * 
*/

// Parent process waits for the child to complete