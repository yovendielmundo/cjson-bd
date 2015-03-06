/*
 * prompt.h
 *
 *  Created on: 04/07/2012
 */

#ifndef PROMPT_H_
#define PROMPT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROMPT_MAX_LINE_SIZE 100
struct prompt_t{
	char * command;
	char * parameters;
};

void prompt_init();
void prompt_read_command(struct prompt_t * prompt);
void prompt_print(char *title, char * data);
void prompt_destroy();

#endif /* PROMPT_H_ */
