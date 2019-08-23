#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CREATED    1
#define READY      2
#define RUNNING    3
#define TERMINATED 0

char** split(char *cmd);
void free_args(char **args);
pid_t execute(char *pgm, char **args);


#endif