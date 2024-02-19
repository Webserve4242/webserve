#include <sys/wait.h>
#include <cerrno>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "webserv.hpp"
#include "Color.hpp"
#include "Configuration.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpMessageParser.hpp"
#include "StringHandler.hpp"

namespace {

}  // namespace

////////////////////////////////////////////////////////////////////////////////

HttpResponse::HttpResponse(const HttpRequest &request,
                           const ServerConfig &server_config)
    : request_(request),
      server_config_(server_config),
      status_code_(STATUS_OK),
      cgi_read_fd_(INIT_FD),
      cgi_pid_(INIT_PID),
      headers_(),
      body_buf_(),
      response_message_() {}


HttpResponse::~HttpResponse() {
    kill_cgi_process();
    close_cgi_fd();
}


void HttpResponse::kill_cgi_process() {
    int status;
    if (!is_cgi_processing(&status)) {
        return;
    }
    errno = 0;
    if (kill(this->cgi_pid_, SIGKILL) == KILL_ERROR) {
        const std::string error_msg = CREATE_ERROR_INFO_ERRNO(errno);
        std::cerr << error_msg << std::endl;  // todo: log
    }
    this->cgi_pid_ = INIT_PID;  // todo: success only?
}


void HttpResponse::close_cgi_fd() {
    if (this->cgi_read_fd_ == INIT_FD) {
        return;
    }
    errno = 0;
    if (close(this->cgi_read_fd_) == CLOSE_ERROR) {
        const std::string error_msg = CREATE_ERROR_INFO_ERRNO(errno);
        std::cerr << error_msg << std::endl;  // todo: log
    }
    this->cgi_read_fd_ = INIT_FD;  // todo: success only?
}


Result<Fd, int> HttpResponse::exec_method() {
    DEBUG_PRINT(YELLOW, " exec_method 1 status_requ(%d), status_resp(%d)",
                this->request_.get_status_code(), this->status_code_);
    this->status_code_ = this->request_.get_status_code();

    if (this->request_.get_status_code() != STATUS_OK) {
        DEBUG_PRINT(YELLOW, "  status error -> error page");
        get_error_page();
        return Result<Fd, int>::err(ERR);  // todo: err ? ok?
    }

    std::string target_path = HttpResponse::get_resource_path(this->request_.get_request_target());
    DEBUG_PRINT(YELLOW, " exec_method 2 path: ", target_path.c_str());
    Method method = HttpMessageParser::get_method(this->request_.get_method());
    DEBUG_PRINT(YELLOW, " exec_method 3 method: ", method);
    std::ostringstream status_line_oss;
    Result<Fd, int> method_result;

    switch (method) {
        case kGET:
            DEBUG_PRINT(YELLOW, " exec_method 4 - GET");
            method_result = get_request_body(target_path);  // cgi -> return Fd
            break;

        case kPOST:
            DEBUG_PRINT(YELLOW, " exec_method 4 - POST");
            method_result = post_request_body(target_path);  // cgi -> return Fd
            break;

        case kDELETE:
            DEBUG_PRINT(YELLOW, " exec_method 4 - DELETE");
            method_result = delete_request_body(target_path);
            break;

        default:
            DEBUG_PRINT(YELLOW, " exec_method 4 - err");
            this->status_code_ = STATUS_BAD_REQUEST;
            method_result = Result<Fd, int>::err(ERR);
    }

    DEBUG_PRINT(YELLOW, " exec_method 5");
    if (method_result.is_err()) {
        DEBUG_PRINT(YELLOW, "  result->error");
        this->body_buf_.clear();
        DEBUG_PRINT(YELLOW, "  buc clear");
        get_error_page();
        DEBUG_PRINT(YELLOW, "  get_error_page ok");
        return Result<Fd, int>::err(ERR);  // todo: err ? ok?
    }

    DEBUG_PRINT(YELLOW, " exec_method 6");
    if (method_result.get_ok_value() == OK) {
        return Result<Fd, int>::ok(OK);
    }
    DEBUG_PRINT(YELLOW, " exec_method 7");
    int cgi_fd = method_result.get_ok_value();

    std::string body(this->body_buf_.begin(), this->body_buf_.end());
    DEBUG_PRINT(YELLOW, " exec_method body:[%s]", body.c_str());
    return Result<Fd, int>::ok(cgi_fd);
}


Result<int, int> HttpResponse::create_cgi_body() {
    // buf -> cgi body

    // translate_to_http_protocol(execute_cgi_result.get_ok_value());

    return Result<int, int>::ok(OK);
}


/*
 HTTP-message = start-line CRLF
				*( field-line CRLF )
				CRLF
				[ message-body ]
 https://triple-underscore.github.io/http1-ja.html#http.message
 */
