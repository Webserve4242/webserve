#include "Color.hpp"
#include "Constant.hpp"
#include "HttpRequest.hpp"
#include "RequestLine.hpp"
#include "SetFieldValues.hpp"
#include "gtest/gtest.h"

TEST(TestMapFieldValues, KeepAlilveOK1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=5, max=1000\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		MapFieldValues *multi_field_values = dynamic_cast<MapFieldValues *>(field_values);
		std::map<std::string, std::string> values = multi_field_values->get_value_map();
		std::map<std::string, std::string> ans = {{"timeout", "5"},
												  {"max", "1000"}};

		EXPECT_EQ(true, values.size() == ans.size());

		std::map<std::string, std::string>::iterator value_itr, ans_itr;
		value_itr = values.begin();
		ans_itr = ans.begin();
		while (value_itr != values.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, value_itr->second);
			++value_itr;
			++ans_itr;
		}
		EXPECT_TRUE(value_itr == values.end());
		EXPECT_TRUE(ans_itr == ans.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveOK2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=0000005, max=0001000\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		MapFieldValues *multi_field_values = dynamic_cast<MapFieldValues *>(field_values);
		std::map<std::string, std::string> values = multi_field_values->get_value_map();
		std::map<std::string, std::string> ans = {{"timeout", "5"},
												  {"max", "0001000"}};

		EXPECT_EQ(true, values.size() == ans.size());

		std::map<std::string, std::string>::iterator value_itr, ans_itr;
		value_itr = values.begin();
		ans_itr = ans.begin();
		while (value_itr != values.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, value_itr->second);
			++value_itr;
			++ans_itr;
		}
		EXPECT_TRUE(value_itr == values.end());
		EXPECT_TRUE(ans_itr == ans.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveOK3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=0  ,  a=\" SP OK\"\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		MapFieldValues *multi_field_values = dynamic_cast<MapFieldValues *>(field_values);
		std::map<std::string, std::string> values = multi_field_values->get_value_map();
		std::map<std::string, std::string> ans = {{"timeout", "0"},
												  {"a", "\" SP OK\""}};

		EXPECT_EQ(true, values.size() == ans.size());

		std::map<std::string, std::string>::iterator value_itr, ans_itr;
		value_itr = values.begin();
		ans_itr = ans.begin();
		while (value_itr != values.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, value_itr->second);
			++value_itr;
			++ans_itr;
		}
		EXPECT_TRUE(value_itr == values.end());
		EXPECT_TRUE(ans_itr == ans.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveOK4) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: a=a, a=b, a=c\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		MapFieldValues *multi_field_values = dynamic_cast<MapFieldValues *>(field_values);
		std::map<std::string, std::string> values = multi_field_values->get_value_map();
		std::map<std::string, std::string> ans = {{"a", "c"}};

		EXPECT_EQ(true, values.size() == ans.size());

		std::map<std::string, std::string>::iterator value_itr, ans_itr;
		value_itr = values.begin();
		ans_itr = ans.begin();
		while (value_itr != values.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, value_itr->second);
			++value_itr;
			++ans_itr;
		}
		EXPECT_TRUE(value_itr == values.end());
		EXPECT_TRUE(ans_itr == ans.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveOK5) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: a=a\r\n"
									 "keep-alive:   a=b\r\n"
									 "Keep-Alive: a=c  \r\n"
									 "KEEP-ALIVE: timeout=5, max=1000\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		MapFieldValues *multi_field_values = dynamic_cast<MapFieldValues *>(field_values);
		std::map<std::string, std::string> values = multi_field_values->get_value_map();
		std::map<std::string, std::string> ans = {{"timeout", "5"},
												  {"max", "1000"}};

		EXPECT_EQ(true, values.size() == ans.size());

		std::map<std::string, std::string>::iterator value_itr, ans_itr;
		value_itr = values.begin();
		ans_itr = ans.begin();
		while (value_itr != values.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, value_itr->second);
			++value_itr;
			++ans_itr;
		}
		EXPECT_TRUE(value_itr == values.end());
		EXPECT_TRUE(ans_itr == ans.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}


TEST(TestMapFieldValues, KeepAlilveOK6) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "KEEP-ALIVE: timeout=5, max=1000, a, b, c=d\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		MapFieldValues *multi_field_values = dynamic_cast<MapFieldValues *>(field_values);
		std::map<std::string, std::string> values = multi_field_values->get_value_map();
		std::map<std::string, std::string> ans = {{"timeout", "5"},
												  {"max", "1000"},
												  {"a", ""},
												  {"b", ""},
												  {"c", "d"}};

		EXPECT_EQ(true, values.size() == ans.size());

		std::map<std::string, std::string>::iterator value_itr, ans_itr;
		value_itr = values.begin();
		ans_itr = ans.begin();
		while (value_itr != values.end() && ans_itr != ans.end()) {
			EXPECT_EQ(ans_itr->second, value_itr->second);
			++value_itr;
			++ans_itr;
		}
		EXPECT_TRUE(value_itr == values.end());
		EXPECT_TRUE(ans_itr == ans.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

////////////////////////////////////////////////////////////////////////////////

TEST(TestMapFieldValues, KeepAlilveNG1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: =\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveNG2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveNG3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=+0;\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveNG4) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=10,\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveNG5) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=10; a=b\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestMapFieldValues, KeepAlilveNG6) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Keep-Alive: timeout=10; a=b\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(KEEP_ALIVE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}