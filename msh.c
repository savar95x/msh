#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define PROMPT "> "

#define READLINE_BUF 1024

#define TOK_BUF 64
#define TOK_DELIM " \t\r\n\a"

// takes a line as input, till EOF or \n
char *read_line() {
	// we'll have to increase this if we overflow
	int bufsize = READLINE_BUF;
	int i = 0;
	char *buffer = malloc(sizeof(char) * bufsize);

	// equivalent to zsh segmentation fault ?
	if (!buffer) {
		fprintf(stderr, "msh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	int c;
	while (1) {
		c = getchar();
		if (c == EOF || c == '\n') { // end loop
			buffer[i] = '\0';
			return buffer;
		} else {
			buffer[i] = c;
		}
		i++;

		// if we exceed the buffer, reallocate bigger memory
		if (i >= bufsize) {
			bufsize += READLINE_BUF;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "msh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

// splits arguments using strtok and returns an array of strings
char **parse_line(char *line) {
	int bufsize = TOK_BUF, i = 0;
	char **tokens = malloc(sizeof(char*) * bufsize);
	char *token;

	if (!tokens) {
		fprintf(stderr, "msh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, TOK_DELIM);
	while (token != NULL) {
		tokens[i++] = token;
		if (i >= TOK_BUF) { // realloc
			bufsize += TOK_BUF;
			tokens = realloc(tokens, bufsize);
			if (!tokens) {
				fprintf(stderr, "msh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, TOK_DELIM);
	}
	tokens[i] = NULL;
	return tokens;
}

// executes the command
// the idea is, in linux,
// we duplicate (fork) a process,
// then replace (exec) the child with what we want to run
int exec_line(char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork(); // this forks the init ? and gives it a different pid
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("msh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Error forking
		perror("msh");
	} else {
		// Parent process
		do {
			// okay WHAT THE FUCK are we doin here ?
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

void main_loop() {
	char *line;
	char **args;
	int status;

	do {
		printf(PROMPT);
		// read
		line = read_line();
		// parse
		args = parse_line(line);
		 // execute
		status = exec_line(args);

		// free mem
		free(line);
		free(args);
	} while (status);
}

int main() {
	main_loop();
	return EXIT_SUCCESS;
}
