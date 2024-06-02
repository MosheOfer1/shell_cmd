#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include "key_shell.h"

char prompt[50] = "hello"; // Default prompt
char cmd[MAX_CMD_LEN] = {'0'};
char last_command[MAX_CMD_LEN] = {'0'};
int last_status = 0;

void reset_fds(int input_fd, int output_fd, int error_fd)
{
	// Reset standard input
	dup2(input_fd, STDIN_FILENO);
	// Reset standard output
	dup2(output_fd, STDOUT_FILENO);
	// Reset standard error
	dup2(error_fd, STDERR_FILENO);
}

void handle_read_command(char *var_name)
{
	char input[MAX_INPUT_SIZE];

	if (fgets(input, sizeof(input), stdin) != NULL)
	{
		size_t len = strlen(input);
		if (len > 0 && input[len - 1] == '\n')
		{
			input[len - 1] = '\0';
		}
		set_variable(var_name, input);
	}
	else
	{
		fprintf(stderr, "Error reading input\n");
	}
}

int execute_shell(char *cmd)
{
	int i, amper;
	int pipes[MAX_ARGS][2];
	char **cmds[MAX_CMD_LEN];
	char *args[MAX_ARGS];
	char *token;

	for (int i = 0; i < MAX_CMD_LEN; i++)
	{
		cmds[i] = NULL;
	}

	int output_fd = STDOUT_FILENO; // default output is stdout
	int error_fd = STDERR_FILENO;
	int input_fd = STDIN_FILENO;
	int retid, status = 0;

	num_cmds = 0;
	output_fd = STDOUT_FILENO; // default output is stdout
	error_fd = STDERR_FILENO;  // default error is stderr
	reset_fds(input_fd, output_fd, error_fd);
	// printf("expected cmd:  %s\n",cmd);

	if (strcmp(cmd, "!!") == 0)
	{
		if (strlen(last_command) == 0)
		{
			printf("No previous command to repeat\n");
			return last_status;
		}
		strcpy(cmd, last_command);
	}
	else
	{
		strcpy(last_command, cmd);
	}

	/* Handle quit command */
	if (strcmp(cmd, "quit") == 0)
	{
		last_status = 0;
		exit(0);
	}

	/* Check if the user wants to change the prompt */
	if (strncmp(cmd, "prompt =", 8) == 0)
	{
		char *new_prompt = cmd + 8;
		while (*new_prompt == ' ')
			new_prompt++; // Skip leading spaces
		strncpy(prompt, new_prompt, sizeof(prompt) - 1);
		prompt[sizeof(prompt) - 1] = '\0'; // Ensure null-termination
		return last_status;
	}
	// Handle "read" command
	if (strncmp(cmd, "read ", 5) == 0)
	{
		char *var_name = cmd + 5;
		trim_spaces(var_name);
		handle_read_command(var_name);
		return last_status;
	}
	if (strncmp(cmd, "echo $?", 7) == 0)
	{
		char echo_var[10];
		char echo_value[10];
		snprintf(echo_var, sizeof(echo_var), "%c", '?');
		snprintf(echo_value, sizeof(echo_value), "%d", last_status);

		set_variable(echo_var, echo_value);
	}

	// Handle variable assignment
	if (strchr(cmd, '=') != NULL)
	{
		char *name = strtok(cmd, "=");
		char *value = strtok(NULL, "=");
		if (name && value)
		{
			// Trim leading/trailing spaces from name and value
			trim_spaces(name);
			trim_spaces(value);

			// Remove leading $ if it exists
			if (*name == '$')
			{
				name++; // Move the pointer to skip the $
			}

			set_variable(name, value);
		}
	}

	i = 0;
	token = strtok(cmd, " ");
	while (token != NULL)
	{
		// pipe
		if (strcmp(token, "|") == 0)
		{
			args[i] = NULL;
			cmds[num_cmds] = (char **)calloc(MAX_ARGS, sizeof(char *));

			for (int x = 0; x < i; x++)
			{
				cmds[num_cmds][x] = (char *)calloc((i + 1), sizeof(char));
				strcpy(cmds[num_cmds][x], args[x]);
			}
			num_cmds++;
			i = 0;
			token = strtok(NULL, " ");
			continue;
		}
		// redirect
		else if (((strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || strcmp(token, "<") == 0) || strcmp(token, "2>") == 0) && i > 0)
		{
			args[i] = NULL;
			cmds[num_cmds] = (char **)calloc(MAX_ARGS, sizeof(char *));
			for (int x = 0; x < i; x++)
			{
				cmds[num_cmds][x] = (char *)calloc((i + 1), sizeof(char));
				strcpy(cmds[num_cmds][x], args[x]);
			}
			char *sign = token;
			token = strtok(NULL, " ");
			// Redirect output to file
			if (strcmp(sign, ">") == 0)
			{
				output_fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			}
			else if (strcmp(sign, "<") == 0)
			{
				input_fd = open(token, O_RDONLY);
			}
			// ">>" appends
			else if (strcmp(sign, ">>") == 0)
			{
				output_fd = open(token, O_WRONLY | O_CREAT | O_APPEND, 0644);
			}
			else
			{
				error_fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			}

			if (output_fd == -1 || input_fd == -1 || error_fd == -1)
			{
				if (DEBUG)
					perror("open");
			}
			token = strtok(NULL, " ");
			continue;
		}

		if (token[0] == '$')
		{
			char *var_name = token + 1; // Skip the $
			char *var_value = get_variable(var_name);
			args[i] = strdup(var_value);
		}
		else
		{
			args[i] = token;
		}
		token = strtok(NULL, " ");
		i++;
	}
	args[i] = NULL;

	/* Does command line end with & */
	if (i > 1 && !strcmp(args[i - 1], "&"))
	{
		amper = 1;
		args[i - 1] = NULL;
		i--;
	}
	else
	{
		amper = 0;
	}

	/* Check if the command is "cd" */
	if (strncmp(cmd, "cd", 2) == 0) // Check if the command is "cd"
	{
		if (args[1] != NULL)
		{
			if (chdir(args[1]) != 0)
			{
				perror("chdir failed");
			}
		}
		else
		{
			// Handle "cd" without arguments
			// Change directory to home directory
			if (chdir(getenv("HOME")) != 0)
			{
				perror("chdir failed");
			}
		}
		last_status = -1;
		return last_status;
	}

	cmds[num_cmds] = (char **)calloc(MAX_ARGS, sizeof(char *));
	for (int x = 0; x < MAX_ARGS; x++)
	{
		cmds[num_cmds][x] = NULL;
	}
	for (int x = 0; x < i; x++)
	{
		cmds[num_cmds][x] = (char *)calloc((i + 1), sizeof(char));
		strcpy(cmds[num_cmds][x], args[x]);
	}
	num_cmds++;

	if (DEBUG)
		printf("num of commends: %d\n", num_cmds);

	// Execute the commands using pipes and forks
	for (i = 0; i < num_cmds; i++)
	{
		pipe(pipes[i]);
	}

	for (i = 0; i < num_cmds; i++)
	{
		pids[i] = fork();
		if (pids[i] == 0)
		{
			// Child process
			if (i > 0)
			{
				// Not the first
				// Redirect input from the pipe of the previous command
				if (DEBUG)
					printf("input: prev[0] = %d, from = %s\n", pipes[i - 1][0], cmds[i][0]);
				dup2(pipes[i - 1][0], STDIN_FILENO);
			}

			if (i < num_cmds - 1)
			{
				// Not the last
				// Redirect output to the pipe of the next command
				if (DEBUG)
					printf("output: p[1] = %d, from = %s\n", pipes[i][1], cmds[i][0]);
				dup2(pipes[i][1], STDOUT_FILENO);
			}

			// The last one
			else
			{
				dup2(output_fd, STDOUT_FILENO);
			}

			if (input_fd != -1)
			{
				// Redirect input from the file descriptor
				dup2(input_fd, STDIN_FILENO);
				// close(input_fd); // Close the original file descriptor
			}
			if (error_fd != -1)
			{
				// Redirect input from the file descriptor
				dup2(error_fd, STDERR_FILENO);
				close(error_fd); // Close the original file descriptor
			}

			close_pipes(pipes, num_cmds - 1);

			trim_spaces(cmds[i][0]);

			if (execvp(cmds[i][0], cmds[i]) < 0)
			{
				if (DEBUG)
					perror("execvp");
			}
			close_pipes(pipes, num_cmds - 1);
			last_status = -1;
			exit(1);
		}
		else if (pids[i] > 0)
		{
			// Parent
		}
		else
		{
			last_status = -1;
			perror("fork");
			exit(1);
		}
	}

	// Closing all pipes
	if (DEBUG)
		printf("parent: close all pipes\n");
	close_pipes(pipes, num_cmds - 1);

	// Wait for all child processes to finish
	status = 0;
	for (int p1 = 0; p1 < num_cmds; p1++)
	{
		if (DEBUG)
			printf("parent: wait to %d\n", pids[p1]);
		retid = waitpid(pids[p1], &status, 0);
		if (amper == 0)
		{
			if (DEBUG)
				printf("wait: %d\n", retid);
			if (WIFEXITED(status))
			{
				last_status = WEXITSTATUS(status);
			}
			else
			{
				last_status = -1; // Indicate abnormal termination
			}
		}
	}

	if (DEBUG)
		printf("All children died\n");

	// Free the allocated bytes of cmds
	for (int x = 0; x < num_cmds; x++)
	{
		int y = 0;
		while (cmds[x][y])
		{
			cmds[x][y] = NULL;
			free(cmds[x][y]);
		}
		free(cmds[x]);
	}
	return status;
}

