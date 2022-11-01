// pipe - cd - fork - pid - dup2 - dup2 - execve - waitpid - close

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void	fatal_exit() {
	return ;
}

void	print_error() {
	return ;
}

void	my_cd(char **argv) {
	int i = 0;
	while (argv[i])
		i++;
	if (i != 2) {
		print_error();
		return ;
	}
	if (chdir(argv[1]) < 0)
		print_error();
}

int	is_pipe(char **argv) {
	int ret = 0;
	int i = 0;

	while (argv[i]) {
		if (strncmp(argv[i], "|", 2) == 0) {
			ret = 1;
			argv[i] = 0;
			break ;
		}
		if (strncmp(argv[i], ";", 2) == 0) {
			ret = 0;
			argv[i] = 0;
			break ;
		}
		i++;
	}
	return (ret);
}

void	command(int argc, char **argv, char **envp, int prev, int* fd_prev) {
	int next;
	int fd_next[2];
	int exit_value;
	pid_t pid;
	int i;

	if (argc <= 1)
		return ;
	next = is_pipe(argv);
	if (argv[0]) {
		if (next && pipe(fd_next) < 0)
			fatal_exit();
		if (strncmp(argv[0], "cd", 3) == 0)
			my_cd(argv);
		else if ((pid = fork()) < 0)
			fatal_exit();
		else if (pid == 0) {
			if (prev && dup2(fd_prev[0], 0) < 0)
				exit (-1);
			if (next && dup2(fd_next[1], 1) < 0)
				exit (-1);
			if (execve(argv[0], argv, envp))
				print_error();
		}
		else {
			if (waitpid(pid, &exit_value, 0) < 0)
				fatal_exit();
			if (prev)
				close(fd_prev[0]);
			if (next)
				close(fd_next[1]);
		}
	}
	i = 0;
	while (argv[i])
		i++;
	command(argc - 1 - i, argv + 1 + i, envp, next, fd_next);
}

int	main(int argc, char **argv, char **envp) {
	int prev = 0;
	int fd_prev[2];

	fd_prev[0] = 0;
	fd_prev[1] = 1;
	command(argc, argv + 1, envp, prev, fd_prev);
	return (0);
}