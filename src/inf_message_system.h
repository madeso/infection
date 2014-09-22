/***************************************************
*           inf_message_system.h                  *
*      written by Gustav 'sirGustav' Jansson       *
*         for his first 3d game 'Infection'        *
*             started: 2002-07-19                  *
*             ended:                               *
*              mail: sir_gustav@passagen.se        *
*    URL: http://www.geocities.com/sir_gustavs     *
****************************************************/

/*
inf_message_system.h
handles system and game messages. Game messages are at the top left
of the screen, and the system messages are at the bottom left of the
screen. The most recent message is the message that is closest to
the screen boundary.
*/

#include "genesis.h"

#ifndef INF_MESSAGE_SYSTEM_H
#define INF_MESSAGE_SYSTEM_H

//initialize the message system
void init_messages();

//input a system message
void system_message(const char *str);

// add another game message
void game_message(const char *sz);

//render the messages
geBoolean render_messages();

//iterates the messages
void tick_messages();

//sets the y shift
void set_y_shift(unsigned char y);

#endif