void wait_for_command()
{
	strcpy(cmd, "");
	while (strcmp(cmd, "") == 0)
	{
		printf("%s: ", prompt);
		getInputWithArrows(cmd, prompt);
	}
}

int parse_if()
{

	char if_condition[MAX_CMD_LEN] = {'0'};
	char then_condition[MAX_CMD_LEN] = {'0'};
	char else_condition[MAX_CMD_LEN] = {'0'};

	strcpy(if_condition, cmd + 3);

	char old_prompt[MAX_INPUT_SIZE];
	strcpy(old_prompt, prompt);
	strcpy(prompt, "if> ");
	while (strncmp(cmd, "then ", 4))
	{
		wait_for_command();
	}
	strcpy(prompt, "then> ");

	// input for then cond.
	wait_for_command();

	strcpy(then_condition, cmd);

	while (strncmp(cmd, "else", 4))
	{
		wait_for_command();
	}
	strcpy(prompt, "else> ");
	wait_for_command();

	strcpy(else_condition, cmd);

	while (strncmp(cmd, "fi", 2))
	{
		wait_for_command();
	}

	execute_shell(if_condition);
	if (last_status == 0)
	{
		execute_shell(then_condition);
	}
	else
	{
		execute_shell(else_condition);
	}
	strcpy(prompt, old_prompt);
	return 0;
}

int main()
{
	int if_command = 0;

	struct sigaction sa = {
		.sa_handler = sigint_handler,
		.sa_flags = 0,
		.sa_mask = {{0}}};

	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		last_status = -1;
		exit(1);
	}

	while (!if_command)
	{
		wait_for_command();

		if (strncmp(cmd, "if ", 3) == 0)
		{
			parse_if();
			continue;
		}
		execute_shell(cmd);
	}
}
