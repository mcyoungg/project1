#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include "myshell.h"

#define TRUE 1

void display_prompt(int argc, char *argv[]){
    bool is_supressed = false;
    char prompt_text[] = "my_shell$";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            is_supressed = true;
        }
    }
    if(!is_supressed){
        printf("%s ", prompt_text);
    }
}

char* read_command(){
    char buffer[513];  
    char* input = NULL;

    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
    else{
        return input;
    }

    input = (char*)malloc(strlen(buffer) + 1);
    if (input != NULL) {
        strcpy(input, buffer);
    }
    else{
        perror("ERROR: memory allocation failed ");
    }
    return input;
}

char** parse_command(char* input, char* delimeter, int* command_length){
    char** commands = NULL;
    char** temp_commands;
    int command_count = 0;

    char* token = strtok(input, delimeter);
    char* clean_input;
    while (token != NULL) {
        clean_input = remove_whitespace(token);

        temp_commands = (char**)realloc(commands, (command_count + 1) * sizeof(char*));
        if(!temp_commands){
            printf("ERROR: memory reallocation failed");
        }
        commands = temp_commands;
        commands[command_count] = clean_input;
        command_count++;

        token = strtok(NULL, delimeter);
    }
    temp_commands = (char**)realloc(commands, (command_count + 1) * sizeof(char*));
    if(!temp_commands){
        printf("ERROR: memory reallocation failed");
    }
    commands = temp_commands;
    commands[command_count] = NULL;

    *(command_length) = command_count;

    return commands;
}

char* remove_whitespace(char* input){
    if (input == NULL) return input;

    char* clean_input = (char*)malloc(strlen(input) + 1);
    if (clean_input == NULL) {
        printf("ERROR: memory allocation failed");
    }
    strcpy(clean_input, input);

    int length = strlen(clean_input);
    int i = 0; int j = 0;

    while (i < length && isspace((unsigned char)clean_input[i])) i++;
    while (i < length) {
        clean_input[j] = clean_input[i];
        i++; j++;
    }
    clean_input[j] = '\0';

    int new_length = j; 

    while (new_length > 0 && isspace((unsigned char)clean_input[new_length-1])) {
        new_length--;
    }
    clean_input[new_length] = '\0';

    return clean_input;
}

char** setup_redirect(char** args, int* fd_in, int* fd_out, int* arg_length_ptr){
   
    char* concat_str = NULL;
    char* in_file = NULL;
    char* out_file = NULL;
    int concat_str_len = 0;
    int found_redirection = 0;
    int i = 0;  int j = 0;
    
    char** clean_args = (char**)malloc((*arg_length_ptr + 1) * sizeof(char*)); // +1 for NULL terminator
    if (clean_args == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }

    for (i = 0; i < *arg_length_ptr; i++) {
        char* arg = args[i];
        int arg_len = strlen(arg);

        for (j = 0; j < arg_len; j++) {
            if ((arg[j] == '<' || arg[j] == '>') && !found_redirection) {
                found_redirection = 1;
            }  

            if (found_redirection) {
                concat_str_len++;
                concat_str = (char*)realloc(concat_str, concat_str_len + 1);
                if (concat_str == NULL) {
                    perror("Memory allocation failed");
                    exit(1);
                }
                concat_str[concat_str_len - 1] = arg[j];
                concat_str[concat_str_len] = '\0';
            }
        }
    }
    
    for (i = 0; args[i] != NULL; i++) {
        char* arg = args[i];
        int arg_len = strlen(arg);

        for (j = 0; j < arg_len; j++) {
            if (arg[j] == '<' || arg[j] == '>') {
                found_redirection = 1;
                break;
            }
        }
            
        if (found_redirection) {
            j--; 
        }

        clean_args[i] = (char*)malloc((j + 2) * sizeof(char));
        if (clean_args[i] == NULL) {
            perror("Memory allocation failed");
            exit(1);
        }

        strncpy(clean_args[i], arg, j + 1);
        clean_args[i][j + 1] = '\0';
        
        if (found_redirection) {
            break;
        }
    }
    
    clean_args[*arg_length_ptr] = NULL;

    int index = 0;
    int length = strlen(concat_str);

    while (index < length && concat_str[index] != '<') {
        index++;
    }

    if (index < length) {
        index++;
        int start = index;
        while (index < length && concat_str[index] != '>') {
            index++;
        }
        if (index > start) {
            int rd_length = index - start;
            in_file = (char*)malloc(rd_length + 1);
            if (in_file == NULL) {
                perror("Memory allocation failed");
                exit(1);
            }
            strncpy(in_file, &concat_str[start], rd_length);
            in_file[rd_length] = '\0';
        }
    }

    index = 0;
    while (index < length && concat_str[index] != '>') {
        index++;
    }

    if (index < length) {
        index++; 

        int start = index;
        while (index < length) {
            index++;
        }

        int rd_length = index - start;

        out_file = (char*)malloc(rd_length + 1); 
        if (out_file == NULL) {
            perror("Memory allocation failed");
            exit(1);
        }

        strncpy(out_file, &concat_str[start], rd_length);
        out_file[rd_length] = '\0';
    }

    if(in_file != NULL){
        *fd_in = open(in_file, O_RDONLY);
         if (*fd_in == -1) {
            perror("ERRORll");
            exit(1);
        }
        dup2(*fd_in, STDIN_FILENO);
        close(*fd_in);
    }
    
    if(out_file != NULL){
        *fd_out = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (*fd_out == -1) {
            perror("ERRORpp");
            exit(1);
        }
        dup2(*fd_out, STDOUT_FILENO);
        close(*fd_out);
    }

    return clean_args;

}

