#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "AllConfig.hpp"
#include "ServerConfig.hpp"
#include "../ErrorPage/ErrorPage.hpp"

class LocationConfig
{
	private:
		bool _autoindex;
		bool _chunked_transferencoding_allow;

		int _server_tokens;

		size_t _client_body_buffer_size;
		size_t _client_body_timeout;
		size_t _client_header_buffer_size;
		size_t _client_header_timeout;
		size_t _client_max_body_size;
		size_t _keepalive_requests;
		size_t _keepalive_timeout;

		std::string _alias;
		std::string _accesslog;
		std::string _cgi_path;
		std::string _default_type;
		std::string _errorlog;
		std::string _upload_path;
		std::string _root;

		std::vector<std::string> _allow_methods;
		std::vector<std::string> _index;
		std::vector<std::string> _server_name;

		ErrorPage _errorpages;

		bool is_valid_field_header_in_location(const std::string &field_header);

	public:
		LocationConfig();
		LocationConfig(const LocationConfig &other);
		~LocationConfig();

		LocationConfig& operator=(const LocationConfig &other);

		bool get_autoindex() const;
		bool get_chunked_transferencoding_allow() const;
		bool set_field_header_field_value(const std::string &field_header,
										  const std::string &field_value);

		int get_server_tokens() const;

		size_t get_client_body_buffer_size() const;
		size_t get_client_body_timeout() const;
		size_t get_client_header_buffer_size() const;
		size_t get_client_header_timeout() const;
		size_t get_client_max_body_size() const;
		size_t get_keepalive_requests() const;
		size_t get_keepalive_timeout() const;

		std::string get_alias() const;
		std::string get_accesslog() const;
		std::string get_cgi_path() const;
		std::string get_default_type() const;
		std::string get_errorlog() const;
		std::string get_upload_path() const;
		std::string get_root() const;

		std::vector<std::string> get_allow_methods() const;
		std::vector<std::string> get_index() const;
		std::vector<std::string> get_server_name() const;

		ErrorPage get_errorpages() const;

		void set_autoindex(const bool &autoindex);
		void set_chunked_transferencoding_allow(const bool &chunked_transferencoding_allow);
		void set_server_tokens(const int &server_tokens);
		void set_client_body_buffer_size(const size_t &client_body_buffer_size);
		void set_client_body_timeout(const size_t &client_body_timeout);
		void set_client_header_buffer_size(const size_t &client_header_buffer_size);
		void set_client_header_timeout(const size_t &client_header_timeout);
		void set_client_max_body_size(const size_t &client_max_body_size);
		void set_keepaliverequests(const size_t &keepalive_request);
		void set_keepalive_timeout(const size_t &keepalive_timeout);
		void set_maxBodySize(const size_t &client_max_body_size);
		void set_alias(const std::string &alias);
		void set_accesslog(const std::string &accesslog);
		void set_cgi_path(const std::string &cgi_path);
		void set_default_type(const std::string &default_type);
		void set_errorlog(const std::string &errorlog);
		void set_upload_path(const std::string &upload_path);
		void set_root(const std::string &root);
		void set_allow_methods(const std::vector<std::string> &allow_methods);
		void set_index(const std::vector<std::string> &index);
		void set_server_name(const std::vector<std::string> &server_name);
		void set_errorpages(const ErrorPage &errorpages);

		void init_location_config();
		void init_location_config_with_server_config(const ServerConfig &inputes_severconfig);
};