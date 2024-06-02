# MyShell Project

## Overview

MyShell is a custom shell program implemented in C, allowing users to execute commands, manage environment variables, handle I/O redirection, and use pipelines. It also supports custom prompt settings and basic control structures like `if-then-else`.

## Files

- **shell.c**: Main source file containing the shell's core logic.
- **key_shell.c**: Source file with additional helper functions.
- **key_shell.h**: Header file for `key_shell.c`.
- **utilty.c**: Utility functions used by the shell.

## Compilation

To compile the MyShell program, use the provided Makefile. Run the following command in the terminal:

```sh
make
```
This will generate an executable file named myshell.
Usage

To start the MyShell program, run:

```sh
./myshell
```
## Supported Features

Basic Commands: Execute standard UNIX commands.
Pipelines: Use | to pipe the output of one command to another.
I/O Redirection: Redirect input and output using >, >>, <, and 2>.
Environment Variables: Set and use environment variables with the syntax variable=value and $variable.
Custom Prompt: Change the shell prompt with prompt = <new_prompt>.
Control Structures: Basic if-then-else structure for conditional execution.

### Example Commands

Execute a command: ls -l
Set a variable: myvar=value
Use a variable: echo $myvar
Change prompt: prompt = myshell>
Redirect output: ls > output.txt
Pipe commands: ls | grep .c
Conditional execution:

```sh

if ls | grep myfile
then echo "This is true"
else echo "This is false"
fi
```

### Signal Handling

The shell includes a handler for the SIGINT signal (Ctrl+C), which prints a message instead of terminating the shell.
### Cleanup

To clean up the generated files, use:

```sh
make clean
```
Authors

    Moshe & Matanya