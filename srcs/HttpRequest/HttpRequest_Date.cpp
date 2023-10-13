#include "Constant.hpp"
#include "Color.hpp"
#include "HttpRequest.hpp"
#include "StringHandler.hpp"
#include "HttpMessageParser.hpp"

namespace {

Result<Date *, int> create_valid_http_date(const std::string &field_value) {
	Date *date;

	date =  new Date(field_value);
	if (date->is_err()) {
		delete date;
		return Result<Date *, int>::err(NG);
	}
	return Result<Date *, int>::ok(date);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

Result<int, int> HttpRequest::set_valid_http_date(const std::string &field_name,
												  const std::string &field_value) {
	Result<Date *, int> date_result;
	Date *date;

	clear_field_values_of(field_name);

	date_result = create_valid_http_date(field_value);
	if (date_result.is_err()) {
		return Result<int, int>::err(NG);
	}

	date = date_result.get_ok_value();
	this->_request_header_fields[field_name] = date;
	return Result<int, int>::ok(OK);
}

// Date: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
Result<int, int> HttpRequest::set_date(const std::string &field_name,
									   const std::string &field_value) {
	return set_valid_http_date(field_name, field_value);
}

// If-Modified-Since = HTTP-date
// https://www.rfc-editor.org/rfc/rfc9110#field.if-modified-since
Result<int, int> HttpRequest::set_if_modified_since(const std::string &field_name,
													const std::string &field_value) {
	if (has_multiple_field_names(field_name)) {
		return Result<int, int>::err(NG);
	}
	return set_valid_http_date(field_name, field_value);
}

// If-Unmodified-Since = HTTP-date
// https://www.rfc-editor.org/rfc/rfc9110#field.if-unmodified-since
Result<int, int> HttpRequest::set_if_unmodified_since(const std::string &field_name,
													  const std::string &field_value) {
	if (has_multiple_field_names(field_name)) {
		return Result<int, int>::err(NG);
	}
	return set_valid_http_date(field_name, field_value);
}

// Last-Modified = HTTP-date
// https://www.rfc-editor.org/rfc/rfc9110#field.last-modified
Result<int, int> HttpRequest::set_last_modified(const std::string &field_name,
												const std::string &field_value) {
	return set_valid_http_date(field_name, field_value);
}
