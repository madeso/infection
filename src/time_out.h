#ifndef __TIME_OUT_H
#define __TIME_OUT_H

void timeout_init();
void timeout_add(float time, char* script, char type);
void timeout_iterate(float timePassed);
void timeout_clear();

#endif