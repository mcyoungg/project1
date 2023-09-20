#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "myshell.h"

#define TRUE 1

void display_prompt(){
    char prompt_text[] = "my_shell$ ";
    printf("%s ", prompt_text);
}

char** read_command(char* delimeter){
    char* input = readline("my_shell$ ");

    char** commands = NULL;
    char** temp_commands;
    int command_count = 0;

    char* token = strtok(input, delimeter);
    char* clean_input;
    while (token != NULL) {
        clean_input = remove_leading_whitespace(token);
        printf("%s\n", clean_input);

        temp_commands = (char**)realloc(commands, (command_count + 1) * sizeof(char*));
        if(!temp_commands){
            printf("%s", "ERROR: memory reallocation failed");
            exit(EXIT_FAILURE);
        }
        commands = temp_commands;
        commands[command_count] = clean_input;
        command_count++;

        token = strtok(NULL, delimeter);
    }

    return commands;
}

char* remove_leading_whitespace(char* input){
    if (input == NULL) return input;

    char* clean_input = (char*)malloc(strlen(input) + 1);
    if (clean_input == NULL) {
        printf("%s", "ERROR: memory reallocation failed");
        exit(EXIT_FAILURE); 
    }
    strcpy(clean_input, input);

    int length = strlen(clean_input);
    int i = 0;
    int j = 0;

    if(isspace(input[0])){
        while (i < length && isspace(clean_input[i])) i++;
        while (i < length) {
            clean_input[j] = clean_input[i];
            i++;
            j++;
        }
        clean_input[j] = '\0';
    }

    int k = strlen(clean_input) - 1;
    if(iispace(input[k])){
        while (i >= 0 && isspace((unsigned char)clean_input[i])) {
            i--;
        }
        clean_input[i] = '\0';
    }
    
    return clean_input;
}

// char** parse_command(char* command){

// }

int main() {
    int p = 5; //for testing
    char** commands;
    char *envp[] = {"PATH=/bin:/usr/bin"};
    char** execute_command;

    while(p){
        //display_prompt();
        commands = read_command("|");

        int *status;
        if(fork() != 0){
            waitpid(-1, status, 0);
        }
        else{
            for(int i = 0; *commands[i] != NULL; i++) {
                execute_command = parse_command(*commands[i]);
                execve(*execute_command[i], execute_command, envp);
            }
        }
        p--;
        
    }



    return 0;
}



