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
        printf("ERROR: memory allocation failed ");
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

char * remove_rd_delimeter(char* input, char* del_in, char* del_out){
    char* token = strtok(input, del_in);
    char* clean_tok = strtok(token, del_out);
    return clean_tok;
}

void setup_redirect(char** args, int is_input_rd, int is_output_id, int* arg_length_ptr){
    int skip_flag = 0; 
    int arg_output_len = 0;
    char** redirect_execute_commands = NULL;
    char** redirect_execute_commands_cln = NULL;
    int fd_in;
    int fd_out;

    if(is_input_rd >= 0){
        fd_in = open(args[is_input_rd+1], O_RDONLY);
         if (fd_in < 0) {
            perror("ERROR");
            exit(1);
        }
        dup2(fd_in, STDIN_FILENO);
    }

    if(is_output_id >= 0){
        fd_out = open(args[is_output_id+1], O_WRONLY | O_CREAT);
        if (fd_out < 0) {
            perror("ERROR");
            exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
    }

    for (int i = 0; i < (*arg_length_ptr); i++) {
        if (skip_flag) {
            skip_flag = 0;
            continue; 
        }
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
            skip_flag = 1; 
            continue;
        }
        redirect_execute_commands[arg_output_len] = args[i];
        (arg_output_len)++;
    }
    
    // for(int i = 0; i < arg_output_len; i++){
    //     redirect_execute_commands[i] = remove_rd_delimeter(redirect_execute_commands[i], "<", ">");
    // }

    if(execvp(redirect_execute_commands[0], redirect_execute_commands) == -1) {
        perror("ERROR");
    }

    if(is_input_rd >= 0) close(fd_in);
    if(is_output_id >= 0) close(fd_out);

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
    int is_input_rd = -1;
    int is_output_rd = -1;
    
    while(TRUE){
    
        display_prompt(argc, argv);
        fflush(stdout);

        input = read_command();
        if(input == NULL){
            break;
	    }

        commands = parse_command(input, "|", command_length_ptr);

        int pipes[command_length - 1][2]; 
        pid_t child_pids[command_length]; 
        for (int i = 0; i < command_length - 1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("ERROR");
            }
        }

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

                for(int j = 0; j < arg_length; j++){
                    if(i == 0 && (!strcmp(execute_command_child[j], input_redirect))){
                        is_input_rd = j;
                    }
                    if(i == command_length - 1 && (!strcmp(execute_command_child[j], output_redirect))){
                        is_output_rd = j;
                    }
                }

                if(is_input_rd >= 0 || is_output_rd >= 0){
                    setup_redirect(execute_command_child, is_input_rd, is_output_rd, arg_length_ptr);
                }
                else{
                    if(execvp(execute_command_child[0], execute_command_child) == -1) {
                        perror("ERROR");
                    }
                }
            }

            if (i > 0) {
                close(pipes[i - 1][0]);
            }
            if (i < (command_length - 1)) {
                close(pipes[i][1]);
            }
            waitpid(child_pids[i], &status, 0);
        }
    }
    return 0;
}