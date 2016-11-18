

//============================================================================
// Name        : SimpleLinuxShell
// Authors     : Keyur Golani | Ved Vasavada | Arpit Desai
// Version     : 1.0
// Copyright   : ©2016 Keyur Golani | Ved Vasavada | Arpit Desai All Rights Reserved
// Description : A Simple Linux Shell in C Language, Ansi-style
//============================================================================


#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * String Array of the commands supported by the Simple Linux Shell.
 * Will provide three commands.
 * 1. cd (cd)
 * 2. help (help)
 * 3. exit (exit)
 */
char *command_bulitin[] = {
	"cd",
	"help",
	"exit"
};

/*
 * SimpleLinuxShell will provide three commands.
 * Declaring the functions that will handle the requests of each command.
 */
int cd_command(char **args);
int help_command(char **args);
int exit_command(char **args);

int (*command_handler[]) (char **) = {
	&cd_command,
	&help_command,
	&exit_command
};

/*
 * Gives Number of Commands Published in Bulitin
 * that needs to be handled by the code
 */
int bulitin_commands_count() {
	return sizeof(command_bulitin) / sizeof(char *);
}

/*
 * Implementing the functionality handled by SimpleLinuxShell
 */

/*
 * @brief Shell Command: change directory.
 * @param args List of arguments.  args[0] is "cd" itself.  args[1] is the directory name.
 * @return It always returns 1, to continue executing.
 */
int cd_command(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "SimpleLinuxShell: Argument To \"cd\" Cannot Be Empty\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("SimpleLinuxShell");
		}
	}
	return 1;
}

/*
 * @brief Shell Command: help.
 * @param List of args not needed. Not expecting any args.
 * @return Always returns 1, to continue executing.
 */
int help_command(char **args) {
	int i;
	printf("Name\t\t:\tSimpleLinuxShell\n");
	printf("Authors\t\t:\tKeyur Golani | Ved Vasavada | Arpit Desai\n");
	printf("Version\t\t:\t1.0\n");
	printf("Copyright\t:\t©2016 Keyur Golani | Ved Vasavada | Arpit Desai All Rights Reserved\n");
	printf("Description\t:\tA Simple Linux Shell in C Language, Ansi-style\n");
	printf("Usage:\n");
	printf("\tType program names and arguments, and hit enter.\n");
	printf("\tThe following are built in:\n");

	for (i = 0; i < bulitin_commands_count(); i++) {
		printf("\t  %s\n", command_bulitin[i]);
	}

	printf("\tFor the other commands, use man command to get usage information.\n");
	return 1;
}

/*
 * @brief Shell Command: exit.
 * @param List of args not needed. Not expecting any args.
 * @return Always returns 0, exit the shell prompt loop.
 */
int exit_command(char **args) {
	return 0;
}

/*
 * @brief Launches a program in a new thread/process
 * @param args Null terminated list of arguments (including program).
 * @return Always returns 1, to continue execution.
 */
int launch(char **args) {
	pid_t pid;
	int return_status;

	pid = fork();
	if (pid == 0) {
		// Creats a child process to run new program in
		if (execvp(args[0], args) == -1) {
			perror("SimpleLinuxShell");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Process forking error
		perror("SimpleLinuxShell");
	} else {
		// Waiting parent process for tthe signal or exit of child process.
		do {
			waitpid(pid, &return_status, WUNTRACED);
		} while (!WIFEXITED(return_status) && !WIFSIGNALED(return_status));
	}
	return 1;
}

/*
 * @brief Exeute the unimplemented commands from linux shell
 * @param args Null terminated list of arguments.
 * @return 1 if the shell should continue running, 0 if it should terminate
 */
int execute_command(char **args) {
	int i;

	if (args[0] == NULL) {
		// Enter Hit.
		return 1;
	}

	for (i = 0; i < bulitin_commands_count(); i++) {
		if (strcmp(args[0], command_bulitin[i]) == 0) {
			return (*command_handler[i])(args);
		}
	}
	return launch(args);
}

#define RL_BUFFERSIZE 1024
/*
 * @brief Reading line from console (stdin)
 * @return The line from stdin.
 */
char *read_line(void) {
	int buffer_size = RL_BUFFERSIZE;
	int position = 0;
	char *buf = malloc(sizeof(char) * buffer_size);
	int input;

	if (!buf) {
		fprintf(stderr, "Error in allocating memory\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		// Reading a character
		input = getchar();

		// On End Of File (EOF), replace it with a null character and return.
		if (input == EOF || input == '\n') {
			buf[position] = '\0';
			return buf;
		} else {
			buf[position] = input;
		}
		position++;

		// When we exceed the buffer capacity, reallocate.
		if (position >= buffer_size) {
			buffer_size += RL_BUFFERSIZE;
			buf = realloc(buf, buffer_size);
			if (!buf) {
				fprintf(stderr, "Error in allocating memory\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_SEPARATOR " \t\r\n\a"
/**
   @brief Splits the command into tokens delimited by the predifined separators.
   @param line.
   @return Null terminated array of tokens.
 */
char **split_line(char *line) {
	int buffer_size = TOKEN_BUFFER_SIZE, position = 0;
	char **tokens = malloc(buffer_size * sizeof(char*));
	char *token, **tokens_collection;

	if (!tokens) {
		fprintf(stderr, "Error in allocating memory\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, TOKEN_SEPARATOR);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= buffer_size) {
			buffer_size += TOKEN_BUFFER_SIZE;
			tokens_collection = tokens;
			tokens = realloc(tokens, buffer_size * sizeof(char*));
			if (!tokens) {
				free(tokens_collection);
				fprintf(stderr, "Error in allocating memory\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, TOKEN_SEPARATOR);
	}

	tokens[position] = NULL;
	return tokens;
}

/**
   @brief SimpleLinuxShell prompt loop to get the command and execute it
 */
void prompt(void) {
	char *command;
	char **args;
	int return_status;
	char cwd[1024];

	do {
		getcwd(cwd, sizeof(cwd));
		printf("%s> ", cwd);
		command = read_line();
		args = split_line(command);
		return_status = execute_command(args);

		free(command);
		free(args);
	} while (return_status);
}

/**
   @brief Main method.
   @param argc Argument count.
   @param argv Argument vector.
   @return return_status code
 */
int main(int argc, char **argv) {
	// Loop prompting the user for command
	prompt();

	return EXIT_SUCCESS;
}