Result<int, int> HttpResponse::create_response_message() {
    std::string status_line = create_status_line() + CRLF;
    std::string field_lines = create_field_lines();
    std::string empty = CRLF;

    this->response_message_.insert(this->response_message_.end(), status_line.begin(), status_line.end());
    this->response_message_.insert(this->response_message_.end(), field_lines.begin(), field_lines.end());
    this->response_message_.insert(this->response_message_.end(), empty.begin(), empty.end());
    this->response_message_.insert(this->response_message_.end(), this->body_buf_.begin(), this->body_buf_.end());

    std::string msg(this->response_message_.begin(), this->response_message_.end());
    DEBUG_SERVER_PRINT("response_message:[%s]", msg.c_str());
    return Result<int, int>::ok(OK);
}


std::string get_status_reason_phrase(StatusCode code) {
    std::map<StatusCode, std::string> reason_phrases;
    reason_phrases[200] = "OK";
    reason_phrases[404] = "Not Found";
    // todo

    std::map<StatusCode, std::string>::const_iterator itr;
    itr = reason_phrases.find(code);
    if (itr == reason_phrases.end()) {
        return EMPTY;
    }
    return reason_phrases[code];
}


// status-line = HTTP-version SP status-code SP [ reason-phrase ]
std::string HttpResponse::create_status_line() const {
    std::string status_line;

    status_line.append(this->request_.get_http_version());
    status_line.append(SP);
    status_line.append(StringHandler::to_string(this->status_code_));
    status_line.append(SP);
    status_line.append(get_status_reason_phrase(this->status_code_));
    return status_line;
}


// field-line = field-name ":" OWS field-values OWS
std::string HttpResponse::create_field_lines() const {
	std::map<std::string, std::string>::const_iterator itr;
	std::ostringstream response_headers_oss;
	std::string field_name, field_value;

	for (itr = headers_.begin(); itr != headers_.end(); ++itr) {
		field_name = itr->first;
		field_value = itr->second;

		response_headers_oss << field_name << ":" << SP << field_value << CRLF;
	}
	return response_headers_oss.str();
}



std::string HttpResponse::get_resource_path(const std::string &request_target) {
    std::string decoded = HttpMessageParser::decode(request_target);
    std::string normalized = HttpMessageParser::normalize(decoded);

    std::string root;
    Result<std::string, int> root_result = Configuration::get_root(this->server_config_, request_target);
    if (root_result.is_ok()) {
        root = root_result.get_ok_value();
    }

    if (request_target == "/") {
        Result<std::string, int> index_result = Configuration::get_index(this->server_config_, request_target);
        std::string index_page;
        if (index_result.is_ok()) {
            index_page = "/" + index_result.get_ok_value();
        }
        return root + index_page;
    }
    return root + request_target;
}


Result<int, int> HttpResponse::recv_cgi_result() {
    int cgi_fd = get_cgi_fd();
    // std::size_t recv_size;  // todo: unused??
    HttpRequest::recv_all_data(cgi_fd, &this->body_buf_);  // todo: continues print script...
    int exit_status;
    if (this->is_cgi_processing(&exit_status)) {
        return Result<int, int>::ok(CGI);
    }
    if (exit_status != EXIT_SUCCESS) {
        this->status_code_ = STATUS_SERVER_ERROR;
        return Result<int, int>::err(ERR);
    }
    close_cgi_fd();
    return Result<int, int>::ok(OK);
}


bool HttpResponse::is_cgi_processing(int *status) {
    if (this->cgi_pid_ == INIT_PID) {
        return false;
    }
    int child_status;
    errno = 0;
    pid_t wait_result = waitpid(this->cgi_pid_, &child_status, WNOHANG);
    if (errno != 0) {
        const std::string error_msg = CREATE_ERROR_INFO_ERRNO(errno);
        std::cerr << error_msg << std::endl;  // todo: log
    }
    if (wait_result == PROCESSING || (wait_result == WAIT_ERROR && errno != ECHILD)) {
        return true;
    }

    if (0 < wait_result && status) {
        *status = WEXITSTATUS(child_status);
    }
    this->cgi_pid_ = INIT_PID;
    return false;
}


const std::vector<unsigned char> &HttpResponse::get_response_message() const {
    return this->response_message_;
}

int HttpResponse::get_cgi_fd() const { return this->cgi_read_fd_; }


#ifdef ECHO

void HttpResponse::create_echo_msg(const std::vector<unsigned char> &recv_msg) {
    this->response_message_ = recv_msg;
    std::string echo_message = std::string(recv_msg.begin(), recv_msg.end());
    DEBUG_SERVER_PRINT("    create_echo_msg:[%s]", echo_message.c_str());
}

#endif
