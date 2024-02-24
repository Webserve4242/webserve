#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <iostream>
#include <set>
#include "webserv.hpp"
#include "Color.hpp"
#include "Constant.hpp"
#include "Config.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "IOMultiplexer.hpp"
#include "Result.hpp"
#include "Server.hpp"

namespace {

const int MAX_SESSION = 128;


void stop_by_signal(int sig) {
    switch (sig) {
        case SIGINT:
            std::cout << "[Server] Running stop" << std::endl;
            std::exit(EXIT_SUCCESS);

        case SIGTERM:
            std::cout << "[Server] Running stop by SIGTERM" << std::endl;
            std::exit(EXIT_SUCCESS);

        case SIGABRT:
            std::cout << "[Error] Server abort" << std::endl;
            std::exit(EXIT_FAILURE);

        default:
            std::cout << "Received unknown signal: " << sig << std::endl;
    }
}


ServerResult set_signal() {
    return ServerResult::ok(OK);

    // todo
	errno = 0;
	if (signal(SIGABRT, stop_by_signal) == SIG_ERR) {
		const std::string error_msg = CREATE_ERROR_INFO_ERRNO(errno);
		return ServerResult::err(error_msg);
	}
	if (signal(SIGINT, stop_by_signal) == SIG_ERR) {
		const std::string error_msg = CREATE_ERROR_INFO_ERRNO(errno);
		return ServerResult::err(error_msg);
	}
	if (signal(SIGTERM, stop_by_signal) == SIG_ERR) {
		const std::string error_msg = CREATE_ERROR_INFO_ERRNO(errno);
		return ServerResult::err(error_msg);
	}
	return ServerResult::ok(OK);
}


}  // namespace

////////////////////////////////////////////////////////////////////////////////

Server::Server(const Config &config)
	: sockets_(),
      fds_(NULL),
      config_(config) {}


Server::~Server() {
    clear_sessions();
    delete_sockets();
    delete this->fds_;
}

////////////////////////////////////////////////////////////////////////////////

ServerResult Server::init() {
    ServerResult socket_result = create_sockets(this->config_);
    if (socket_result.is_err()) {
        const std::string socket_err_msg = socket_result.get_err_value();
        std::ostringstream oss;
        oss << RED << "[Server Error] Initialization error: " << socket_err_msg << RESET;
        return ServerResult::err(oss.str());
    }

    ServerResult signal_result = set_signal();
    if (signal_result.is_err()) {
        std::ostringstream oss;
        oss << RED << "[Server Error] Initialization error: signal: " << signal_result.get_err_value() << RESET;
        return ServerResult::err(oss.str());
    }

    Result<IOMultiplexer *, std::string> fds_result = create_io_multiplexer_fds();
    if (fds_result.is_err()) {
        std::ostringstream oss;
        oss << RED << "[Server Error] Initialization error: " << fds_result.get_err_value() << RESET;
        return ServerResult::err(oss.str());
    }
    this->fds_ = fds_result.get_ok_value();
    return ServerResult::ok(OK);
}


Result<Socket *, std::string> Server::create_socket(const std::string &address,
                                                    const std::string &port) {
    Socket *socket = NULL;

    try {
        socket = new Socket(address, port);

        SocketResult init_result = socket->init();
        if (init_result.is_err()) {
            throw std::runtime_error(init_result.get_err_value());
        }

        SocketResult bind_result = socket->bind();
        if (bind_result.is_err()) {
            throw std::runtime_error(bind_result.get_err_value());
        }

        SocketResult listen_result = socket->listen();
        if (listen_result.is_err()) {
            throw std::runtime_error(listen_result.get_err_value());
        }

        SocketResult set_fd_result = socket->set_fd_to_nonblock();
        if (set_fd_result.is_err()) {
            throw std::runtime_error(set_fd_result.get_err_value());
        }

        return Result<Socket *, std::string>::ok(socket);
    }
    catch (std::bad_alloc const &e) {
        std::string err_info = CREATE_ERROR_INFO_STR("Failed to allocate memory");
        return Result<Socket *, std::string>::err(err_info);
    }
    catch (std::runtime_error const &e) {
        delete socket;
        return Result<Socket *, std::string>::err(e.what());
    }
}


