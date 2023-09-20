//testing: echo -e "a\nb\nc" | grep b | cat -n 
//testing: /bin/echo foo | /bin/grep -o -m 1 f | /usr/bin/wc -c > /tmp/x

#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "myshell.h"

#define TRUE 1

char* display_prompt(){
    char prompt_text[] = "my_shell$";
    printf("%s ", prompt_text);

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

    fflush(stdout);

    return input;
}

char** parse_command(char* input, char* delimeter){

    char** commands = NULL;
    char** temp_commands;
    int command_count = 0;

    char* token = strtok(input, delimeter);
    char* clean_input;
    while (token != NULL) {
        clean_input = remove_whitespace(token);
        temp_commands = (char**)realloc(commands, (command_count + 1) * sizeof(char*));
        if(!temp_commands){
            printf("%s", "ERROR: memory reallocation failed");
        }
        commands = temp_commands;
        commands[command_count] = clean_input;
        command_count++;

        token = strtok(NULL, delimeter);
    }
    temp_commands = (char**)realloc(commands, (command_count + 1) * sizeof(char*));
    if(!temp_commands){
        printf("%s", "ERROR: memory reallocation failed");
    }
    commands = temp_commands;
    commands[command_count] = NULL;

    return commands;
}

char* remove_whitespace(char* input){
    if (input == NULL) return input;

    char* clean_input = (char*)malloc(strlen(input) + 1);
    if (clean_input == NULL) {
        printf("%s", "ERROR: memory reallocation failed");
    }
    strcpy(clean_input, input);

    int length = strlen(clean_input);
    int i = 0; int j = 0;

    if(isspace(input[0])){
        while (i < length && isspace((unsigned char)clean_input[i])) i++;
        while (i < length) {
            clean_input[j] = clean_input[i];
            i++;
            j++;
        }
        clean_input[j] = '\0';
    }
    return clean_input;
}

int main() {
    char** commands;
    char* input;
    char** execute_command;

    while(TRUE){
        input = display_prompt();
        if(input == NULL) break;
        commands = parse_command(input, "|");
        for (int i = 0; commands[i] != NULL; i++) {
            printf("%s\n", commands[i]);
        }

        int status;
        pid_t pid = fork();
        if(pid != 0){
            waitpid(pid, &status, 0);
        }
        else{
            execute_command = parse_command(commands[0], " ");

            if (strchr(execute_command[0], '/') != NULL) {
                if (execve(execute_command[0], execute_command, NULL) == -1) {
                    perror("ERROR");
                }
            } 
            else {
                if (execvp(execute_command[0], execute_command) == -1) {
                    perror("ERROR");
                    
                }
            }
        }
    }
    return 0;
}



