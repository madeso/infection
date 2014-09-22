#ifndef __TIME_OUT_H
#define __TIME_OUT_H

void timeout_init();
void timeout_add(float time, const char* script);
void timeout_iterate(float timePassed);
void timeout_clear();

#endif