ServerResult Server::create_sockets(const Config &config) {
    const std::map<ServerInfo, const ServerConfig *> &server_configs = config.get_server_configs();

    std::map<ServerInfo, const ServerConfig *>::const_iterator servers;
    for (servers = server_configs.begin(); servers != server_configs.end(); ++servers) {
        const std::string address = servers->first.address;
        const std::string port = servers->first.port;

        // std::cout << CYAN
        // << "create_sockets -> ip: " << address
        // << ", port: " << port << RESET << std::endl;
        try {
            Result<Socket *, std::string> socket_result = create_socket(address, port);
            if (socket_result.is_err()) {
                const std::string error_msg = socket_result.get_err_value();
                return ServerResult::err(error_msg);
            }
            Socket *socket = socket_result.get_ok_value();
            int socket_fd = socket->get_socket_fd();
            sockets_[socket_fd] = socket;
            // std::cout << "socket_fd: " << socket_fd << std::endl;
        }
        catch (std::bad_alloc const &e) {
            std::string err_info = CREATE_ERROR_INFO_STR("Failed to allocate memory");
            return ServerResult ::err(err_info);
        }
    }
    return ServerResult::ok(OK);
}


void Server::delete_sockets() {
    std::map<Fd, Socket *>::iterator itr;
    for (itr = this->sockets_.begin(); itr != this->sockets_.end(); ++itr) {
        delete itr->second;
    }
    this->sockets_.clear();
}


void Server::close_client_fd(int fd) {
    if (fd == INIT_FD) {
        return;
    }
    if (this->fds_) {
        this->fds_->clear_fd(fd);
    }
    std::deque<Fd>::iterator itr;
    for (itr = this->client_fds_.begin(); itr != this->client_fds_.end(); ++itr) {
        if (*itr != fd) {
            continue;
        }
        this->client_fds_.erase(itr);
        break;
    }
    int close_ret = close(fd);
    if (close_ret == CLOSE_ERROR) {
        std::cout << CYAN << "close error" << RESET << std::endl;  // todo: log
    }
}


Result<IOMultiplexer *, std::string> Server::create_io_multiplexer_fds() {
    try {
#if defined(__linux__) && !defined(USE_SELECT)
        IOMultiplexer *fds = new EPoll();
#elif defined(__APPLE__) && !defined(USE_SELECT)
        IOMultiplexer *fds = new Kqueue();
#else
        IOMultiplexer *fds = new Select();
#endif
        std::map<Fd, Socket *>::const_iterator socket;
        for (socket = this->sockets_.begin(); socket != this->sockets_.end(); ++socket) {
            int socket_fd = socket->first;

#if defined(__linux__) && !defined(USE_SELECT)
            fds->register_fd(socket_fd);
#elif defined(__APPLE__) && !defined(USE_SELECT)
            fds->register_fd(socket_fd);
#else
            fds->register_read_fd(socket_fd);
#endif
            this->socket_fds_.push_back(socket_fd);
            DEBUG_SERVER_PRINT(" socket_fd: %d", socket_fd);
        }
        return Result<IOMultiplexer *, std::string>::ok(fds);
    } catch (std::bad_alloc const &e) {
        std::string err_info = CREATE_ERROR_INFO_STR("Failed to allocate memory");
        return Result<IOMultiplexer *, std::string>::err(err_info);
    }
}


////////////////////////////////////////////////////////////////////////////////


