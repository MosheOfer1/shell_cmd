#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "key_shell.h"

#define MAX_INPUT_SIZE 1024
#define MAX_HISTORY_SIZE 100

static char history[MAX_HISTORY_SIZE][MAX_INPUT_SIZE];
static int history_count = 0;
static int history_index = 0;

char *getInputWithArrows(char* input, char* prompt) {
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

    int pos = 0;
    int len = 0;
    int c;

    while ((c = getchar()) != '\n') {
        switch (c) {
            case 127: // Backspace
            case '\b':
                if (pos > 0) {
                    input[--pos] = '\0';
                    len--;
                    printf("\b \b");
                }
                break;
            case 27: // Arrow key
                getchar(); // Consume '['
                switch (getchar()) {
                    case 'D': // Left arrow
                        if (pos > 0) {
                            pos--;
                            printf("\033[1D");
                        }
                        break;
                    case 'C': // Right arrow
                        if (pos < len) {
                            pos++;
                            printf("\033[1C");
                        }
                        break;
                    case 'A': // Up arrow
                        if (history_index > 0) {
                            history_index--;
                            strcpy(input, history[history_index]);
                            len = strlen(input);
                            pos = len;
                            printf("\r\033[K%s: %s", prompt, input);
                        }
                        break;
                    case 'B': // Down arrow
                        if (history_index < history_count - 1) {
                            history_index++;
                            strcpy(input, history[history_index]);
                            len = strlen(input);
                            pos = len;
                            printf("\r\033[K%s: %s", prompt, input);
                        } else if (history_index == history_count - 1) {
                            input[0] = '\0';
                            len = 0;
                            pos = 0;
                            printf("\r\033[K%s: ", prompt);
                        }
                        break;
                }
                break;
            default:
                if (len < MAX_INPUT_SIZE - 1) {
                    input[pos++] = c;
                    len++;
                    input[len] = '\0';
                    printf("%s", input + pos - 1);
                }
                break;
        }
    }

    if (len > 0) {
        strncpy(history[history_count % MAX_HISTORY_SIZE], input, MAX_INPUT_SIZE);
        history_count++;
        history_index = history_count;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    printf("\n");
    trim_spaces(input);
    return input;
}