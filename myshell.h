#ifndef MYSHELL_H
#define MYSHELL_H

char*  display_prompt();
char** parse_command(char* , char*);
char* remove_whitespace(char*);
#endif