ServerResult Server::run() {
    this->set_timeout(1000);
	while (true) {
        DEBUG_SERVER_PRINT(" run 1 get_io_ready_fd");
        management_timeout_sessions();

        DEBUG_SERVER_PRINT(" run 2 get_io_ready_fd");
        ServerResult fd_ready_result = this->fds_->get_io_ready_fd();
        DEBUG_SERVER_PRINT(" run 3 ready result");
		if (fd_ready_result.is_err()) {
            DEBUG_SERVER_PRINT(" run : error 1");
            const std::string error_msg = fd_ready_result.get_err_value();
            return ServerResult::err(error_msg);
		}
		int ready_fd = fd_ready_result.get_ok_value();
        DEBUG_SERVER_PRINT(" run 4 ready_fd: %d", ready_fd);
		if (ready_fd == IO_TIMEOUT) {
			std::cerr << "[Server INFO] timeout" << std::endl;
#ifdef UNIT_TEST
            break;
#else
            continue;
#endif
		}
        DEBUG_SERVER_PRINT(" run 5 communicate");
        ServerResult communicate_result = communicate_with_client(ready_fd);
		if (communicate_result.is_err()) {
            const std::string error_msg = communicate_result.get_err_value();
            DEBUG_SERVER_PRINT(" run : error 2");
            return ServerResult::err(error_msg);
		}
        DEBUG_SERVER_PRINT(" run 6 next loop");
    }
    return ServerResult::ok(OK);
}


void Server::erase_from_timeout_manager(int cgi_fd) {
    std::set<FdTimeoutLimitPair>::iterator itr;
    for (itr = this->cgi_fds_.begin(); itr != this->cgi_fds_.end(); ++itr) {
        int fd = itr->second;
        if (cgi_fd == fd) {
            this->cgi_fds_.erase(itr);
            return;
        }
    }
}


void Server::clear_cgi_fd_from_event_manager(int cgi_fd) {
    this->sessions_.erase(cgi_fd);
    this->fds_->clear_fd(cgi_fd);
    erase_from_timeout_manager(cgi_fd);
}


void Server::management_timeout_sessions() {
    // cgi session
    time_t current_time = std::time(NULL);
    DEBUG_PRINT(GREEN, "management_timeout current: %zu", current_time);
    std::set<FdTimeoutLimitPair>::const_iterator cgi;
    for (cgi = this->cgi_fds_.begin(); cgi != this->cgi_fds_.end(); ++cgi) {
        time_t timeout_limit = cgi->first;
        DEBUG_SERVER_PRINT(" cgi_fd: %d, timelimit: %zu, current: %zu", cgi->second, timeout_limit, current_time);
        if (current_time < timeout_limit) {
            break;
        }

        int cgi_fd = cgi->second;
        DEBUG_PRINT(GREEN, " timeout cgi %d -> kill", cgi_fd);
        ClientSession *client = this->sessions_[cgi_fd];
        client->kill_cgi_process();
        DEBUG_PRINT(GREEN, " cgi killed by signal");
    }

    // client session
    // todo
}


ServerResult Server::communicate_with_client(int ready_fd) {
    if (is_socket_fd(ready_fd)) {
        DEBUG_SERVER_PRINT("  ready_fd=socket fd: %d", ready_fd);
        return create_session(ready_fd);
    } else {
        DEBUG_SERVER_PRINT("  ready_fd=client fd: %d", ready_fd);
        return process_session(ready_fd);
    }
}


ServerResult Server::echo() {
    // todo
    return ServerResult::ok(OK);
}


bool Server::is_socket_fd(int fd) const {
    return this->sockets_.find(fd) != this->sockets_.end();
}


