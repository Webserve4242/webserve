#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "Color.hpp"
#include "Error.hpp"
#include "HttpResponse.hpp"
#include "Result.hpp"

extern char **environ;

// todo: mv Const.h
namespace {

const int OK = 0;
const int ERR = -1;
const int CLOSE_ERROR = -1;
const int DUP_ERROR = -1;
const int FORK_ERROR = -1;
const int SOCKETPAIR_ERROR = -1;

const int FLAG_NONE = 0;
const int CHILD_PROC = 0;

const std::size_t IN = 0;
const std::size_t OUT = 1;

Result<int, std::string> create_socketpair(int socket_fds[2]) {
	std::string err_info;

	errno = 0;
	if (socketpair(AF_UNIX, SOCK_STREAM, FLAG_NONE, socket_fds) == SOCKETPAIR_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp
		return Result<int, std::string>::err(err_info);
	}
	return Result<int, std::string>::ok(OK);
}

std::vector<char *> get_argv_for_execve(const std::vector<std::string> &interpreter,
										const std::string &file_path) {
	Result<std::vector<std::string>, int> result;
	std::vector<std::string>::const_iterator itr;
	std::vector<char *> argv;

	for (itr = interpreter.begin(); itr != interpreter.end(); ++itr) {
		argv.push_back(const_cast<char *>((*itr).c_str()));
	}
	argv.push_back(const_cast<char *>(file_path.c_str()));
	argv.push_back(NULL);
	return argv;
}

int execute_cgi_script_in_child(int socket_fds[2],
							 const std::string &file_path,
							 const std::string &query) {
	Result<std::vector<std::string>, int> interpreter_result;
	std::vector<std::string> interpreter;
	std::vector<char *>argv;
	std::string err_info;
	(void)query;

	interpreter_result = get_interpreter(file_path);
	if (interpreter_result.is_err()) {
		return EXIT_FAILURE;
	}
	interpreter = interpreter_result.get_ok_value();

	argv = get_argv_for_execve(interpreter, file_path);

	errno = 0;
	if (close(socket_fds[IN]) == CLOSE_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp -> log?
		return EXIT_FAILURE;
	}

	errno = 0;
	if (dup2(socket_fds[OUT], STDOUT_FILENO) == DUP_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp -> log?
		return EXIT_FAILURE;
	}

	errno = 0;
	if (close(socket_fds[OUT]) == CLOSE_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp -> log?
		return EXIT_FAILURE;
	}

	execve(argv[0], argv.data(), environ);

	err_info = create_error_info(errno, __FILE__, __LINE__);
	std::cerr << err_info << std::endl;  // todo: tmp -> log?
	return EXIT_FAILURE;
}


}  // namespace

Result<std::vector<std::string>, int> get_interpreter(const std::string &file_path) {
	std::vector<std::string> interpreter;
	std::ifstream		file;
	std::string			shebang_line;
	std::string			word;
	const std::size_t	shebang_len = 2;

	file.open(file_path.c_str());
	if (file.fail()) {
		return Result<std::vector<std::string>, int>::err(ERR);
	}

	std::getline(file, shebang_line);

	std::istringstream	iss(shebang_line);

	while (getline(iss, word, ' ')) {
		interpreter.push_back(word);
	}
	file.close();

	std::vector<std::string>::iterator itr;
	itr = interpreter.begin();
	if (itr == interpreter.end()) {
		return Result<std::vector<std::string>, int>::err(ERR);
	}

	if (shebang_len <= (*itr).length() && (*itr)[0] == '#' && (*itr)[1] == '!') {
		*itr = (*itr).substr(shebang_len);
		return Result<std::vector<std::string>, int>::ok(interpreter);
	}
	return Result<std::vector<std::string>, int>::err(ERR);
}

Result<std::string, int> get_cgi_result_via_socket(int socket_fds[2], int pid) {
	std::string	cgi_result;
	std::string err_info;
	char		buf[BUFSIZ];
	ssize_t 	read_bytes;
	bool		is_error = false;
	Result<int, std::string> socketpair_result, fork_result;
	Result<int, std::string> parent_proc_result;

	errno = 0;
	if (close(socket_fds[OUT]) == CLOSE_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp
		return Result<std::string, int>::err(ERR);
	}

	// errno = 0;
	// if (fcntl(socket_fds[IN], F_SETFL, O_NONBLOCK | FD_CLOEXEC) == ERR) {
	// 	err_info = create_error_info(errno, __FILE__, __LINE__);
	// 	std::cerr << err_info << std::endl;  // todo: tmp
	// 	is_error = true;
	// }

	cgi_result = "";
	while (true) {
		// if (is_error) { break; }
		errno = 0;
		read_bytes = recv(socket_fds[IN], buf, BUFSIZ - 1, FLAG_NONE);
		// read_bytes = recv(socket_fds[IN], buf, BUFSIZ - 1, MSG_DONTWAIT);
		if (read_bytes == -1) {
			err_info = create_error_info(errno, __FILE__, __LINE__);
			std::cerr << err_info << std::endl;  // todo: tmp
			is_error = true;
			break;
		}
		if (read_bytes == 0) {
			break;
		}
		buf[read_bytes] = '\0';
		cgi_result.append(buf);
	}

	errno = 0;
	if (close(socket_fds[IN]) == CLOSE_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp
		return Result<std::string, int>::err(ERR);
	}

	int status = 0;
	errno = 0;
	pid_t ret = waitpid(pid, &status, WNOHANG);
	if (ret == ERR && errno != ECHILD) {
		errno = 0;
		if (kill(pid, SIGKILL) == ERR) {
			err_info = create_error_info(errno, __FILE__, __LINE__);
			std::cerr << err_info << std::endl;  // todo: tmp
			is_error = true;
		}
	}
	// std::cout << CYAN << "errno:" << errno << ", ECHILD:" << ECHILD << RESET << std::endl;

	if (status != 0) {
		is_error = true;
	}

	// if (pid > 0) {
	// 	int status;
	// 	waitpid(pid, &status, 0);
	// }

	if (is_error) {
		return Result<std::string, int>::err(ERR);
	}
	return Result<std::string, int>::ok(cgi_result);
}

Result<std::string, int> HttpResponse::get_cgi_result(const std::string &file_path,
													  const std::string &query) const {
	// std::string	cgi_result;
	// Result<std::string, int> get_cgi_result;
	std::string refactor_content;
	std::string err_info;
	int			socket_fds[2];
	pid_t		pid;
	Result<int, std::string> socketpair_result;

	socketpair_result = create_socketpair(socket_fds);
	if (socketpair_result.is_err()) {
		err_info = socketpair_result.get_err_value();
		std::cerr << "[Error] socketpair: " << err_info << std::endl;  // todo: tmp
		return Result<std::string, int>::err(ERR);
	}

	errno = 0;
	pid = fork();
	if (pid == FORK_ERROR) {
		err_info = create_error_info(errno, __FILE__, __LINE__);
		std::cerr << err_info << std::endl;  // todo: tmp
		return Result<std::string, int>::err(ERR);
	}

	if (pid == CHILD_PROC) {
		std::exit(execute_cgi_script_in_child(socket_fds, file_path, query));
	}
	return get_cgi_result_via_socket(socket_fds, pid);

	// todo: LF -> CRLF, refactor field-lines,...
	// get_cgi_result = get_cgi_result_via_socket(socket_fds, pid);
	// if (get_cgi_result.is_err()) {
		// return Result<std::string, int>::err(ERR);
	// }
	// cgi_result = get_cgi_result.get_ok_value();
}
