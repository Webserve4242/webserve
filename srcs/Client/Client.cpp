#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "webserv.hpp"
#include "Color.hpp"
#include "Constant.hpp"
#include "Client.hpp"
#include "Debug.hpp"

namespace {

int create_connect_socket() {
	int connect_fd;
	errno = 0;
	connect_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect_fd == ERR) {
		std::string err_str = "[Client Error] socket: " + std::string(strerror(errno));
		throw std::runtime_error(RED + err_str + RESET);
	}
	return connect_fd;
}

struct sockaddr_in create_connect_addr(const char *server_ip,
									   const char *server_port) {
	struct sockaddr_in	addr = {};
	const int 			DECIMAL_BASE = 10;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(std::strtol(server_port, NULL, DECIMAL_BASE));
	addr.sin_addr.s_addr = inet_addr(server_ip);
	return addr;
}

void connect_to_server(int connect_fd, struct sockaddr_in addr) {
	socklen_t len = sizeof(addr);

	errno = 0;
	if (connect(connect_fd, (struct sockaddr *)&addr, len) == ERR) {
		std::string err_str = "[Client Error] connect: " + std::string(strerror(errno));
		throw std::runtime_error(RED + err_str + RESET);
	}
}

void send_to_server(int connect_fd, const std::string &send_msg) {
	ssize_t	send_size;
	size_t	msg_len = send_msg.size() + 1;
	const char *msg = send_msg.c_str();

	errno = 0;
	send_size = send(connect_fd, msg, msg_len, MSG_DONTWAIT);
	if (send_size == SEND_ERROR) {
		std::string err_str = "[Client Error] send: " + std::string(strerror(errno));
		throw std::runtime_error(RED + err_str + RESET);
	}
}

std::string recv_message_from_server(int connect_fd) {
	ssize_t		recv_size;
	char		buf[BUFSIZ + 1];
	std::string	recv_msg;

	while (true) {
		errno = 0;
		recv_size = recv(connect_fd, buf, BUFSIZ, 0);
		if (recv_size == RECV_ERROR) {
			std::string err_str = "[Client Error] recv: " + std::string(strerror(errno));
			throw std::runtime_error(RED + err_str + RESET);
		}
		buf[recv_size] = '\0';
		recv_msg += buf;
		if (recv_size < BUFSIZ) {
			break;
		}
	}
	// std::cout << "recv_size:" << recv_size << std::endl;
	return recv_msg;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

Client::Client(const char *server_ip, const char *server_port) {
	this->connect_fd_ = create_connect_socket();
	this->addr_ = create_connect_addr(server_ip, server_port);
}

Client::~Client() {
	if (connect_fd_ != ERR) {
		errno = 0;
		if (close(connect_fd_) == ERR) {
			std::cerr << RED "[Client Error] close: " << strerror(errno) << RESET << std::endl;
		}
        connect_fd_ = ERR;
	}
}

void Client::process_server_connect(const std::string &send_msg) {
	connect_to_server(this->connect_fd_, this->addr_);
	send_to_server(this->connect_fd_, send_msg);
	this->recv_message_ = recv_message_from_server(this->connect_fd_);
}

std::string Client::get_recv_message() const { return this->recv_message_; }