ServerResult Server::create_session(int socket_fd) {
    struct sockaddr_storage client_addr = {};
    ServerResult accept_result = accept_connect_fd(socket_fd, &client_addr);
    if (accept_result.is_err()) {
        const std::string error_msg = accept_result.get_err_value();
        return ServerResult::err(error_msg);
    }
    int connect_fd = accept_result.get_ok_value();
    // todo: mv
    errno = 0;
    if (fcntl(connect_fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC) == FCNTL_ERROR) {
        std::string err_info = CREATE_ERROR_INFO_ERRNO(errno);
        return Result<int, std::string>::err("fcntl:" + err_info);
    }

    // std::cout << CYAN << " accept fd: " << connect_fd << RESET << std::endl;

    if (this->sessions_.find(connect_fd) != this->sessions_.end()) {
        return ServerResult::err("error: fd duplicated");  // ?
    }
    try {
        // std::cout << CYAN << " new_session created" << RESET << std::endl;
        AddressPortPair client_listen = ClientSession::get_client_listen(client_addr);

        std::ostringstream oss; oss << client_listen;
        DEBUG_SERVER_PRINT("%s", oss.str().c_str());

        ClientSession *new_session = new ClientSession(socket_fd, connect_fd, client_listen, this->config_);
        this->sessions_[connect_fd] = new_session;
        // std::cout << CYAN << " session start" << connect_fd << RESET << std::endl;
        return ServerResult::ok(OK);
    }
    catch (const std::exception &e) {
        std::string err_info = CREATE_ERROR_INFO_STR("Failed to allocate memory: " + std::string(e.what()));
        return ServerResult::err(err_info);
    }
}


void Server::update_fd_type_read_to_write(const SessionState &session_state, int fd) {
    FdType fd_type = this->fds_->get_fd_type(fd);
    if (session_state == kSendingResponse && fd_type == kReadFd) {
        DEBUG_SERVER_PRINT("fd read -> write");
        this->fds_->clear_fd(fd);
        this->fds_->register_write_fd(fd);
        // std::cout << RED << "update write fd: " << fd << RESET << std::endl;
    }
}


void Server::delete_session(std::map<Fd, ClientSession *>::iterator session) {
    ClientSession *client_session = session->second;
    int client_fd = client_session->client_fd();

    this->fds_->clear_fd(client_fd);
    delete client_session;
    this->sessions_.erase(session);
}


bool is_cgi_fd(int result_value) {
    return result_value != OK;  // result_value is cgi_fd
}


void Server::update_fd_type(int fd,
                            FdType update_from,
                            FdType update_to) {
    if (this->fds_->get_fd_type(fd) == update_to || update_from == update_to) {
        return;
    }
    this->fds_->clear_fd(fd);
    if (update_to == kReadFd) {
        this->fds_->register_read_fd(fd);
    } else if (update_to == kWriteFd) {
        this->fds_->register_write_fd(fd);
    }
}


void Server::init_session(ClientSession *session) {
    if (!session) {
        return;
    }
    int client_fd = session->client_fd();
    update_fd_type(client_fd, kWriteFd, kReadFd);
    session->set_session_state(kReceivingRequest);
    session->clear_request();
    session->clear_response();
}


void Server::clear_sessions() {
    std::map<Fd, ClientSession *>::iterator session;
    for (session = this->sessions_.begin(); session != sessions_.end(); ++session) {
        delete session->second;
        session->second = NULL;
    }
    this->sessions_.clear();
}


bool Server::is_fd_type_expect(int fd, const FdType &type) {
    return this->fds_->get_fd_type(fd) == type;
}


void Server::register_cgi_fd_to_event_manager(ClientSession **client) {
    int fd = (*client)->cgi_fd();
    DEBUG_SERVER_PRINT("       cgi_fd: %d", fd);

    if (fd != INIT_FD) {
        this->fds_->register_read_fd(fd);
        this->sessions_[fd] = *client;

        time_t timeout_limit = (*client)->cgi_timeout_limit();
        DEBUG_SERVER_PRINT("       timeout: %zu", timeout_limit);
        this->cgi_fds_.insert(FdTimeoutLimitPair(timeout_limit, fd));
    }
}


bool Server::is_ready_to_send_response(const ClientSession &client) {
    return client.is_session_state_expect(kSendingResponse)
            && is_fd_type_expect(client.client_fd(), kReadFd);
}


bool Server::is_cgi_execute_completed(const ClientSession &client) {
    return client.is_session_state_expect(kCreatingCGIBody);
}


bool Server::is_session_completed(const ClientSession &client) {
    return client.is_session_state_expect(kSessionCompleted);
}


