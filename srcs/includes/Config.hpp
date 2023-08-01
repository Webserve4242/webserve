#ifndef Config_HPP
#define Config_HPP

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class	ServerConfig;
class	LocationConfig;

class	Config
{
	private:
		std::map<std::string, ServerConfig>							server_configs;

	public:
		Config(std::string	const &config);
		~Config();

		void									config_linecheck(std::string &line, bool &in_server, bool &in_location, ServerConfig &server_config);
		void									handle_serverinfs(std::string &line, bool &in_server, bool &in_location, ServerConfig &server_config);
		bool									handle_locationinfs(std::string &line, bool &in_server, bool &in_location, LocationConfig &server_config, std::map<std::string, ServerConfig>::iterator &it, std::string const &location_path, size_t pos);
		
		std::map<std::string, ServerConfig>		get_server_config(){ return (this->server_configs); };

		void									config_linecheck(std::string &line, bool &in_server, bool &in_location, ServerConfig &server_config);

		void									handle_serverinfs(std::string &line, bool &in_server, bool &in_location, ServerConfig &server_config);

		// void									reset_contents();
		void									config_location_check(std::string &line, bool &in_server, bool &in_location, LocationConfig &location_config, std::string &location_path, std::map<std::string, ServerConfig>::iterator	&it, size_t &pos);
		void									handle_locationinfs(std::string &line, bool &in_server, bool &in_location, LocationConfig &location_config, std::map<std::string, ServerConfig>::iterator	&it);

		class	ConfigError
		{
			public:
				virtual const char* what() const throw(){};
		};
};

#endif