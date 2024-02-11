#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "Color.hpp"
#include "Constant.hpp"
#include "HttpRequest.hpp"
#include "HttpMessageParser.hpp"
#include "StringHandler.hpp"

/* sub funcs; unnamed namespace */
namespace {

// field-line = field-name ":" OWS field-value OWS
//              ^head       ^colon
Result<std::string, int> parse_field_name(const std::string &field_line,
										  std::size_t *pos) {
	std::size_t head_pos, colon_pos, len;
	std::string field_name;

	if (!pos) { return Result<std::string, int>::err(ERR); }

	head_pos = 0;
	colon_pos = field_line.find(':', head_pos);
	if (colon_pos == std::string::npos || colon_pos <= head_pos) {
		return Result<std::string, int>::err(ERR);
	}
	len = colon_pos - head_pos;

	field_name = field_line.substr(head_pos, len);
	*pos += len;
	return Result<std::string, int>::ok(field_name);
}

void skip_whitespace(const std::string &str, std::size_t *pos) {
	if (!pos) { return; }

	while (HttpMessageParser::is_whitespace(str[*pos])) {
		(*pos)++;
	}
}

void skip_non_whitespace(const std::string &str, std::size_t *pos) {
	if (!pos) { return; }

	while (str[*pos] && !HttpMessageParser::is_whitespace(str[*pos])) {
		(*pos)++;
	}
}

// field-line CRLF
// field-line = field-name ":" OWS field-value OWS
//                                 ^head
Result<std::string, int> parse_field_value(const std::string &field_line,
										   std::size_t *head_pos) {
	std::size_t len, ws_len;
	std::string field_value;

	if (!head_pos) { return Result<std::string, int>::err(ERR); }

	len = 0;
	while (field_line[*head_pos + len]) {
		skip_non_whitespace(&field_line[*head_pos], &len);

		ws_len = 0;
		skip_whitespace(&field_line[*head_pos + len], &ws_len);

		if (field_line[*head_pos + len + ws_len] == '\0') {
			break;
		}
		len += ws_len;
	}

	field_value = field_line.substr(*head_pos, len);
	*head_pos += len;
	return Result<std::string, int>::ok(field_value);
}

void restore_crlf_to_ss(std::stringstream *ss) {
	std::streampos current_pos = ss->tellg();
	ss->seekg(current_pos - std::streamoff(std::string(CRLF).length()));
}

Result<std::string, int> get_field_line_by_remove_cr(const std::string &line_end_with_cr) {
	std::string field_line;

	if (!HttpMessageParser::is_end_with_cr(line_end_with_cr)) {
		return Result<std::string, int>::err(ERR);
	}
	field_line = line_end_with_cr.substr(0, line_end_with_cr.length() - 1);
	return Result<std::string, int>::ok(field_line);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
/* constructor, destructor */

HttpRequest::HttpRequest(const std::string &input) {
	init_field_name_parser();
	init_field_name_counter();
	this->status_code_ = parse_and_validate_http_request(input);
}

HttpRequest::~HttpRequest()
{
	std::map<std::string, FieldValueBase*>::iterator itr;

	itr = this->request_header_fields_.begin();
	while (itr != this->request_header_fields_.end()) {
		delete itr->second;
		++itr;
	}
}

void HttpRequest::clear_field_values_of(const std::string &field_name) {
	if (is_valid_field_name_registered(field_name)) {
		delete this->request_header_fields_[field_name];
		this->request_header_fields_.erase(field_name);
	}
}

////////////////////////////////////////////////////////////////////////////////
/* parse and validate http_request */

/*
 HTTP-message
	= start-line CRLF
	  *( field-line CRLF )
	  CRLF
	  [ message-body ]
 */
int HttpRequest::parse_and_validate_http_request(const std::string &input) {
	std::stringstream	ss(input);
	std::string 		line;

	// start-line CRLF
	std::getline(ss, line, LF);
    Result<int, int> request_line_result = this->request_line_.parse_and_validate(line);
	if (request_line_result.is_err()) {
		return STATUS_BAD_REQUEST;
	}

	// *( field-line CRLF )
	try {
        Result<int, int> field_line_result = parse_and_validate_field_lines(&ss);

        if (field_line_result.is_err()) {
            if (field_line_result.get_err_value() == STATUS_SERVER_ERROR) {
                return STATUS_SERVER_ERROR;
            }
            return STATUS_BAD_REQUEST;
        }
	} catch (const std::bad_alloc &e) {
		return STATUS_SERVER_ERROR;
	}

	// CRLF
	std::getline(ss, line, LF);
	if (line != std::string(1, CR)) {
		return STATUS_BAD_REQUEST;
	}

	// [ message-body ]
	message_body_ = parse_message_body(&ss);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
/* field-line parse and validate */

/*
 field-line CRLF
  v getline
 field-line CR

 field-line = field-name ":" OWS field-value OWS
 */
Result<int, int> HttpRequest::parse_and_validate_field_lines(std::stringstream *ss) {
	while (true) {
        std::string	line_end_with_cr;
		std::getline(*ss, line_end_with_cr, LF);
		if (ss->eof()) {
			return Result<int, int>::err(ERR);
		}
		if (HttpMessageParser::is_header_body_separator(line_end_with_cr)) {
			restore_crlf_to_ss(ss);
			break;
		}

        Result<std::string, int> field_line_result = get_field_line_by_remove_cr(line_end_with_cr);
		if (field_line_result.is_err()) {
			return Result<int, int>::err(ERR);
		}
		std::string field_line = field_line_result.get_ok_value();

        std::string	field_name, field_value;
        Result<int, int> parse_result = parse_field_line(field_line, &field_name, &field_value);
		if (parse_result.is_err()) {
			return Result<int, int>::err(ERR);
		}

		if (!HttpMessageParser::is_valid_field_name_syntax(field_name)
			|| !HttpMessageParser::is_valid_field_value_syntax(field_value)) {
			return Result<int, int>::err(ERR);
		}

		field_name = StringHandler::to_lower(field_name);
		if (is_field_name_supported_parsing(field_name)) {
			increment_field_name_counter(field_name);

			parse_result = (this->*field_value_parser_[field_name])(field_name, field_value);
			if (parse_result.is_err()) {
				return Result<int, int>::err(parse_result.get_err_value());
			}
			continue;
		}
	}

	// todo: validate field_names, such as 'must' header,...
	if (!is_valid_field_name_registered(std::string(HOST))) {
		// std::cout << MAGENTA << "!is valid field name registered" << RESET << std::endl;
		return Result<int, int>::err(ERR);
	}
	return Result<int, int>::ok(OK);
}

// field-line = field-name ":" OWS field-value OWS
Result<int, int> HttpRequest::parse_field_line(const std::string &field_line,
								  std::string *ret_field_name,
								  std::string *ret_field_value) {
	if (!ret_field_name || !ret_field_value) { return Result<int, int>::err(ERR); }

	// field-name
	std::size_t pos = 0;
    Result<std::string, int> field_name_result = parse_field_name(field_line, &pos);
	if (field_name_result.is_err()) {
		return Result<int, int>::err(ERR);
	}
	std::string field_name = field_name_result.get_ok_value();

	// ":"
	if (field_line[pos] != ':') {
		return Result<int, int>::err(ERR);
	}
	++pos;

	// OWS
	while (HttpMessageParser::is_whitespace(field_line[pos])) {
		++pos;
	}

	// field-value
    Result<std::string, int> field_value_result = parse_field_value(field_line, &pos);
	if (field_value_result.is_err()) {
		return Result<int, int>::err(ERR);
	}
    std::string field_value = field_value_result.get_ok_value();

	// OWS
	while (HttpMessageParser::is_whitespace(field_line[pos])) {
		++pos;
	}
	if (field_line[pos] != '\0') {
		return Result<int, int>::err(ERR);
	}

	*ret_field_name = field_name;
	*ret_field_value = field_value;
	return Result<int, int>::ok(OK);
}

// [ message-body ]
std::string HttpRequest::parse_message_body(std::stringstream *ss) {
	return ss->str();
}

bool HttpRequest::is_valid_field_name_registered(const std::string &field_name) {
	return this->request_header_fields_.count(field_name) != 0;
}

// call this function after increment
bool HttpRequest::is_field_name_repeated_in_request(const std::string &field_name) {
	return SINGLE_OCCURRENCE_LIMIT < this->field_name_counter_[field_name];
}

void HttpRequest::increment_field_name_counter(const std::string &field_name) {
	this->field_name_counter_[field_name]++;
}

bool HttpRequest::is_field_name_supported_parsing(const std::string &field_name) {
	if (HttpMessageParser::is_ignore_field_name(field_name)) {
		return false;
	}
	return this->field_value_parser_.count(field_name) != 0;
}

////////////////////////////////////////////////////////////////////////////////

void HttpRequest::init_field_name_counter() {
	std::vector<std::string>::const_iterator itr;
	for (itr = FIELD_NAMES.begin(); itr != FIELD_NAMES.end(); ++itr) {
		this->field_name_counter_[*itr] = COUNTER_INIT;
	}
}

void HttpRequest::init_field_name_parser() {
	std::map<std::string, func_ptr> map;

	map[std::string(ACCEPT)] = &HttpRequest::set_accept;
	map[std::string(ACCEPT_ENCODING)] = &HttpRequest::set_accept_encoding;
	map[std::string(ACCEPT_LANGUAGE)] = &HttpRequest::set_accept_language;
	map[std::string(ACCESS_CONTROL_REQUEST_HEADERS)] = &HttpRequest::set_access_control_request_headers;
	map[std::string(ACCESS_CONTROL_REQUEST_METHOD)] = &HttpRequest::set_access_control_request_method;
	map[std::string(ALT_USED)] = &HttpRequest::set_alt_used;
	map[std::string(AUTHORIZATION)] = &HttpRequest::set_authorization;
	map[std::string(CACHE_CONTROL)] =  &HttpRequest::set_cache_control;
	map[std::string(CONNECTION)] = &HttpRequest::set_connection;
	map[std::string(CONTENT_DISPOSITION)] = &HttpRequest::set_content_disposition;
	map[std::string(CONTENT_ENCODING)] = &HttpRequest::set_content_encoding;
	map[std::string(CONTENT_LANGUAGE)] = &HttpRequest::set_content_language;
	map[std::string(CONTENT_LENGTH)] = &HttpRequest::set_content_length;
	map[std::string(CONTENT_LOCATION)] = &HttpRequest::set_content_location;
	map[std::string(CONTENT_TYPE)] = &HttpRequest::set_content_type;
	map[std::string(COOKIE)] = &HttpRequest::set_cookie;
	map[std::string(DATE)] = &HttpRequest::set_date;
	map[std::string(EXPECT)] = &HttpRequest::set_expect;
	map[std::string(FORWARDED)] = &HttpRequest::set_forwarded;
	map[std::string(FROM)] = &HttpRequest::set_from;
	map[std::string(HOST)] = &HttpRequest::set_host;
	map[std::string(IF_MATCH)] = &HttpRequest::set_if_match;
	map[std::string(IF_MODIFIED_SINCE)] = &HttpRequest::set_if_modified_since;
	map[std::string(IF_NONE_MATCH)] = &HttpRequest::set_if_none_match;
	map[std::string(IF_RANGE)] = &HttpRequest::set_if_range;
	map[std::string(IF_UNMODIFIED_SINCE)] = &HttpRequest::set_if_unmodified_since;
	map[std::string(KEEP_ALIVE)] = &HttpRequest::set_keep_alive;
	map[std::string(LAST_MODIFIED)] = &HttpRequest::set_last_modified;
	map[std::string(LINK)] = &HttpRequest::set_link;
	map[std::string(MAX_FORWARDS)] = &HttpRequest::set_max_forwards;
	map[std::string(ORIGIN)] = &HttpRequest::set_origin;
	map[std::string(PROXY_AUTHORIZATION)] = &HttpRequest::set_proxy_authorization;
	map[std::string(RANGE)] = &HttpRequest::set_range;
	map[std::string(REFERER)] = &HttpRequest::set_referer;
	map[std::string(SEC_FETCH_DEST)] = &HttpRequest::set_sec_fetch_dest;
	map[std::string(SEC_FETCH_MODE)] = &HttpRequest::set_sec_fetch_mode;
	map[std::string(SEC_FETCH_SITE)] = &HttpRequest::set_sec_fetch_site;
	map[std::string(SEC_FETCH_USER)] = &HttpRequest::set_sec_fetch_user;
	map[std::string(SEC_PURPOSE)] = &HttpRequest::set_sec_purpose;
	map[std::string(SERVICE_WORKER_NAVIGATION_PRELOAD)] = &HttpRequest::set_service_worker_navigation_preload;
	map[std::string(TE)] = &HttpRequest::set_te;
	map[std::string(TRAILER)] = &HttpRequest::set_trailer;
	map[std::string(TRANSFER_ENCODING)] = &HttpRequest::set_transfer_encoding;
	map[std::string(UPGRADE)] = &HttpRequest::set_upgrade;
	map[std::string(UPGRADE_INSECURE_REQUESTS)] = &HttpRequest::set_upgrade_insecure_requests;
	map[std::string(USER_AGENT)] = &HttpRequest::set_user_agent;
	map[std::string(VIA)] = &HttpRequest::set_via;

	this->field_value_parser_ = map;
}

std::string HttpRequest::get_method() const {
	return this->request_line_.get_method();
}

std::string HttpRequest::get_request_target() const {
	return this->request_line_.get_request_target();
}

std::string HttpRequest::get_http_version() const {
	return this->request_line_.get_http_version();
}

FieldValueBase* HttpRequest::get_field_values(const std::string &key) {
	return this->request_header_fields_[key];
}

std::map<std::string, FieldValueBase*> HttpRequest::get_request_header_fields(void) {
	return this->request_header_fields_;
}

int	HttpRequest::get_status_code() const {
	return this->status_code_;
}