bool Server::is_session_error_occurred(const ClientSession &client) {
    return client.is_session_state_expect(kSessionError);
}


ServerResult Server::process_session(int ready_fd) {
    std::map<Fd, ClientSession *>::iterator session = this->sessions_.find(ready_fd);
    if (session == this->sessions_.end()) {
        const std::string error_msg = CREATE_ERROR_INFO_CSTR("error: fd unknown");
        return ServerResult::err(error_msg);
    }

    SessionResult result;
    ClientSession *client = session->second;
    if (ready_fd == client->client_fd()) {
        DEBUG_SERVER_PRINT("process_client_event");
        result = client->process_client_event();
    } else if (ready_fd == client->cgi_fd()) {
        DEBUG_SERVER_PRINT("process_file_event");
        result = client->process_file_event();
    } else {
        const std::string error_msg = CREATE_ERROR_INFO_CSTR("error: fd unknown");
        return ServerResult::err(error_msg);
    }

    if (result.is_err()) {
        const std::string error_msg = result.get_err_value();
        return ServerResult::err(error_msg);

    } else if (ClientSession::is_continue_recv(result)) {
        DEBUG_SERVER_PRINT("      process_session -> recv continue");
        return ServerResult::ok(OK);

    } else if (ClientSession::is_executing_cgi(result)) {
        DEBUG_SERVER_PRINT("      process_session -> cgi");
        register_cgi_fd_to_event_manager(&client);
        return ServerResult::ok(OK);

    } else if (ClientSession::is_connection_closed(result)) {
        delete_session(session);
        DEBUG_PRINT(RED, "connection closed");
        return ServerResult::ok(OK);
    }

    if (is_ready_to_send_response(*client)) {
        DEBUG_SERVER_PRINT("fd read -> write");
        this->fds_->clear_fd(ready_fd);
        this->fds_->register_write_fd(ready_fd);

    } else if (is_cgi_execute_completed(*client)) {
        DEBUG_SERVER_PRINT("client process completed(CGI): fd %d -> close", ready_fd);
        clear_cgi_fd_from_event_manager(ready_fd);
        return process_session(client->client_fd());

    } else if (is_session_completed(*client)) {
        DEBUG_SERVER_PRINT("client process completed(Client): fd %d -> close", ready_fd);
        delete_session(session);  // todo -> init_session
        // init_session(client);

    } else if (is_session_error_occurred(*client)) {
        DEBUG_SERVER_PRINT("client process error occurred: fd %d -> close", ready_fd);
        delete_session(session);
    }
    return ServerResult::ok(OK);
}


ServerResult Server::accept_connect_fd(int socket_fd,
                                       struct sockaddr_storage *client_addr) {
    if (MAX_SESSION <= this->client_fds_.size()) {
        std::cerr << "[Server Error] exceed max connection" << std::endl;
        return ServerResult::ok(OK);  // todo: continue, ok?
    }

    SocketResult accept_result = Socket::accept(socket_fd, client_addr);
    if (accept_result.is_err()) {
        const std::string error_msg = accept_result.get_err_value();
        return ServerResult::err(error_msg);
    }
	int connect_fd = accept_result.get_ok_value();
    DEBUG_SERVER_PRINT("  accepted connect fd: %d", connect_fd);

    ServerResult fd_register_result = this->fds_->register_read_fd(connect_fd);
	if (fd_register_result.is_err()) {
		std::string err_info = CREATE_ERROR_INFO_STR(fd_register_result.get_err_value());
		std::cerr << "[Server Error]" << err_info << std::endl;
		errno = 0;
		if (close(connect_fd) == CLOSE_ERROR) {
			err_info = CREATE_ERROR_INFO_ERRNO(errno);
			std::cerr << "[Server Error] close: "<< err_info << std::endl;
		}
	}
    this->client_fds_.push_back(connect_fd);
	return ServerResult::ok(connect_fd);
}


void Server::set_timeout(int timeout_msec) {
    this->fds_->set_timeout(timeout_msec);
}
