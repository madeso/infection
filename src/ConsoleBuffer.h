#ifndef __CONSOLE_BUFFER_H
#define __CONSOLE_BUFFER_H

typedef struct _ConsoleBuffer ConsoleBuffer;

char* consoleBuffer_previous(ConsoleBuffer *buffer);
char* consoleBuffer_next(ConsoleBuffer *buffer);
void consoleBuffer_add(ConsoleBuffer* buffer, char* toAdd);
void consoleBuffer_delete(ConsoleBuffer* buffer);
ConsoleBuffer* consoleBuffer_init(unsigned int maximumCount);
char* consoleBuffer_resetCursor(ConsoleBuffer *buffer);
#endif
