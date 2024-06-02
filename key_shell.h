#ifndef INPUT_WITH_ARROWS_H
#define INPUT_WITH_ARROWS_H

#define MAX_INPUT_SIZE 1024
#define MAX_HISTORY_SIZE 100
#define MAX_ARGS 1024
#define MAX_CMD_LEN 1024
#define _XOPEN_SOURCE 700
#define DEBUG 0
#define MAX_CMD_LEN 1024






/**
 * @brief Reads a newline-terminated string from stdin, allowing the use of
 *        arrow keys for cursor movement, backspace and delete for erasing
 *        characters, and up and down arrows for navigating through the history
 *        of previously entered strings.
 *
 * @param input A character array where the input string will be stored.
 *              The array should have enough space to hold the maximum input
 *              size (MAX_INPUT_SIZE).
 * @param prompt The prompt string to display before reading input.
 *
 * @note This function modifies the terminal settings temporarily to handle
 *       input character by character. It restores the original settings
 *       before returning.
 */
char *getInputWithArrows(char* input, char* prompt);

/**
 * @brief Trims leading and trailing whitespace characters from a string.
 *
 * @param str The string to be trimmed.
 */
void trim_spaces(char* str);

/**
 * @brief Sets the value of a variable.
 *
 * @param name The name of the variable.
 * @param value The value to be assigned to the variable.
 */
void set_variable(const char* name, const char* value);

/**
 * @brief Retrieves the value of a variable.
 *
 * @param name The name of the variable.
 * @return The value of the variable, or an empty string if the variable is not found.
 */
char* get_variable(const char* name);

/**
 * @brief Signal handler function for SIGINT (Control-C).
 *
 * @param signum The signal number.
 */
void sigint_handler(int signum);

/**
 * @brief Closes all pipes.
 *
 * @param pipes A 2D array containing the file descriptors for the pipes.
 * @param num_pipes The number of pipes to be closed.
 */
void close_pipes(int (*pipes)[2], int num_pipes);

/**
 * @brief Parses a command string into an array of arguments.
 *
 * @param cmd_str The command string to be parsed.
 * @return An array of strings representing the individual arguments.
 */
// int parse_command(char **args, char *cmd_str);

/**
 * @brief Frees the memory allocated for an array of command arguments.
 *
 * @param cmds The array of command arguments to be freed.
 */
void free_commands(char** cmds);
int execute_shell(char* cmd);
int parse_if();
void wait_for_command();


typedef struct {
    char name[MAX_CMD_LEN];
    char value[MAX_CMD_LEN];
} variable;

extern variable variables[MAX_ARGS];
extern int num_vars;
extern pid_t pids[MAX_ARGS];
extern int num_cmds;

#endif