void sigchld_handler(int signum) {
    int status;
    pid_t child_pid;

    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0);

}

int main(int argc, char *argv[]) {
    char** commands;
    char* input;
    int status;
    char** execute_command_child;
    char** execute_rd_child;
    int command_length;
    int* command_length_ptr = &command_length;
    int arg_length;
    int* arg_length_ptr = &arg_length;
    char* input_redirect = "<";
    char* output_redirect = ">";
    int cmdrd_length;
    int* cmdrd_length_ptr = &cmdrd_length;
    int fd_in = -1;
    int fd_out = -1;
    int* input_rd_fd = &fd_in;
    int* output_rd_fd = &fd_out;
    
    signal(SIGCHLD, sigchld_handler);
    while(TRUE){
        
        display_prompt(argc, argv);
        fflush(stdout);

        input = read_command();
        if(input == NULL){
            break;
	    }

        int run_in_background = 0;
        if (input[strlen(input) - 1] == '&') {
            run_in_background = 1;
            input[strlen(input) - 1] = '\0';
        }
        
        //input = "echo abc > lol.txt";
        commands = parse_command(input, "|", command_length_ptr);

        int pipes[command_length - 1][2]; 
        pid_t child_pids[command_length]; 
        for (int i = 0; i < command_length - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("ERROR");
            }
        }

        int is_input_rd = -1;
        int is_output_rd = -1;

        for(int i = 0; i < command_length; i++) {

            child_pids[i] = fork();

            if(child_pids[i] == 0){

                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                    close(pipes[i - 1][0]);
                    close(pipes[i - 1][1]);
                }

                if (i < (command_length - 1)) {
               
                    dup2(pipes[i][1], STDOUT_FILENO); 
                    close(pipes[i][1]);
                    close(pipes[i][0]);
                }

                execute_command_child = parse_command(commands[i], " ", arg_length_ptr);

                // for(int j = 0; j < arg_length; j++){
                //     if(i == 0 && (!strcmp(execute_command_child[j], input_redirect))){
                //         is_input_rd = j;
                //     }
                //     if(i == command_length - 1 && (!strcmp(execute_command_child[j], output_redirect))){
                //         is_output_rd = j;
                //     }
                // }

                // if(is_input_rd >= 0 || is_output_rd >= 0){
                //     printf("exec, in_rd %d, out_rd_rd %d\n", is_input_rd, is_output_rd);
                    execute_rd_child = setup_redirect(execute_command_child, input_rd_fd, output_rd_fd, arg_length_ptr);
                    if(execvp(execute_rd_child[0], execute_rd_child) == -1) {
                        perror("ERROR");
                    }
                //}
                // else{
                //     if(execvp(execute_command_child[0], execute_command_child) == -1) {
                //     perror("ERROR");
                //     }
                // }
            }
            if (!run_in_background) {

                if (i > 0) {
                    close(pipes[i - 1][0]);
                }
                if (i < (command_length - 1)) {
                    close(pipes[i][1]);
                }

                // if(fd_in != -1){
                //     close(fd_in);
                // }

                // if(fd_out != -1){
                //     close(fd_out);
                // }

                waitpid(child_pids[i], &status, 0);
            }
        }
    }
    return 0;
}