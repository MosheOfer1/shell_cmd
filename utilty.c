#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include "key_shell.h"

void trim_spaces(char *str)
{
	char *start, *end;

	// Find the start of the trimmed string
	start = str;
	while (*start && isspace((unsigned char)*start))
	{
		start++;
	}

	// Find the end of the trimmed string
	end = str + strlen(str) - 1;
	while (end > start && isspace((unsigned char)*end))
	{
		end--;
	}

	// Shift the trimmed string to the start of the buffer
	if (start > str)
	{
		memmove(str, start, end - start + 1);
	}

	// Null terminate the string after the last non-space character
	str[end - start + 1] = '\0';
}

pid_t pids[MAX_ARGS];
int num_cmds;

variable variables[MAX_ARGS];
int num_vars = 0;


void pass_args_to_commands(char **args, char **commands) {
    int i = 0;
    while (args[i] != NULL) {
        commands[i] = strdup(args[i]); // Dynamically allocate memory for each argument
        i++;
    }
    commands[i] = NULL; // Add a NULL pointer at the end
}

void set_variable(const char *name, const char *value)
{
	for (int i = 0; i < num_vars; i++)
	{
		if (strcmp(variables[i].name, name) == 0)
		{
			strncpy(variables[i].value, value, MAX_CMD_LEN);
			return;
		}
	}
	strncpy(variables[num_vars].name, name, MAX_CMD_LEN);
	strncpy(variables[num_vars].value, value, MAX_CMD_LEN);
	num_vars++;
	if (DEBUG)
		printf("set_variable: %s = %s\n", name, value);
}

char *get_variable(const char *name)
{
	for (int i = 0; i < num_vars; i++)
	{
		if (strcmp(variables[i].name, name) == 0)
		{
			if (DEBUG)
				printf("get var %s\n", variables[i].value);
			return variables[i].value;
		}
		else if (DEBUG)
			printf("var number %d is %s\nname is %s\n", i, variables[i].name, name);
	}
	return "";
}

// Signal handler function
void sigint_handler(int signum)
{
	(void)signum;  // Suppress unused parameter warning
	printf("You typed Control-C!");
}

void close_pipes(int (*pipes)[2], int num_pipes)
{
	// Close all pipes
	for (int p = 0; p < num_pipes; p++)
	{
		int c1 = close(pipes[p][0]);
		int c2 = close(pipes[p][1]);
		if (DEBUG)
			fprintf(stderr, "%d code when closed [%d] [0] by %d\n", c1, p, getpid());
		if (DEBUG)
			fprintf(stderr, "%d code when closed [%d] [1] by %d\n", c2, p, getpid());
	}
}


void free_commands(char **cmds)
{
	int i = 0;
	while (cmds[i] != NULL)
	{
		free(cmds[i]);
		i++;
	}
	free(cmds);
}
