#include "Color.hpp"
#include "Constant.hpp"
#include "HttpRequest.hpp"
#include "MediaType.hpp"
#include "RequestLine.hpp"
#include "gtest/gtest.h"

TEST(TestMediaType, ContentTypeOK1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: text/html\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValues *field_values = request.get_field_values(field_name);
		MediaType *data = dynamic_cast<MediaType *>(field_values);

		//----------------------------------------------------------------------

		EXPECT_EQ("text", data->get_type());
		EXPECT_EQ("html", data->get_subtype());

		//----------------------------------------------------------------------

		std::map<std::string, std::string> params = data->get_parameters();
		std::map<std::string, std::string> ans = {};

		EXPECT_EQ(true, params.size() == ans.size());

		std::map<std::string, std::string>::iterator params_itr, ans_itr;
		params_itr = params.begin();
		ans_itr = ans.begin();
		while (params_itr != params.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, params_itr->second);
			++params_itr;
			++ans_itr;
		}
		EXPECT_TRUE(params_itr == params.end());
		EXPECT_TRUE(ans_itr == ans.end());

		//----------------------------------------------------------------------

		EXPECT_EQ(true, data->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMediaType, ContentTypeOK2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: text/html; charset=utf-8\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValues *field_values = request.get_field_values(field_name);
		MediaType *data = dynamic_cast<MediaType *>(field_values);

		//----------------------------------------------------------------------

		EXPECT_EQ("text", data->get_type());
		EXPECT_EQ("html", data->get_subtype());

		//----------------------------------------------------------------------

		std::map<std::string, std::string> params = data->get_parameters();
		std::map<std::string, std::string> ans = {{"charaset", "utf-8"}};

		EXPECT_EQ(true, params.size() == ans.size());

		std::map<std::string, std::string>::iterator params_itr, ans_itr;
		params_itr = params.begin();
		ans_itr = ans.begin();
		while (params_itr != params.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, params_itr->second);
			++params_itr;
			++ans_itr;
		}
		EXPECT_TRUE(params_itr == params.end());
		EXPECT_TRUE(ans_itr == ans.end());

		//----------------------------------------------------------------------

		EXPECT_EQ(true, data->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMediaType, ContentTypeOK3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: multipart/form-data ; charset=utf-8 ; boundary=something\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValues *field_values = request.get_field_values(field_name);
		MediaType *data = dynamic_cast<MediaType *>(field_values);

		//----------------------------------------------------------------------

		EXPECT_EQ("multipart", data->get_type());
		EXPECT_EQ("form-data", data->get_subtype());

		//----------------------------------------------------------------------

		std::map<std::string, std::string> params = data->get_parameters();
		std::map<std::string, std::string> ans = {{"charaset", "utf-8"},
												  {"boundary", "something"}};

		EXPECT_EQ(true, params.size() == ans.size());

		std::map<std::string, std::string>::iterator params_itr, ans_itr;
		params_itr = params.begin();
		ans_itr = ans.begin();
		while (params_itr != params.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, params_itr->second);
			++params_itr;
			++ans_itr;
		}
		EXPECT_TRUE(params_itr == params.end());
		EXPECT_TRUE(ans_itr == ans.end());

		//----------------------------------------------------------------------

		EXPECT_EQ(true, data->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMediaType, ContentTypeOK4) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: multipart/form-data ; charset=\"utf-8  sp ok \"   ; a=b  \r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValues *field_values = request.get_field_values(field_name);
		MediaType *data = dynamic_cast<MediaType *>(field_values);

		//----------------------------------------------------------------------

		EXPECT_EQ("multipart", data->get_type());
		EXPECT_EQ("form-data", data->get_subtype());

		//----------------------------------------------------------------------

		std::map<std::string, std::string> params = data->get_parameters();
		std::map<std::string, std::string> ans = {{"charaset", "\"utf-8  sp ok \""},
												  {"a", "b"}};

		EXPECT_EQ(true, params.size() == ans.size());

		std::map<std::string, std::string>::iterator params_itr, ans_itr;
		params_itr = params.begin();
		ans_itr = ans.begin();
		while (params_itr != params.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, params_itr->second);
			++params_itr;
			++ans_itr;
		}
		EXPECT_TRUE(params_itr == params.end());
		EXPECT_TRUE(ans_itr == ans.end());

		//----------------------------------------------------------------------

		EXPECT_EQ(true, data->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

////////////////////////////////////////////////////////////////////////////////

TEST(TestMediaType, ContentTypeNG1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: text\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMediaType, ContentTypeNG2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: text/html;\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMediaType, ContentTypeNG3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Type: text/html; hoge\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_TYPE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}