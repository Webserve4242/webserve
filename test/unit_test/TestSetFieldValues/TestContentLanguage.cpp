#include "Color.hpp"
#include "Constant.hpp"
#include "HttpRequest.hpp"
#include "RequestLine.hpp"
#include "SetFieldValues.hpp"
#include "gtest/gtest.h"

TEST(TestSetFieldValues, ContentLanguageOK1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: de-DE\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		SetFieldValues *multi_field_values = dynamic_cast<SetFieldValues *>(field_values);
		std::set<std::string> actual_set = multi_field_values->get_values();
		std::set<std::string> expected_set = {"de-DE"};

		EXPECT_EQ(actual_set.size(), expected_set.size());

		std::set<std::string>::iterator actual_itr, expected_itr;
		actual_itr = actual_set.begin();
		expected_itr = expected_set.begin();
		while (actual_itr != actual_set.end() && expected_itr != expected_set.end()) {
			EXPECT_EQ(*expected_itr, *actual_itr);
			++actual_itr;
			++expected_itr;
		}
		EXPECT_TRUE(actual_itr == actual_set.end());
		EXPECT_TRUE(expected_itr == expected_set.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestSetFieldValues, ContentLanguageOK2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: de-DE, en-CA\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		SetFieldValues *multi_field_values = dynamic_cast<SetFieldValues *>(field_values);
		std::set<std::string> actual_set = multi_field_values->get_values();
		std::set<std::string> expected_set = {"de-DE", "en-CA"};

		EXPECT_EQ(actual_set.size(), expected_set.size());

		std::set<std::string>::iterator actual_itr, expected_itr;
		actual_itr = actual_set.begin();
		expected_itr = expected_set.begin();
		while (actual_itr != actual_set.end() && expected_itr != expected_set.end()) {
			EXPECT_EQ(*expected_itr, *actual_itr);
			++actual_itr;
			++expected_itr;
		}
		EXPECT_TRUE(actual_itr == actual_set.end());
		EXPECT_TRUE(expected_itr == expected_set.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestSetFieldValues, ContentLanguageOK3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: AAA-CCCC-123-12345-a-12-1234bbbb-1212-x-12345678-aaaa-12ab\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		SetFieldValues *multi_field_values = dynamic_cast<SetFieldValues *>(field_values);
		std::set<std::string> actual_set = multi_field_values->get_values();
		std::set<std::string> expected_set = {"AAA-CCCC-123-12345-a-12-1234bbbb-1212-x-12345678-aaaa-12ab"};

		EXPECT_EQ(actual_set.size(), expected_set.size());

		std::set<std::string>::iterator actual_itr, expected_itr;
		actual_itr = actual_set.begin();
		expected_itr = expected_set.begin();
		while (actual_itr != actual_set.end() && expected_itr != expected_set.end()) {
			EXPECT_EQ(*expected_itr, *actual_itr);
			++actual_itr;
			++expected_itr;
		}
		EXPECT_TRUE(actual_itr == actual_set.end());
		EXPECT_TRUE(expected_itr == expected_set.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestSetFieldValues, ContentLanguageOK4) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: en-GB-oed, i-hak\r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_TRUE(has_field_name);

	if (has_field_name) {
		FieldValueBase *field_values = request.get_field_values(field_name);
		SetFieldValues *multi_field_values = dynamic_cast<SetFieldValues *>(field_values);
		std::set<std::string> actual_set = multi_field_values->get_values();
		std::set<std::string> expected_set = {"en-GB-oed", "i-hak"};

		EXPECT_EQ(actual_set.size(), expected_set.size());

		std::set<std::string>::iterator actual_itr, expected_itr;
		actual_itr = actual_set.begin();
		expected_itr = expected_set.begin();
		while (actual_itr != actual_set.end() && expected_itr != expected_set.end()) {
			EXPECT_EQ(*expected_itr, *actual_itr);
			++actual_itr;
			++expected_itr;
		}
		EXPECT_TRUE(actual_itr == actual_set.end());
		EXPECT_TRUE(expected_itr == expected_set.end());

	} else {
		ADD_FAILURE() << field_name << " not found";
	}

	EXPECT_EQ(STATUS_OK, request.get_status_code());
}


////////////////////////////////////////////////////////////////////////////////

TEST(TestSetFieldValues, ContentLanguageNG1) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: ,,a \r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}


TEST(TestSetFieldValues, ContentLanguageNG2) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: a,b, \r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}

TEST(TestSetFieldValues, ContentLanguageNG3) {
	const std::string request_line = "GET /index.html HTTP/1.1\r\n"
									 "Host: example.com\r\n"
									 "Content-Language: a-b-c \r\n"
									 "\r\n";
	HttpRequest request(request_line);
	bool has_field_name;
	std::string field_name = std::string(CONTENT_LANGUAGE);

	has_field_name = request.is_valid_field_name_registered(field_name);
	EXPECT_FALSE(has_field_name);
	EXPECT_EQ(STATUS_OK, request.get_status_code());
}
