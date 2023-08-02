/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: user <user@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/30 11:35:54 by user              #+#    #+#             */
/*   Updated: 2023/08/01 23:03:03 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"

ServerConfig::ServerConfig():_maxBodySize(1024), _chunked_transferencoding_allow(false),
_keepaliverequests(10), _server_tokens(1), _autoindex(false), _default_type("application/octet-stream"){}

ServerConfig::~ServerConfig(){}

bool ServerConfig::serverkeyword_ch(const std::string& word)
{
	const std::string server_keyset_arr[] = {
        "listen", "server_name", "root", "index", "allow_methods", "error_page", "types", "cgi_extension",
        "chunked_transfer_encoding", "access_log", "error_log", "keepalive_requests",
        "keepalive_timeout", "server_tokens", "autoindex", "rewrite", "return", "client_body_buffer_size",
        "client_body_timeout", "client_header_buffer_size", "client_header_timeout",
        "client_max_body_size", "default_type"
    };

    const std::set<std::string> server_keyset
    (
        server_keyset_arr,
        server_keyset_arr + sizeof(server_keyset_arr) / sizeof(server_keyset_arr[0])
    );

    if (server_keyset.count(word) > 0)
        return true;
    return false;
}

bool	ServerConfig::serverkeyword_insert(std::string const &line)
{
	std::istringstream	splited_woeds(line);
	std::string			key_word;
	std::string			val;
	std::string			val_two;

	splited_woeds >> key_word >> val_two;
	val = HandlingString::skip_lastsemicoron(val_two);
	if (this->serverkeyword_ch(key_word) == false)
		throw ServerConfig::ServerKeywordError();
	if (key_word == "listen")
		this->set_port(val);
	else if (key_word == "cgi_extension")//何するかわかってない
		;
	else if (key_word == "server_name")
		this->set_servername(HandlingString::inputarg_tomap_without_firstword(line));
	else if (key_word == "root")
		this->set_root(val);
	else if (key_word == "index")
		this->set_root(val);
	else if (key_word == "allow_methods")
		this->set_allowmethod_set(HandlingString::inputarg_tomap_without_firstword(line));
	else if (key_word == "error_page")
		this->set_errorlog(val);
	else if (key_word == "chunked_transfer_encoding")
	{
		if (val != "on" && val != "off")
			return (false);
		this->set_chunked_transferencoding_allow(HandlingString::return_matchpattern("on", "off", val));
	}
	else if (key_word == "access_log")
		this->set_accesslog(val);
	else if (key_word == "error_log")
		this->set_errorlog(val);
	else if (key_word == "keepalive_requests")
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_keepaliverequests(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "keepalive_timeout")//timeoutの実装がC++98のみでは難しい、Cでも許可された関数にない
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_keepalive_timeout(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "server_tokens")
		;
	else if (key_word == "autoindex")
	{
		if (val != "on" && val != "off")
			return (false);
		this->set_autoindex(HandlingString::return_matchpattern("on", "off", val));
	}
	else if (key_word == "rewrite")//何するのかわからん
		;
	else if (key_word == "return")//何するのかわからん
		;
	else if (key_word == "client_body_buffer_size")//単位付きで入ってくる場合に対応する必要性、簡単のために単位なしに一旦する
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_client_body_buffer_size(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "client_body_timeout")
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_client_body_timeout(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "client_header_buffer_size")
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_client_header_buffer_size(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "client_header_timeout")
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_client_header_timeout(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "client_max_body_size")
	{
		if (HandlingString::check_under_intmax(val) == false)
			return (false);
		this->set_client_maxbody_size(HandlingString::str_to_int(HandlingString::skip_lastsemicoron(val)));
	}
	else if (key_word == "default_type")
		this->set_default_type(val);
	else
		return (false);
	return (true);
}