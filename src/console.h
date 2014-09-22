/*
console.h
handles the console, and executes the console messages that the main apllication sends
*/

#ifndef CONSOLE_H
#define CONSOLE_H

void console_message(char* msg);
void clear_console();
void render_console();

void console_get(char* str);
void console_backspace();
void console_put(char c);

void console_init();
void console_delete();

void execute_command(char *command);

void fgetln(char* str, FILE* f);
void execute_scriptFile(char* file);

void console_up();
void console_down();
#endif