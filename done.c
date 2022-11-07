#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void	put_char(char c) {
	write(STDERR_FILENO, &c, 1);
}

void	put_str(char *s) {
	int i = -1;

	while (s[++i])
		put_char(s[i]);
}

void	print_error(char *err, char *arg) {
	put_str(err);
	if (arg)
		put_str(arg);
	put_char('\n');
}

void	fatal_exit() {
	print_error("error: fatal", NULL);
	exit(1);
}

int	is_pipe(char **argv) {
	int i = 0;
	int ret = 0;

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

void	my_cd(char **argv) {
	int	i = 0;

	while (argv[i])
		i++;
	if (i != 2){
		print_error("error: cd: bad arguments", NULL);
		return ;
	}
	if (chdir(argv[1]) < 0)
		print_error("error: cd: cannot change directory to ", argv[i]);
}

void	command(int argc, char **argv, char **envp, int prev, int* fd_prev) {
	int next;
	int fd_next[2];
	pid_t pid;
	int i;
	int exit_value;

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
			if (execve(argv[0], argv, envp) < 0) {
				print_error("error: cannot execute ", argv[0]);
				exit (-1);
			}
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
	int	fd_prev[2];

	fd_prev[0] = 0;
	fd_prev[1] = 1;
	command(argc, argv + 1, envp, prev, fd_prev);
	return(0);
}