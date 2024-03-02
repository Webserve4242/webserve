#include "Color.hpp"
#include "Constant.hpp"
#include "HttpRequest.hpp"
#include "RequestLine.hpp"
#include "Date.hpp"
#include "gtest/gtest.h"

TEST(TestDate, IfModifiedSinceOK1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "If-Modified-Since: Wed, 21 Oct 2015 07:28:00 GMT\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(IF_MODIFIED_SINCE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		Date *date = dynamic_cast<Date *>(field_values);
		EXPECT_EQ("Wed", date->get_day_name());
		EXPECT_EQ("21", date->get_day());
		EXPECT_EQ("Oct", date->get_month());
		EXPECT_EQ("2015", date->get_year());
		EXPECT_EQ("07", date->get_hour());
		EXPECT_EQ("28", date->get_minute());
		EXPECT_EQ("00", date->get_second());
		EXPECT_EQ("GMT", date->get_gmt());
		EXPECT_EQ(true, date->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.request_status());
}

TEST(TestDate, IfModifiedSinceOK2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "If-Modified-Since: Fri, 13 Oct 2023 00:21:45 GMT \r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(IF_MODIFIED_SINCE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		Date *date = dynamic_cast<Date *>(field_values);

		EXPECT_EQ("Fri", date->get_day_name());
		EXPECT_EQ("13", date->get_day());
		EXPECT_EQ("Oct", date->get_month());
		EXPECT_EQ("2023", date->get_year());
		EXPECT_EQ("00", date->get_hour());
		EXPECT_EQ("21", date->get_minute());
		EXPECT_EQ("45", date->get_second());
		EXPECT_EQ("GMT", date->get_gmt());
		EXPECT_EQ(true, date->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.request_status());
}

TEST(TestDate, IfModifiedSinceOK3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "If-Modified-Since: Sun, 28 Feb 2021 00:21:45 GMT\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(IF_MODIFIED_SINCE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		Date *date = dynamic_cast<Date *>(field_values);

		EXPECT_EQ("Sun", date->get_day_name());
		EXPECT_EQ("28", date->get_day());
		EXPECT_EQ("Feb", date->get_month());
		EXPECT_EQ("2021", date->get_year());
		EXPECT_EQ("00", date->get_hour());
		EXPECT_EQ("21", date->get_minute());
		EXPECT_EQ("45", date->get_second());
		EXPECT_EQ("GMT", date->get_gmt());
		EXPECT_EQ(true, date->is_ok());
	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.request_status());
}

////////////////////////////////////////////////////////////////////////////////

TEST(TestDate, IfModifiedSinceNG1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "If-Modified-Since: Wed,  21  Oct  2015  07:28:00  GMT\r\n"  // ERR
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(IF_MODIFIED_SINCE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);

	EXPECT_EQ(STATUS_BAD_REQUEST, request.request_status());
}

TEST(TestDate, IfModifiedSinceNG2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "If-Modified-Since: ERR-dayo-\r\n"  // ERR
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(IF_MODIFIED_SINCE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);

	EXPECT_EQ(STATUS_BAD_REQUEST, request.request_status());
}

TEST(TestDate, IfModifiedSinceNG3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "If-Modified-Since: Thu, 29 Feb 2024 00:21:45 GMT\r\n"  // OK
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(IF_MODIFIED_SINCE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);

	EXPECT_EQ(STATUS_BAD_REQUEST, request.request_status());
}
