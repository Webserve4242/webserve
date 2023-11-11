#pragma once

# include <string>
# include <vector>
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"

class LocationConfig;
class ServerConfig;

class IsConfigFormat
{
	private:
		IsConfigFormat();
		IsConfigFormat(const IsConfigFormat &other);
		~IsConfigFormat();
		IsConfigFormat& operator=(const IsConfigFormat &other);

	public:
		static bool is_block_end(const std::string &config_line);
		static bool is_block_start(const std::string &block_end_word);
		static bool is_empty_file(const std::string &config_file_name);
		static bool is_ignore_line(const std::string &config_line);

		static int is_field_header(const std::string &config_line, size_t *pos);
		static int is_field_value(const std::string &config_line, size_t *pos);
		static int is_location_block_format(const std::string &config_line);
		static int is_start_location_block(const std::string &config_line,
										   std::string *config_location_path);
		static int is_start_location_block(const std::string &config_line);
		static int is_start_server_block(const std::string &config_line,
										 bool *in_server_block);
		static int is_server_block_format(const std::string	&config_line,
										  std::vector<std::string> field_headers);
		static bool	is_errorpage_format(const std::string &field_value);
		static bool	is_errorpage_with_response(const std::string &field_value);
		static bool is_errorpage(const std::string &field_value);
		static bool	is_statuscodes(const std::string &target_string);
		static bool	is_uri(const std::string &target_string);
};